#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(ASongVideo, asongVideo)

//QAtomicPointer<ASongVideo> ASongVideo:: _instance = nullptr;
//QMutex ASongVideo::_mutex;

ASongVideo* ASongVideo::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    //    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    //        //        qDebug() << "----";
    //        _instance.testAndSetOrdered(nullptr, new ASongVideo);
    //    }
    return asongVideo;
}

// 初始化参数
void ASongVideo::setMetaData(AVCodecContext *_pCodecCtx, const int _videoIdx, const AVRational timeBase, bool _hasCover)
{
    pCodecCtx = _pCodecCtx;
    videoIdx = _videoIdx;
    tb = av_q2d(timeBase);
    lastFrameDelay = tb;
    AV_SYNC_THRESHOLD_MAX = 1.5 * tb;
    hasCover = _hasCover;
}

//void ASongVideo::setNeededVideoCode()
//{
//    neededVideoCode = true;
//}

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
    // frameDelay = 1/fps+extra_delay
    frameDelay *= (1.0 + 0.5 * frame->repeat_pict);
    videoClock += frameDelay; // 此时videoClock实际上也是下一帧的时间点
}

// 同步
double ASongVideo::synVideo(const double pts)
{
    //    double pts = *((double*)frame->opaque);
    //    qDebug() << pts;
    double delay = pts - lastFramePts;
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
    //    qDebug() << diff;
    // 没有超过非同步阈值则可以同步
    if(fabs(diff) < FFMAX(noSynUpperBound * ASongFFmpeg::getInstance()->getSpeed(), 12.0))
    {
        // 视频慢于音频的时间超过同步阈值，降低延迟
        if(diff <= -syn_threshold)
        {
            delay = FFMAX(0.0, delay + diff);
        }
        else
        {
            // 视频快于音频的时间超过最小刷新时间，增加延迟
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
        frameTimer += delay;
        return delay;
    }
    // 视频时钟与音频时钟相差太大，重新获取系统时间
    else
    {
        frameTimer = av_gettime_relative() / 1000000.0;
        return 0.0;
    }
    // 真正延时时间
    //    double actualDelay = fmax(frameTimer - av_gettime_relative() / 1000000.0, synLowerBound);
}

//void ASongVideo::resetWH(const int _out_width, const int _out_height)
//{
//    out_width = _out_width;
//    out_height = _out_height;
//}

/*thread*/
void ASongVideo::start(Priority pri)
{
    stopReq = false;
    stopFlag = false;
    pauseReq = false;
    pauseFlag = false;
    frameTimer = av_gettime_relative() / 1000000.0;
    QThread::start(pri);
}

void ASongVideo::stop()
{
    // 先结束视频解码线程
    if(QThread::isRunning())
    {
        stopReq = true;
        //        needPaused = false;
        //        pauseFlag = false;
        // 解码线程可能因为未渲染的frame队列过长而阻塞，先唤醒
        //        DataSink::getInstance()->wakeVideoWithFraCond();
        // 可能处于暂停中，先唤醒
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    // 关闭视频解码器上下文
    if(nullptr != pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
    }
    videoClock = 0.0;
    // frameTimer
    frameTimer = 0.0;
    // 上一帧pts
    lastFramePts = 0.0;
    // 上一帧delay
    lastFrameDelay = 0.0;
    // stream_index
    videoIdx = -1;
    // 源视频流的宽高
    //    srcWidth = 0, srcHeight = 0;
    //    qDebug() << 3;
}

//void ASongVideo::pause()
//{
//    QMutexLocker locker(&_pauseMutex);
//    if(!pauseFlag && QThread::isRunning())
//    {
//        needPaused = true;
//        // 解码线程可能因为未渲染的frame队列过长而阻塞，先唤醒
//        DataSink::getInstance()->wakeVideoWithFraCond();
//        pauseCond.wait(&_pauseMutex);
//        locker.relock();
//    }
//    qDebug() << 3;
//}

void ASongVideo::resume()
{
    frameTimer = av_gettime_relative() / 1000000.0; // 更新起始时间基准
    //    QMutexLocker locker(&_pauseMutex);
    //    if(pauseFlag && QThread::isRunning())
    //    {
    //        needPaused = false;
    //        pauseCond.wakeAll();
    //        pauseCond.wait(&_pauseMutex);
    //    }
}

void ASongVideo::pauseThread()
{
    QMutexLocker locker(&_pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&_pauseMutex);
        locker.relock();
    }
}

void ASongVideo::resumeThread()
{
    QMutexLocker locker(&_pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        pauseReq = false;
        pauseCond.wakeAll();
        pauseCond.wait(&_pauseMutex);
    }
    else if(QThread::isFinished())
    {
        start();
    }
}

void ASongVideo::run()
{
    //    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    AVPacket *packet = nullptr;
    std::atomic_bool coverAlready = false;
    for(;;)
    {
        if(stopReq)
        {
            stopReq = false;
            break;
        }
        if(pauseReq)
        {
            QMutexLocker locker(&_pauseMutex);
            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 音频解码线程阻塞
            pauseCond.wait(&_pauseMutex);
            locker.relock();
            pauseFlag = false;
            // 唤醒主线程
            pauseCond.wakeAll();
        }
        //        qDebug() << "start";
        if(DataSink::getInstance()->allowAddVFrame())
        {
            packet = DataSink::getInstance()->takeNextPacket(1);
            if(nullptr != packet)
            {
                // flushpkt
                if(packet == ASongFFmpeg::getInstance()->flushPacket)
                {
                    // 有seek操作
                    avcodec_flush_buffers(pCodecCtx);
                    continue;
                }
                // 解码
                int ret = avcodec_send_packet(pCodecCtx, packet);
                // avcodec_send_packet成功
                if(ret == 0)
                {
                    //                while(1)
                    //                {
                    AVFrame *frame = av_frame_alloc();
                    ret = avcodec_receive_frame(pCodecCtx, frame);
                    if(ret == 0)
                    {
                        frame->opaque = (double*)new double(getPts(frame));
                        // 扔掉小于seek的目标pts的帧
                        //                        if(ASongFFmpeg::getInstance()->seekVideo)
                        //                        {
                        //                            if(videoClock < ASongFFmpeg::getInstance()->seekTime)
                        //                            {
                        //                                av_packet_free(&packet);
                        //                                av_frame_free(&frame);
                        //                                continue;
                        //                            }
                        //                            else
                        //                            {
                        //                                ASongFFmpeg::getInstance()->seekVideo = false;
                        //                            }
                        //                        }
                        //
                        DataSink::getInstance()->appendFrameList(1, frame);
                        // 如果是带音频的封面，该线程只做一次循环
                        if(hasCover)
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
                            frame->opaque = (double*)new double(getPts(frame));
                            // 扔掉小于seek的目标pts的帧
                            //                            if(ASongFFmpeg::getInstance()->seekVideo)
                            //                            {
                            //                                if(videoClock < ASongFFmpeg::getInstance()->seekTime)
                            //                                {
                            //                                    av_frame_free(&frame);
                            //                                    continue;
                            //                                }
                            //                                else
                            //                                {
                            //                                    ASongFFmpeg::getInstance()->seekVideo = false;
                            //                                }
                            //                            }
                            //
                            DataSink::getInstance()->appendFrameList(1, frame);
                        }
                        // 然后再调用avcodec_send_packet
                        ret = avcodec_send_packet(pCodecCtx, packet);
                        if(ret == 0)
                        {
                            AVFrame *frame = av_frame_alloc();
                            ret = avcodec_receive_frame(pCodecCtx, frame);
                            if(ret == 0)
                            {
                                frame->opaque = (double*)new double(getPts(frame));
                                // 扔掉小于seek的目标pts的帧
                                //                                if(ASongFFmpeg::getInstance()->seekVideo)
                                //                                {
                                //                                    if(videoClock < ASongFFmpeg::getInstance()->seekTime)
                                //                                    {
                                //                                        av_packet_free(&packet);
                                //                                        av_frame_free(&frame);
                                //                                        continue;
                                //                                    }
                                //                                    else
                                //                                    {
                                //                                        ASongFFmpeg::getInstance()->seekVideo = false;
                                //                                    }
                                //                                }
                                //
                                DataSink::getInstance()->appendFrameList(1, frame);
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
                //        }
                if(coverAlready)
                {
                    stopReq = true;
                    break;
                }
            }
            else
            {
                QMutexLocker locker(&ASongFFmpeg::getInstance()->stopMutex);
                if(ASongFFmpeg::getInstance()->stopFlag)
                {
                    locker.unlock();
                    stopReq = true;
                }
                else
                {
                    locker.unlock();
                    msleep(5);
                }
            }
        }
        else
        {
            msleep(60);
        }
    }
}
