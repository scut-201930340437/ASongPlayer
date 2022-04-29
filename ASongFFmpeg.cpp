#include "ASongFFmpeg.h"

#include<QDebug>

int ASongFFmpeg::curMediaStatus = 0;
int ASongFFmpeg::maxPacketListLength = 10;
QAtomicPointer<ASongFFmpeg> ASongFFmpeg::_instance = nullptr;
QMutex ASongFFmpeg::_mutex;

// 默认构造
ASongFFmpeg::ASongFFmpeg()
{
    //    av_register_all();
    //    avcodec_register_all();
}
// 全局访问点
ASongFFmpeg* ASongFFmpeg::getInstance()
{
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        _instance.testAndSetOrdered(nullptr, new ASongFFmpeg);
    }
    return _instance;
}

//ASongFFmpeg::ASongFFmpeg(QWidget*screen_widget)
//{
//    //    av_register_all();
////    avformat_network_init();

//    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
//    {
//        qDebug() << "Could not initialize SDL" << SDL_GetError();
//    }
//    screen = SDL_CreateWindowFrom((void *)screen_widget->winId());
//    if (!screen)
//    {
//        qDebug() << "SDL: could not create window - exiting:" << SDL_GetError();
//    }
//    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
//}

//ASongFFmpeg::~ASongFFmpeg()
//{
//    // 销毁渲染器
//    if (sdlRenderer)
//    {
//        SDL_DestroyRenderer(sdlRenderer);
//    }
//    sdlRenderer = nullptr;
//    // 销毁窗口
//    if (screen)
//    {
//        SDL_DestroyWindow(screen);
//    }
//    screen = nullptr;
//    // 退出SDL
//    SDL_Quit();
//}

//int ASongFFmpeg::thread_exit = 0;

//int ASongFFmpeg::sfp_signal_thread(void *opaque)
//{
//    double *avg_frame_rate = (double*)opaque;
//    //    qDebug() << *avg_frame_rate;
//    curMediaStatus = 1;
//    while (curMediaStatus == 1 || curMediaStatus == 2)
//    {
//        SDL_Event event;
//        if(curMediaStatus == 1)
//        {
//            event.type = SFM_REFRESH_EVENT;
//        }
//        else
//        {
//            event.type = SFM_PAUSE_EVENT;
//        }
//        SDL_PushEvent(&event);
//        // 播放帧率控制，可不可以通过其他方式控制，该方式很难处理不同帧率的视频
//        SDL_Delay(ceil(600 / (*avg_frame_rate)));
//    }
//    curMediaStatus = 0;
//    //Break
//    SDL_Event event;
//    event.type = SFM_BREAK_EVENT;
//    SDL_PushEvent(&event);
//    return 0;
//}

int ASongFFmpeg::load(QString path)
{
    // 加锁，保证_instance同一时间只能被一个线程使用
    QMutexLocker locker(&_mutex);
    pFormatCtx = avformat_alloc_context();
    // 获取文件路径
    mediaMetaData.path = path;
    QStringList list = path.split("/");
    // 获取文件名
    mediaMetaData.filename = list[list.size() - 1];
    // 获取文件格式
    list = mediaMetaData.filename.split(".");
    mediaMetaData.format = list[list.size() - 1];
    // 获取总时长
    mediaMetaData.duration = pFormatCtx->duration;
    //    curMediaStatus = 1;
    std::string str = path.toStdString();
    // 打开文件
    int ret = avformat_open_input(&pFormatCtx, str.c_str(), nullptr, nullptr);
    if(ret != 0)
    {
        qDebug() << "Couldn't open input stream.";
        return -1;
    }
    // 读取文件信息
    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    if(ret < 0)
    {
        qDebug() << "Couldn't find stream information.";
        return -1;
    }
    // 分出视频流和音频流
    AVCodecContext* pCodecCtx = nullptr;
    AVCodec* pCodec = nullptr;
    for (int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        // 先获取解码器上下文
        pCodecCtx = pFormatCtx->streams[i]->codec;
        // 如果是视频流
        if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIdx = i;
            // 获取解码器
            pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
            if(!pCodec)
            {
                qDebug() << "Couldn't find video code.";
                return -1;
            }
            // 打开解码器
            ret = avcodec_open2(pCodecCtx, pCodec, nullptr);
            if(ret != 0)
            {
                qDebug() << "Couldn't open vidoe code.";
                return -1;
            }
            // 获取视频流元数据
            // 获取帧率
            mediaMetaData.video_meta_data.frame_rate = ceil(av_q2d(pFormatCtx->streams[i]->avg_frame_rate));
            mediaMetaData.video_meta_data.width = pCodecCtx->width;
            mediaMetaData.video_meta_data.height = pCodecCtx->height;
        }
        else
        {
            // 如果是音频流
            if(pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audioIdx = i;
                // 获取解码器
                pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
                if(!pCodec)
                {
                    qDebug() << "Couldn't find video code.";
                    return -1;
                }
                // 打开解码器
                ret = avcodec_open2(pCodecCtx, pCodec, nullptr);
                if(ret != 0)
                {
                    qDebug() << "Couldn't open vidoe code.";
                    return -1;
                }
                // 获取音频流元数据
                switch (pCodecCtx->sample_fmt)
                {
                    case AV_SAMPLE_FMT_S16:
                        mediaMetaData.audio_meta_data.sample_fmt = 16;
                        break;
                    case AV_SAMPLE_FMT_S32:
                        mediaMetaData.audio_meta_data.sample_fmt = 32;
                        break;
                    default:
                        mediaMetaData.audio_meta_data.sample_fmt = pCodecCtx->sample_fmt;
                        break;
                }
                mediaMetaData.audio_meta_data.sample_rate = pCodecCtx->sample_rate;
                mediaMetaData.audio_meta_data.channels = pCodecCtx->channels;
            }
        }
    }
    // 音频重采样
    //    pCodecCtx = pFormatCtx->streams[audioIdx]->codec;
    pAudioSwrCtx = swr_alloc();
    // 设置重采样参数
    swr_alloc_set_opts(pAudioSwrCtx,
                       pCodecCtx->channel_layout, AV_SAMPLE_FMT_S16, 44100,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate,
                       0, nullptr);
    // 初始化
    swr_init(pAudioSwrCtx);
    // 释放资源
    if(pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
        pCodec = nullptr;
    }
    return 0;
    //    pFrame = av_frame_alloc();
    //    pFrameYUV = av_frame_alloc();
    //    out_buffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    //    avpicture_fill((AVPicture*)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    //    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
    //                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    //    //SDL 2.0 Support for multiple windows
    //    screen_w = pCodecCtx->width;
    //    screen_h = pCodecCtx->height;
    //    //cout << screen_w << ' ' << screen_h << endl;
    //    //    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    //    //                              screen_w, screen_h, SDL_WINDOW_OPENGL);
    //    //IYUV: Y + U + V  (3 planes)
    //    //YV12: Y + V + U  (3 planes)
    //    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
    //    sdlRect.x = 0;
    //    sdlRect.y = 0;
    //    sdlRect.w = screen_w;
    //    sdlRect.h = screen_h;
    //    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    //    avg_frame_rate = av_q2d(pFormatCtx->streams[videoindex]->avg_frame_rate);
    //    //    qDebug() << avg_frame_rate;
    //    video_tid = SDL_CreateThread(sfp_signal_thread, NULL, &avg_frame_rate);
    //    // event loop
    //    while (true)
    //    {
    //        //Wait
    //        SDL_WaitEvent(&event);
    //        if (event.type == SFM_REFRESH_EVENT)
    //        {
    //            //------------------------------
    //            if (av_read_frame(pFormatCtx, packet) >= 0)
    //            {
    //                if (packet->stream_index == videoindex)
    //                {
    //                    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); // 解码
    //                    if (ret < 0)
    //                    {
    //                        qDebug() << "Decode Error.";
    //                        return -1;
    //                    }
    //                    if (got_picture)
    //                    {
    //                        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
    //                        //SDL---------------------------
    //                        SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
    //                        SDL_RenderClear(sdlRenderer);
    //                        //SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
    //                        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    //                        SDL_RenderPresent(sdlRenderer);
    //                        //SDL End-----------------------
    //                    }
    //                }
    //                av_free_packet(packet);
    //            }
    //            else
    //            {
    //                //Exit Thread
    //                curMediaStatus = 0;
    //            }
    //        }
    //        else if (event.type == SDL_QUIT)
    //        {
    //            curMediaStatus = 0;
    //        }
    //        else if (event.type == SFM_BREAK_EVENT)
    //        {
    //            break;
    //        }
    //    }
    //    // 销毁纹理
    //    if (sdlTexture)
    //    {
    //        SDL_DestroyTexture(sdlTexture);
    //    }
    //    sdlTexture = nullptr;
    //    // 释放FFmpeg资源
    //    if(img_convert_ctx)
    //    {
    //        sws_freeContext(img_convert_ctx);
    //    }
    //    img_convert_ctx = nullptr;
    //    if(pFrameYUV)
    //    {
    //        av_frame_free(&pFrameYUV);
    //    }
    //    pFrameYUV = nullptr;
    //    if(pFrame)
    //    {
    //        av_frame_free(&pFrame);
    //    }
    //    pFrame = nullptr;
    ////    if(pCodecCtx)
    ////    {
    ////        avcodec_close(pCodecCtx);
    ////    }
    ////    pCodecCtx = nullptr;
    ////    if(pFormatCtx)
    ////    {
    ////        avformat_close_input(&pFormatCtx);
    ////    }
    ////    pFormatCtx = nullptr;
    //    return 0;
}

AVPacket* ASongFFmpeg::readFrame()
{
    QMutexLocker locker(&_mutex);
    if(!pFormatCtx)
    {
        return nullptr;
    }
    AVPacket* packet = av_packet_alloc();
    int ret = av_read_frame(pFormatCtx, packet);
    if(ret != 0)
    {
        packet->size = -1;
    }
    return packet;
}

AVFrame* ASongFFmpeg::decode(AVPacket* packet)
{
    QMutexLocker locker(&_mutex);
    if(!pFormatCtx)
    {
        return nullptr;
    }
    AVFrame* frame = av_frame_alloc();
    int ret = avcodec_send_frame()
}


int ASongFFmpeg::play()
{
    curMediaStatus = 1;
    return 0;
}
int ASongFFmpeg::pause()
{
    curMediaStatus = 2;
    return 0;
}
int ASongFFmpeg::stop()
{
    curMediaStatus = 3;
    return 0;
}

int ASongFFmpeg::getScreenW()
{
    return screen_w;
}

int ASongFFmpeg::getScreenH()
{
    return screen_h;
}

int ASongFFmpeg::getMediaStatus()
{
    return curMediaStatus;
}

//void ASongFFmpeg::setMediaStatus(int status)
//{
//    curMediaStatus=status;
//}
