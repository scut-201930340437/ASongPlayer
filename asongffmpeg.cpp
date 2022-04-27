#include "asongffmpeg.h"

#include<QDebug>

int ASongFFmpeg::curMediaStatus = 0;

ASongFFmpeg::ASongFFmpeg(QWidget*screen_widget)
{
    //    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {
        qDebug() << "Could not initialize SDL" << SDL_GetError();
    }
    screen = SDL_CreateWindowFrom((void *)screen_widget->winId());
    if (!screen)
    {
        qDebug() << "SDL: could not create window - exiting:" << SDL_GetError();
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
}

ASongFFmpeg::~ASongFFmpeg()
{
    // 销毁渲染器
    if (sdlRenderer)
    {
        SDL_DestroyRenderer(sdlRenderer);
    }
    sdlRenderer = nullptr;
    // 销毁窗口
    if (screen)
    {
        SDL_DestroyWindow(screen);
    }
    screen = nullptr;
    // 退出SDL
    SDL_Quit();
}

//int ASongFFmpeg::thread_exit = 0;

int ASongFFmpeg::sfp_signal_thread(void *opaque)
{
    curMediaStatus = 1;
    while (curMediaStatus == 1 || curMediaStatus == 2)
    {
        SDL_Event event;
        if(curMediaStatus == 1)
        {
            event.type = SFM_REFRESH_EVENT;
        }
        else
        {
            event.type = SFM_PAUSE_EVENT;
        }
        SDL_PushEvent(&event);
        SDL_Delay(1);
    }
    curMediaStatus = 0;
    //Break
    SDL_Event event;
    event.type = SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

int ASongFFmpeg::load(QString filename)
{
    curMediaStatus = 1;
    std::string str = filename.toStdString();
    // 打开文件
    if (avformat_open_input(&pFormatCtx, str.c_str(), NULL, NULL) != 0)
    {
        qDebug() << "Couldn't open input stream.";
        return -1;
    }
    // 读取文件流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug() << "Couldn't find stream information.";
        return -1;
    }
    // 找到视频流
    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    if (videoindex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        return -1;
    }
    // 找到解码器
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        qDebug() << "Codec not found.";
        return -1;
    }
    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        qDebug() << "Could not open codec.";
        return -1;
    }
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (uint8_t*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture*)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    //SDL 2.0 Support for multiple windows
    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    //cout << screen_w << ' ' << screen_h << endl;
    //    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    //                              screen_w, screen_h, SDL_WINDOW_OPENGL);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = screen_w;
    sdlRect.h = screen_h;
    packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    video_tid = SDL_CreateThread(sfp_signal_thread, NULL, NULL);
    // event loop
    for (;;)
    {
        //Wait
        SDL_WaitEvent(&event);
        if (event.type == SFM_REFRESH_EVENT)
        {
            //------------------------------
            if (av_read_frame(pFormatCtx, packet) >= 0)
            {
                if (packet->stream_index == videoindex)
                {
                    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet); // 解码
                    if (ret < 0)
                    {
                        qDebug() << "Decode Error.";
                        return -1;
                    }
                    if (got_picture)
                    {
                        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                        //SDL---------------------------
                        SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
                        SDL_RenderClear(sdlRenderer);
                        //SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
                        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                        SDL_RenderPresent(sdlRenderer);
                        //SDL End-----------------------
                    }
                }
                av_free_packet(packet);
            }
            else
            {
                //Exit Thread
                curMediaStatus = 0;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            curMediaStatus = 0;
        }
        else if (event.type == SFM_BREAK_EVENT)
        {
            break;
        }
    }
    // 销毁纹理
    if (sdlTexture)
    {
        SDL_DestroyTexture(sdlTexture);
    }
    sdlTexture = nullptr;
    // 释放FFmpeg资源
    if(img_convert_ctx)
    {
        sws_freeContext(img_convert_ctx);
    }
    img_convert_ctx = nullptr;
    if(pFrameYUV)
    {
        av_frame_free(&pFrameYUV);
    }
    pFrameYUV = nullptr;
    if(pFrame)
    {
        av_frame_free(&pFrame);
    }
    pFrame = nullptr;
    if(pCodecCtx)
    {
        avcodec_close(pCodecCtx);
    }
    pCodecCtx = nullptr;
    if(pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
    }
    pFormatCtx = nullptr;
    return 0;
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

int ASongFFmpeg::getMediaStatus()
{
    return curMediaStatus;
}

//void ASongFFmpeg::setMediaStatus(int status)
//{
//    curMediaStatus=status;
//}
