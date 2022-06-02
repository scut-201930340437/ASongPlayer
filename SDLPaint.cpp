#include "ASongFFmpeg.h"
#include "ASongVideo.h"
#include "ASongAudioOutput.h"
#include "DataSink.h"
#include "SDLPaint.h"

Q_GLOBAL_STATIC(SDLPaint, sdlPaint)

SDLPaint::~SDLPaint()
{
    if(nullptr != sdlTimer)
    {
        sdlTimer->stop();
        delete sdlTimer;
    }
}

SDLPaint* SDLPaint::getInstance()
{
    return sdlPaint;
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
    sdlSurface = SDL_GetWindowSurface(screen);
    if(nullptr == sdlSurface)
    {
        qDebug() << "SDL: could not create surface - exiting:" << SDL_GetError();
        return -1;
    }
    //
    lastScreenWidth = sdlSurface->w;
    lastScreenHeight = sdlSurface->h;
    //
    sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    if(nullptr == sdlRenderer)
    {
        qDebug() << "create rendered failed";
        return -1;
    }
    // 创建纹理
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, srcWidth, srcHeight);
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
        qDebug() << "sdl 208:swsGetCtx failed";
        return -1;
    }
    // sar
    AVFormatContext *pFormatCtx = ASongFFmpeg::getInstance()->pFormatCtx;
    int idx = ASongFFmpeg::getInstance()->mediaMetaData->vMetaDatas[ASongFFmpeg::getInstance()->videoIdx].idx;
    // 获取采样纵横比
    sar = av_guess_sample_aspect_ratio(pFormatCtx, pFormatCtx->streams[idx], nullptr);
    calDisplayRect(&sdlRect, 0, 0, lastScreenWidth, lastScreenHeight, srcWidth, srcHeight);
    // 初始化各变量
    pauseFlag = false;
    basePts = 0.0;
    curPts = 0.0;
    curFrameNum = -1;
    // 开启定时器
    sdlTimer = new QTimer;
    connect(sdlTimer, &QTimer::timeout, this, &SDLPaint::getFrameYUV);
    if(frameRate != -1)
    {
        preDelay = int(1000.0 / frameRate + 0.5);
    }
    else
    {
        preDelay = 40;
    }
    sdlTimer->start(preDelay);
    return 0;
}

void SDLPaint::setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt, const AVRational time_base)
{
    // 设置参数
    srcWidth = width;
    srcHeight = height;
    frameRate = _frameRate;
    pix_fmt = _pix_fmt;
    tb = av_q2d(time_base);
}

void SDLPaint::calDisplayRect(SDL_Rect *rect,
                              int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                              int pic_width, int pic_height)
{
    // 视频的SAR 采样纵横比
    AVRational aspect_ratio = sar;
    int64_t width, height, x, y;
    if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
    {
        aspect_ratio = av_make_q(1, 1);
    }
    // aspect_ratio 输出的宽高比 = 采样纵横比aspect_ratio * 视频width / 视频height
    aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));
    height = scr_height;
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
    if (width > scr_width)
    {
        width = scr_width;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
    }
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop  + y;
    rect->w = FFMAX((int)width,  1);
    rect->h = FFMAX((int)height, 1);
}

// 转换为YUV图像并进行同步
void SDLPaint::getFrameYUV()
{
    // 没有暂停
    if(!pauseFlag)
    {
        AVFrame *frame = nullptr;
        // 取一帧
        if(ASongFFmpeg::getInstance()->invertFlag)
        {
            frame = DataSink::getInstance()->takeInvertFrame(1);
        }
        else
        {
            frame = DataSink::getInstance()->takeNextFrame(1);
        }
        if(nullptr != frame)
        {
            // 扔掉小于stepSeek的目标帧号的帧
            if(ASongFFmpeg::getInstance()->stepSeek && ASongFFmpeg::getInstance()->seekVideo)
            {
                if(ASongFFmpeg::getInstance()->_step > 1)
                {
                    av_frame_free(&frame);
                    --ASongFFmpeg::getInstance()->_step;
                    return;
                }
                else
                {
                    if(ASongFFmpeg::getInstance()->_step == 1)
                    {
                        ASongFFmpeg::getInstance()->seekVideo = false;
                    }
                    else
                    {
                        if(frame->pts * tb < ASongFFmpeg::getInstance()->targetPts - 0.5 * basePts)
                        {
                            av_frame_free(&frame);
                            return;
                        }
                        else
                        {
                            ASongFFmpeg::getInstance()->seekVideo = false;
                        }
                    }
                }
            }
            // 更新最近一帧的pts
            if(basePts >= -DBL_EPSILON && basePts <= DBL_EPSILON)
            {
                basePts = frame->pts * tb;
            }
            curPts = frame->pts * tb;
            curFrameNum = curPts / basePts;
            // 适应窗口--begin
            // 计算rect参数
            sdlSurface = SDL_GetWindowSurface(screen);
            if(sdlSurface->w != lastScreenWidth || sdlSurface->h != lastScreenHeight)
            {
                calDisplayRect(&sdlRect, 0, 0, sdlSurface->w, sdlSurface->h, frame->width, frame->height);
                lastScreenWidth = sdlSurface->w;
                lastScreenHeight = sdlSurface->h;
            }
            // 适应窗口--end
            // 同步
            double actualDelay = ASongVideo::getInstance()->synVideo(*((double*)frame->opaque));
            // 倍速>=8，丢帧（仅对视频）
            if(!ASongFFmpeg::getInstance()->hasCover && ASongFFmpeg::getInstance()->getSpeed() >= 7.9999 && actualDelay <= 0.0001)
            {
                av_frame_free(&frame);
                preDelay = 1;
                sdlTimer->setInterval(preDelay);
                return;
            }
            else
            {
                AVFrame *frameYUV = av_frame_alloc();
                uint8_t *cur_out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                          srcWidth, srcHeight, 1));
                av_image_fill_arrays(frameYUV->data, frameYUV->linesize,
                                     cur_out_buffer, AV_PIX_FMT_YUV420P,
                                     srcWidth, srcHeight, 1);
                sws_scale(pSwsCtx, (const uint8_t* const*)frame->data,
                          frame->linesize, 0, srcHeight,
                          frameYUV->data, frameYUV->linesize);
                if(frameRate != -1)
                {
                    // 绘制
                    paint(frameYUV);
                    // 对于视频，需要重设延时
                    preDelay = int(actualDelay * 1000.0 + 0.5);
                    sdlTimer->setInterval(preDelay);
                }
                else
                {
                    // 绘制
                    paint(frameYUV);
                }
                // 同步后释放该帧
                av_frame_free(&frame);
                // 修改preFrame
                if(nullptr != pre_out_buffer)
                {
                    av_free(pre_out_buffer);
                    pre_out_buffer = nullptr;
                }
                if(nullptr != preFrame)
                {
                    av_frame_free(&preFrame);
                    preFrame = nullptr;
                }
                pre_out_buffer = cur_out_buffer;
                preFrame = frameYUV;
            }
        }
        else
        {
            // 音频播放线程结束且拿不到frame，此时说明播放结束
            if(ASongAudioOutput::getInstance()->isFinished())
            {
                stop();
            }
            // 播放未结束但是拿不到frame，渲染上一帧
            else
            {
                if(nullptr != preFrame)
                {
                    // 适应窗口--begin
                    // 计算rect参数
                    sdlSurface = SDL_GetWindowSurface(screen);
                    if(sdlSurface->w != lastScreenWidth || sdlSurface->h != lastScreenHeight)
                    {
                        calDisplayRect(&sdlRect, 0, 0, sdlSurface->w, sdlSurface->h, srcWidth, srcHeight);
                        lastScreenWidth = sdlSurface->w;
                        lastScreenHeight = sdlSurface->h;
                    }
                    // 适应窗口--end
                    paint(preFrame);
                }
            }
        }
    }
    else
    {
        if(nullptr != preFrame)
        {
            // 适应窗口--begin
            // 计算rect参数
            sdlSurface = SDL_GetWindowSurface(screen);
            if(sdlSurface->w != lastScreenWidth || sdlSurface->h != lastScreenHeight)
            {
                calDisplayRect(&sdlRect, 0, 0, sdlSurface->w, sdlSurface->h, srcWidth, srcHeight);
                lastScreenWidth = sdlSurface->w;
                lastScreenHeight = sdlSurface->h;
            }
            // 适应窗口--end
            paint(preFrame);
        }
    }
}

void SDLPaint::paint(AVFrame *frameYUV)
{
    if (frameYUV->linesize[0] > 0 && frameYUV->linesize[1] > 0 && frameYUV->linesize[2] > 0)
    {
        SDL_UpdateYUVTexture(sdlTexture, NULL, frameYUV->data[0], frameYUV->linesize[0],
                             frameYUV->data[1], frameYUV->linesize[1],
                             frameYUV->data[2], frameYUV->linesize[2]);
    }
    else if (frameYUV->linesize[0] < 0 && frameYUV->linesize[1] < 0 && frameYUV->linesize[2] < 0)
    {
        SDL_UpdateYUVTexture(sdlTexture, NULL, frameYUV->data[0] + frameYUV->linesize[0] * (frameYUV->height - 1), -frameYUV->linesize[0],
                             frameYUV->data[1] + frameYUV->linesize[1] * (AV_CEIL_RSHIFT(frameYUV->height, 1) - 1), -frameYUV->linesize[1],
                             frameYUV->data[2] + frameYUV->linesize[2] * (AV_CEIL_RSHIFT(frameYUV->height, 1) - 1), -frameYUV->linesize[2]);
    }
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopyEx(sdlRenderer, sdlTexture, NULL, &sdlRect, 0.0, NULL,  SDL_FLIP_NONE);
    SDL_RenderPresent(sdlRenderer);
}

void SDLPaint::stop()
{
    // 停止定时器
    if(nullptr != sdlTimer)
    {
        sdlTimer->stop();
    }
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
    // 释放preFrame
    if(nullptr != pre_out_buffer)
    {
        av_free(pre_out_buffer);
        pre_out_buffer = nullptr;
    }
    if(nullptr != preFrame)
    {
        av_frame_free(&preFrame);
        preFrame = nullptr;
    }
    if(nullptr != sdlTimer)
    {
        delete sdlTimer;
        sdlTimer = nullptr;
    }
}

void SDLPaint::pause()
{
    pauseFlag = true;
}

void SDLPaint::resume()
{
    pauseFlag = false;
}

void SDLPaint::restartTimer()
{
    if(nullptr != sdlTimer)
    {
        sdlTimer->start(preDelay);
    }
}

void SDLPaint::stopTimer()
{
    if(nullptr != sdlTimer)
    {
        sdlTimer->stop();
    }
}

