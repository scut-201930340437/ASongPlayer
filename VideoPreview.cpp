#include "VideoPreview.h"
#include<QDebug>

VideoPreview* VideoPreview::getInstance(){
    static VideoPreview _instance;
    return &_instance;
}

void VideoPreview::start(QString path, int videoIndex, Priority prio){
    if(isRunning())return;
    this->path = path;
    this->vidx = videoIndex;
    QThread::start(prio);
}

void VideoPreview::stop(){
    this->abort_req = true;
    if(continue_preview_thread){
        SDL_CondSignal(continue_preview_thread);
    }
    QThread::wait();
    continue_preview_thread = NULL;
    preview_img = QImage();
}

void VideoPreview::pause(){
    this->preview_req = false;
}

void VideoPreview::resume(){
    this->preview_req = true;
    if(continue_preview_thread){
        SDL_CondSignal(continue_preview_thread);
    }
}

void VideoPreview::run(){

    thread_fail = false;
    abort_req = false;
    preview_req = false;
    posSec = -1;

    int64_t lastPosSec = -1;
    int nb_bytes, y_size, ret;
    const AVPixelFormat pixelfmt = AV_PIX_FMT_RGB32;
    AVFormatContext *pfctx = NULL;

    AVCodecParameters *pCodecPara = NULL;
    AVCodec *pCodec = NULL;
    AVCodecContext *pCodecCtx = NULL;

    AVFrame *pFrame = NULL, *pFrameRGB = NULL;
    SwsContext *img_ctx = NULL;
    uint8_t *out_buffer = NULL;

    AVPacket *pkt = NULL, *nullpkt = NULL;

    SDL_mutex *mutex = NULL;

    continue_preview_thread = SDL_CreateCond();
    if(!continue_preview_thread){
        qDebug()<<"preview_thread: create cond error";
        thread_fail = true;
        goto preview_thread_quit;
    }
    mutex = SDL_CreateMutex();
    if(!mutex){
        qDebug()<<"preview_thread: create mutex error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    //没有视频流
    if(vidx == -1){
        qDebug()<<"preview_thread: file has no video stream";
        thread_fail = true;
        goto preview_thread_quit;
    }
    //分配avformat context
    if(!(pfctx = avformat_alloc_context())){
        qDebug()<<"preview_thread: Could not allocate av_format_context";
        thread_fail = true;
        goto preview_thread_quit;
    }
    //打开文件
    if(avformat_open_input(&pfctx, path.toStdString().c_str(), NULL, NULL) < 0){
        qDebug()<<"preview_thread: Could not open file";
        thread_fail = true;
        goto preview_thread_quit;
    }
    //获取流信息
    if(avformat_find_stream_info(pfctx, NULL) < 0){
        qDebug()<<"preview_thread: Could not find stream info";
        thread_fail = true;
        goto preview_thread_quit;
    }

    //获取解码器
    pCodecPara = pfctx->streams[vidx]->codecpar;
    pCodec = avcodec_find_decoder(pCodecPara->codec_id);
    if(!pCodec){
        qDebug()<<"preview_thread: Could not find decoder";
        thread_fail = true;
        goto preview_thread_quit;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx){
        qDebug()<<"preview_thread: Could not allocate av_codec_context";
        thread_fail = true;
        goto preview_thread_quit;
    }
    if(avcodec_parameters_to_context(pCodecCtx, pCodecPara) < 0){
        qDebug()<<"preview_thread: avcodec_parameters_to_context error";
        thread_fail = true;
        goto preview_thread_quit;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        thread_fail = true;
        goto preview_thread_quit;
    }

    if(!(pFrame = av_frame_alloc()) || !(pFrameRGB = av_frame_alloc())){
        qDebug()<<"preview_thread: av_frame_alloc error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    img_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     pixelfmt, SWS_BICUBIC, NULL, NULL, NULL);
    if(!img_ctx){
        qDebug()<<"preview_thread: sws_getContext error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    nb_bytes = av_image_get_buffer_size(pixelfmt, pCodecCtx->width, pCodecCtx->height, 1);
    if(nb_bytes < 0){
        qDebug()<<"get nb_bytes error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    out_buffer = (uint8_t*)av_malloc(nb_bytes*sizeof(uint8_t));
    if(!out_buffer){
        qDebug()<<"preview_thread: av_malloc out_buffer error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    if(av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, pixelfmt, pCodecCtx->width, pCodecCtx->height, 1) < 0){
        qDebug()<<"preview_thread: av_image_fill_arrays error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    y_size = pCodecCtx->width * pCodecCtx->height;

    if(!(pkt = av_packet_alloc()) || av_new_packet(pkt, y_size) < 0){
        qDebug()<<"preview_thread: allocate packet error";
        thread_fail = true;
        goto preview_thread_quit;
    }

    if(!(nullpkt = av_packet_alloc())){
        qDebug()<<"preview_thread: allocate null packet error";
        goto preview_thread_quit;
    }
    nullpkt->data = NULL;
    nullpkt->size = 0;

    for(;;){
        if(abort_req){
            break;
        }

        if(!preview_req){
            SDL_LockMutex(mutex);
            SDL_CondWait(continue_preview_thread, mutex);
            SDL_UnlockMutex(mutex);
        }

        if(lastPosSec != posSec){//位置发生变化，进行seek操作，更新缩略图
            lastPosSec = posSec;
            //seek操作
            ret = av_seek_frame(pfctx, -1, lastPosSec * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);

            //存一帧图片
            while(av_read_frame(pfctx, pkt) >= 0){
                if(pkt->stream_index == vidx){
                    if(avcodec_send_packet(pCodecCtx, pkt) == 0){
                        //送入空包开启draining mode
                        avcodec_send_packet(pCodecCtx, nullpkt);
                        while((avcodec_receive_frame(pCodecCtx, pFrame)) != AVERROR_EOF){
                            sws_scale(img_ctx,
                                      pFrame->data, pFrame->linesize,
                                      0, pCodecCtx->height,
                                      pFrameRGB->data, pFrameRGB->linesize);

                            preview_img = QImage(out_buffer, pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                        }
                        //重置解码器状态
                        avcodec_flush_buffers(pCodecCtx);
                    }
                    else{
                        //黑色空图片
                        preview_img = QImage(pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                        preview_img.fill(Qt::black);
                    }
                    av_packet_unref(pkt);
                    break;
                }
            }
        }
        img_up_to_date = true;
    }

preview_thread_quit:
    //退出操作
    if(pkt){
        av_packet_free(&pkt);
    }
    if(nullpkt){
        av_packet_free(&nullpkt);
    }
    if(out_buffer){
        av_free(out_buffer);
    }
    if(pFrameRGB){
        av_free(pFrameRGB);
    }
    if(pFrame){
        av_free(pFrame);
    }
    if(img_ctx){
        sws_freeContext(img_ctx);
    }
    if(pCodecCtx){
        avcodec_free_context(&pCodecCtx);
    }
    if(pfctx){
        avformat_close_input(&pfctx);
    }
    if(continue_preview_thread){
        SDL_DestroyCond(continue_preview_thread);
        continue_preview_thread = NULL;
    }
    if(mutex){
        SDL_DestroyMutex(mutex);
    }
}

QImage VideoPreview::getPreviewImg(){
    img_up_to_date = false;
    while(!img_up_to_date && isRunning());
    return preview_img;
}

void VideoPreview::setPosSec(int64_t posSec){
    this->posSec = posSec;
}
