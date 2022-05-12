#include "ASongFFmpeg.h"
#include "ASongVideo.h"
#include "DataSink.h"
#include "SDLPaint.h"
QAtomicPointer<SDLPaint> SDLPaint::_instance = nullptr;
//QMutex SDLPaint::_mutex;

SDLPaint::~SDLPaint()
{
    //    sws_freeContext(pSwsCtx);
    //    SDL_DestroyRenderer(sdlRenderer);
    //    SDL_DestroyTexture(sdlTexture);
    //    SDL_DestroyWindow(screen);
    //    SDL_Quit();
    //
    if(nullptr != sdlTimer)
    {
        sdlTimer->stop();
        delete sdlTimer;
    }
}

SDLPaint* SDLPaint::getInstance()
{
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        _instance.testAndSetOrdered(nullptr, new SDLPaint);
    }
    return _instance;
}

int SDLPaint::init(void *winID)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {
        qDebug() << "Could not initialize SDL" << SDL_GetError();
        return -1;
    }
    screen = SDL_CreateWindowFrom(winID);
    if (nullptr == screen)
    {
        qDebug() << "SDL: could not create window - exiting:" << SDL_GetError();
        return -1;
    }
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    if(nullptr == sdlRenderer)
    {
        qDebug() << "create rendered failed";
        return -1;
    }
    // 设置输出宽高和pix_fmt
    //    setDstWH(initWidth, initHeight);
    // 创建纹理
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, srcWidth, srcHeight);
    //    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGRA4444, SDL_TEXTUREACCESS_STREAMING, dstWidth, dstHeight);
    if(nullptr == sdlTexture)
    {
        qDebug() << "create texture failed";
        return -1;
    }
    // 初始化swsCtx
    pSwsCtx = sws_getCachedContext(pSwsCtx, srcWidth, srcHeight, pix_fmt,
                                   srcWidth, srcHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    if(nullptr == pSwsCtx)
    {
        qDebug() << "swsGetCtx failed";
        return -1;
    }
    //    qDebug() << "sdlinifinish";
    // 开启定时器
    if(frameRate != -1)
    {
        preDelay = int(1000.0 / frameRate + 0.5);
        sdlTimer = new QTimer(this);
        connect(sdlTimer, &QTimer::timeout, this, &SDLPaint::getFrameYUV);
        sdlTimer->start(preDelay);
    }
    else
    {
        QTimer::singleShot(1500, this, &SDLPaint::getFrameYUV);
    }
    return 0;
}

void SDLPaint::setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt)
{
    // 设置参数
    srcWidth = width;
    srcHeight = height;
    //    srcRate = (float)srcWidth / srcHeight;
    frameRate = _frameRate;
    pix_fmt = _pix_fmt;
}

//void SDLPaint::setDstWH(const int screenWidth, const int screenHeight)
//{
//    // 保证源视频流的宽高比
//    // 宽>高，以宽为基准
//    if(srcRate > 1.0)
//    {
//        dstWidth = screenWidth;
//        dstHeight = screenWidth / srcRate;
//    }
//    // 宽<=高，以高为基准
//    else
//    {
//        dstHeight = screenHeight;
//        dstWidth = screenHeight * srcRate;
//    }
//}

// 转换为YUV图像并进行同步
void SDLPaint::getFrameYUV()
{
    AVFrame *frame = DataSink::getInstance()->takeNextFrame(1);
    if(nullptr == frame)
    {
        // 解码线程结束且拿不到frame，此时说明视频播放结束
        if(ASongVideo::getInstance()->isFinished())
        {
            pause();
        }
        return;
    }
    AVFrame *frameYUV = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                          srcWidth, srcHeight, 1));
    av_image_fill_arrays(frameYUV->data, frameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, srcWidth, srcHeight, 1);
    sws_scale(pSwsCtx, (const uint8_t* const*)frame->data,
              frame->linesize, 0, srcHeight,
              frameYUV->data, frameYUV->linesize);
    if(frameRate != -1)
    {
        double actualDelay = ASongVideo::getInstance()->synVideo(*((double*)frame->opaque));
        // 同步后释放该帧
        av_frame_free(&frame);
        // 绘制
        paint(frameYUV);
        // 重设延时
        preDelay = int(actualDelay * 1000.0 + 0.5);
        sdlTimer->setInterval(preDelay);
        // 释放
        av_free(out_buffer);
        av_frame_free(&frameYUV);
    }
    else
    {
        av_frame_free(&frame);
        // 绘制封面
        paint(frameYUV);
        // 释放
        av_free(out_buffer);
        av_frame_free(&frameYUV);
    }
}

void SDLPaint::paint(AVFrame *frameYUV)
{
    SDL_UpdateTexture(sdlTexture, nullptr, frameYUV->data[0], frameYUV->linesize[0]);
    SDL_RenderClear(sdlRenderer);
    //SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
    SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
    SDL_RenderPresent(sdlRenderer);
}

void SDLPaint::pause()
{
    if(nullptr != sdlTimer)
    {
        sdlTimer->stop();
    }
}

void SDLPaint::stop()
{
    // 停止定时器
    pause();
    // 渲染黑色图像
    if(nullptr != sdlRenderer)
    {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderPresent(sdlRenderer);
        // 销毁渲染器
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }
    // 销毁纹理
    if(nullptr != sdlTexture)
    {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = nullptr;
    }
    if(nullptr != screen)
    {
        SDL_DestroyWindow(screen);
        screen = nullptr;
    }
    // 退出sdl
    SDL_Quit();
    // 关闭图像处理上下文
    if(nullptr != pSwsCtx)
    {
        sws_freeContext(pSwsCtx);
        pSwsCtx = nullptr;
    }
    if(nullptr != sdlTimer)
    {
        delete sdlTimer;
        sdlTimer = nullptr;
    }
}

void SDLPaint::reStart()
{
    if(nullptr != sdlTimer)
    {
        sdlTimer->start(preDelay);
    }
}

//int SDLPaint::sfp_signal_thread(void *opaque)
//{
//    double *avg_frame_rate = (double*)opaque;
//    //    qDebug() << *avg_frame_rate;
//    //    curMediaStatus = 1;
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
