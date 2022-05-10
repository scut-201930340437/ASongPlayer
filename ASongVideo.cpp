#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "DataSink.h"

QAtomicPointer<ASongVideo> ASongVideo:: _instance = nullptr;
QMutex ASongVideo::_mutex;

ASongVideo* ASongVideo::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        //        qDebug() << "----";
        _instance.testAndSetOrdered(nullptr, new ASongVideo);
    }
    return _instance;
}

// 初始化参数
void ASongVideo::setMetaData(AVCodecContext *_pCodecCtx, const int _videoIdx, const AVRational timeBase, bool _hasCover)
{
    pCodecCtx = _pCodecCtx;
    videoIdx = _videoIdx;
    tb = av_q2d(timeBase);
    lastFrameDelay = tb;
    //    frameRate = _frameRate;
    srcWidth = pCodecCtx->width;
    srcHeight = pCodecCtx->height;
    hasCover = _hasCover;
}

/*thread*/
void ASongVideo::start(Priority pro)
{
    allowRunVideo = true;
    frameTime = av_gettime() / 1000000.0;
    //    qDebug() << "videoThread begin";
    QThread::start(pro);
}

void ASongVideo::run()
{
    AVPacket *packet = nullptr;
    bool coverAlready = false;
    while(allowRunVideo)
    {
        if(DataSink::getInstance()->frameListSize(1) >= DataSink::maxFrameListLength)
        {
            msleep(20);
        }
        else
        {
            packet = DataSink::getInstance()->takeNextPacket(1);
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
        }
        if(coverAlready)
        {
            allowRunVideo = false;
            break;
        }
    }
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
    double frameDelay;
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
    frameDelay = tb;
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
    // 获取音频时钟
    double audioClock = ASongAudio::getInstance()->getAudioClock();
    // 计算视频帧pts和音频时钟的差
    double diff = pts - audioClock;
    // 计算最小刷新时间
    double minFlushT = fmax(delay, synLowerBound);
    // 没有超过非同步阈值则可以同步
    if(fabs(diff) < noSynUpperBound)
    {
        // 视频慢于音频的时间超过最小刷新时间，降低延迟
        if(diff <= -minFlushT)
        {
            delay = 0.0;
            //            qDebug() << 0;
        }
        else
        {
            // 视频快于音频的时间超过最小刷新时间，增加延迟
            if(diff >= minFlushT)
            {
                delay *= 2.0;
                //                qDebug() << 1;
            }
        }
    }
    frameTime += delay;
    // 真正延时时间
    //    qDebug() << tmp;
    double actualDelay = fmax(frameTime - av_gettime() / 1000000.0, synLowerBound);
    qDebug() << actualDelay;
    //        actualDelay = fmax(actualDelay, 0.010);
    return actualDelay;
}

//void ASongVideo::resetWH(const int _out_width, const int _out_height)
//{
//    out_width = _out_width;
//    out_height = _out_height;
//}

void ASongVideo::pause()
{
    if(isRunning())
    {
        allowRunVideo = false;
        wait();
    }
    //    return true;
}

void ASongVideo::stop()
{
    pause();
    // 关闭视频解码器上下文
    if(nullptr != pCodecCtx)
    {
        avcodec_close(pCodecCtx);
    }
    pCodecCtx = nullptr;
    videoClock = 0.0;
    // frameTime
    frameTime = 0.0;
    // 上一帧pts
    lastFramePts = 0.0;
    // 上一帧delay
    lastFrameDelay = 0.0;
    // stream_index
    videoIdx = -1;
    // 源视频流的宽高
    srcWidth = 0, srcHeight = 0;
}
