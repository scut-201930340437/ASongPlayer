#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(ASongVideo, asongVideo)

ASongVideo* ASongVideo::getInstance()
{
    return asongVideo;
}

// 初始化参数
void ASongVideo::setMetaData(AVCodecContext *_pCodecCtx, const AVRational timeBase)
{
    pCodecCtx = _pCodecCtx;
    tb = av_q2d(timeBase);
    lastFrameDelay = tb;
    AV_SYNC_THRESHOLD_MAX = 1.5 * tb;
}

double ASongVideo::getPts(AVFrame *frame)
{
    double pts = 0.0;
    if((pts = frame->best_effort_timestamp) == AV_NOPTS_VALUE)
    {
        pts = 0.0;
    }
    pts *= tb;
    caliBratePts(frame, pts);
    return pts;
}
// 修正pts
void ASongVideo::caliBratePts(AVFrame *frame, double &pts)
{
    // 如果通过frame->best_effort_timestamp获取到了最合适的clock，更新videoClock
    if(pts != 0.0)
    {
        videoClock = pts;
    }
    // 如果没有，使用当前的videoClock(这是上一帧getPts时预测下一帧的播放时间点)
    else
    {
        pts = videoClock;
    }
    double frameDelay = tb;
    // extra_delay = repeat_pict / (2*fps)
    frameDelay *= (1.0 + 0.5 * frame->repeat_pict);
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        videoClock -= frameDelay;
    }
    else
    {
        videoClock += frameDelay;
    }
    // 此时videoClock实际上也是下一帧的时间点
}

// 同步
double ASongVideo::synVideo(const double pts)
{
    double delay = pts - lastFramePts;
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        delay = -delay;
    }
    // 延时太小或太大都不正常，使用上一帧计算的delay
    if(delay <= 0.0 || delay  >= 1.0)
    {
        delay = lastFrameDelay;
    }
    // 更新参数
    lastFramePts = pts;
    lastFrameDelay = delay;
    // 计算同步阈值
    double syn_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN / ASongFFmpeg::getInstance()->getSpeed(),
                                 FFMIN(AV_SYNC_THRESHOLD_MAX / ASongFFmpeg::getInstance()->getSpeed(), delay));
    // 计算视频帧pts和音频时钟的差
    double diff = pts - ASongAudio::getInstance()->getAudioClock();
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        diff = -diff;
    }
    // 没有超过非同步阈值则可以同步
    if(fabs(diff) < FFMAX(AV_NOSYNC * ASongFFmpeg::getInstance()->getSpeed(), 12.0))
    {
        // 视频慢于音频的时间超过同步阈值，降低延迟
        if(diff <= -syn_threshold)
        {
            delay = FFMAX(0.0, delay + diff);
        }
        else
        {
            // 视频快于音频的时间超过同步阈值，增加延迟
            if(diff >= syn_threshold)
            {
                if(delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                {
                    delay += diff;
                }
                else
                {
                    delay *= 2.0;
                }
            }
        }
        return delay;
    }
    else
    {
        return 0.0;
    }
}

void ASongVideo::resetPara()
{
    stopReq = false;
    pauseReq = false;
    pauseFlag = false;
    videoClock = 0.0;
    // 上一帧pts
    lastFramePts = 0.0;
    // 上一帧delay
    lastFrameDelay = 0.0;
}

/*thread*/
void ASongVideo::start(Priority pri)
{
    resetPara();
    QThread::start(pri);
}

void ASongVideo::stop()
{
    // 先结束视频解码线程
    if(QThread::isRunning())
    {
        QMutexLocker locker(&pauseMutex);
        stopReq = true;
        locker.unlock();
        // 可能处于暂停中，先唤醒
        pauseCond.wakeAll();
        QThread::wait();
    }
    // 关闭视频解码器上下文
    if(nullptr != pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
    }
}

void ASongVideo::pauseThread()
{
    QMutexLocker locker(&pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&pauseMutex);
        locker.relock();
    }
}

void ASongVideo::resumeThread()
{
    QMutexLocker locker(&pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        pauseReq = false;
        pauseCond.wakeAll();
        pauseCond.wait(&pauseMutex);
    }
}

void ASongVideo::setSleepTime(int _sleepTime)
{
    sleepTime = _sleepTime;
}

void ASongVideo::run()
{
    AVPacket *packet = nullptr;
    invertFrameList = new QList<AVFrame*>;
    std::atomic_bool coverAlready = false;
    for(;;)
    {
        if(stopReq)
        {
            AVFrame *frame = nullptr;
            while(!invertFrameList->isEmpty())
            {
                frame = invertFrameList->takeFirst();
                av_frame_free(&frame);
            }
            invertFrameList->clear();
            delete invertFrameList;
            invertFrameList = nullptr;
            stopReq = false;
            break;
        }
        if(pauseReq)
        {
            QMutexLocker locker(&pauseMutex);
            if(stopReq)
            {
                continue;
            }
            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 解码线程阻塞
            pauseCond.wait(&pauseMutex);
            locker.relock();
            pauseFlag = false;
            // 唤醒主线程
            pauseCond.wakeAll();
        }
        if(!ASongFFmpeg::getInstance()->invertFlag && DataSink::getInstance()->allowAddFrame(1) ||
                ASongFFmpeg::getInstance()->invertFlag && DataSink::getInstance()->allowAddInvertFrameList(1))
        {
            packet = DataSink::getInstance()->takeNextPacket(1);
            if(nullptr != packet)
            {
                // flushpkt
                if(packet->data == (uint8_t*)packet)
                {
                    avcodec_flush_buffers(pCodecCtx);
                    av_packet_free(&packet);
                    continue;
                }
                // 解码
                int ret = avcodec_send_packet(pCodecCtx, packet);
                // avcodec_send_packet成功
                if(ret == 0)
                {
                    AVFrame *frame = av_frame_alloc();
                    ret = avcodec_receive_frame(pCodecCtx, frame);
                    if(ret == 0)
                    {
                        appendFrame(frame);
                        if(nullptr == packet->data && packet->size == 0)
                        {
                            // 放入结束帧
                            AVFrame *endFrame = av_frame_alloc();
                            endFrame->pts = -1;
                            DataSink::getInstance()->appendFrame(1, endFrame);
                            pauseReq = true;
                        }
                        // 如果是带音频的封面，该线程只做一次循环
                        if(ASongFFmpeg::getInstance()->hasCover)
                        {
                            coverAlready = true;
                        }
                    }
                    else
                    {
                        if(ret == AVERROR_EOF)
                        {
                            // 复位解码器
                            avcodec_flush_buffers(pCodecCtx);
                        }
                        av_frame_free(&frame);
                    }
                }
                else
                {
                    // 如果是AVERROR(EAGAIN)，需要先调用avcodec_receive_frame将frame读取出来
                    if(ret == AVERROR(EAGAIN))
                    {
                        while(1)
                        {
                            AVFrame *frame = av_frame_alloc();
                            ret = avcodec_receive_frame(pCodecCtx, frame);
                            if(ret == AVERROR_EOF)
                            {
                                av_frame_free(&frame);
                                // 复位解码器
                                avcodec_flush_buffers(pCodecCtx);
                                break;
                            }
                            appendFrame(frame);
                        }
                        // 然后再调用avcodec_send_packet
                        ret = avcodec_send_packet(pCodecCtx, packet);
                        if(ret == 0)
                        {
                            AVFrame *frame = av_frame_alloc();
                            ret = avcodec_receive_frame(pCodecCtx, frame);
                            if(ret == 0)
                            {
                                appendFrame(frame);
                                if(nullptr == packet->data && packet->size == 0)
                                {
                                    // 放入结束帧
                                    AVFrame *endFrame = av_frame_alloc();
                                    endFrame->pts = -1;
                                    DataSink::getInstance()->appendFrame(1, endFrame);
                                    pauseReq = true;
                                }
                            }
                            else
                            {
                                if(ret == AVERROR_EOF)
                                {
                                    // 复位解码器
                                    avcodec_flush_buffers(pCodecCtx);
                                }
                                av_frame_free(&frame);
                            }
                        }
                    }
                }
                // 释放
                av_packet_free(&packet);
                if(coverAlready)
                {
                    stopReq = true;
                }
            }
            else
            {
                msleep(5);
            }
        }
        else
        {
            msleep(sleepTime);
        }
    }
}

void ASongVideo::appendFrame(AVFrame *frame)
{
    // 处于倒放
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        if(frame->pts * tb < ASongFFmpeg::getInstance()->invertPts)
        {
            frame->opaque = (double*)new double(getPts(frame));
            invertFrameList->append(frame);
        }
        else
        {
            // 释放该帧
            av_frame_free(&frame);
            if(!invertFrameList->isEmpty())
            {
                DataSink::getInstance()->appendInvertFrameList(1, invertFrameList);
                ASongFFmpeg::getInstance()->invertPts = invertFrameList->first()->pts * tb;
                invertFrameList = new QList<AVFrame*>;
            }
            else
            {
                // 倒放seek不到帧，再向前倒退一个pts
                ASongFFmpeg::getInstance()->invertPts -= SDLPaint::getInstance()->basePts;
            }
            ASongFFmpeg::getInstance()->needInvertSeek = true;
        }
    }
    else
    {
        frame->opaque = (double*)new double(getPts(frame));
        // 逐帧时只加入所需帧
        if(ASongFFmpeg::getInstance()->stepSeek)
        {
            if((*(double*)frame->opaque) >= ASongFFmpeg::getInstance()->targetPts - 0.5 * SDLPaint::getInstance()->basePts
                    && (*(double*)frame->opaque) <= ASongFFmpeg::getInstance()->targetPts + 0.6 * SDLPaint::getInstance()->basePts)
            {
                DataSink::getInstance()->appendFrame(1, frame);
                pauseReq = true;
            }
            else
            {
                av_frame_free(&frame);
            }
        }
        else
        {
            DataSink::getInstance()->appendFrame(1, frame);
        }
    }
}

