#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongAudioOutput.h"
#include "DataSink.h"

Q_GLOBAL_STATIC(ASongAudio, asongAudio)

// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    return asongAudio;
}

// 初始化音频设备参数
void ASongAudio::initParaAndSwr()
{
    ASongAudioOutput::getInstance()->initAudioPara(pCodecCtx->channels, pCodecCtx->sample_rate,
            pCodecCtx->channel_layout, pCodecCtx->sample_fmt, tb);
    ASongAudioOutput::getInstance()->initSwr();
}

/*设置成员变量*/
void ASongAudio::setMetaData(AVCodecContext * _pCodecCtx, const AVRational timeBase)
{
    pCodecCtx = _pCodecCtx;
    tb = av_q2d(timeBase);
}
// 设置时钟
void ASongAudio::setAudioClock(AVFrame * frame, const double duration)
{
    if(frame->pts != AV_NOPTS_VALUE)
    {
        audioClock = frame->pts * tb;
    }
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        audioClock -= duration;
    }
    else
    {
        audioClock += duration;
    }
}
//获取时钟
double ASongAudio::getAudioClock()
{
    return audioClock;
}

void ASongAudio::resetPara()
{
    stopReq = false;
    pauseReq = false;
    pauseFlag = false;
    audioClock = 0.0;
}

void ASongAudio::start(Priority pri)
{
    resetPara();
    QThread::start(pri);
}

void ASongAudio::stop()
{
    // 结束音频解码线程
    if(QThread::isRunning())
    {
        stopReq = true;
        // 可能暂停中，先唤醒
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    // 关闭解码器上下文
    if(nullptr != pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
    }
}

void ASongAudio::pauseThread()
{
    QMutexLocker locker(&pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&pauseMutex);
        locker.relock();
    }
}

void ASongAudio::resumeThread()
{
    QMutexLocker locker(&pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        pauseReq = false;
        pauseCond.wakeAll();
        pauseCond.wait(&pauseMutex);
    }
    else if(QThread::isFinished())
    {
        start();
    }
}

void ASongAudio::setSleepTime(int _sleepTime)
{
    sleepTime = _sleepTime;
}

void ASongAudio::run()
{
    AVPacket *packet = nullptr;
    invertFrameList = new QList<AVFrame*>;
    for(;;)
    {
        if(stopReq)
        {
            stopReq = false;
            AVFrame *frame = nullptr;
            while(!invertFrameList->isEmpty())
            {
                frame = invertFrameList->takeFirst();
                av_frame_free(&frame);
            }
            invertFrameList->clear();
            delete invertFrameList;
            invertFrameList = nullptr;
            break;
        }
        if(pauseReq)
        {
            QMutexLocker locker(&pauseMutex);
            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 音频解码线程阻塞
            pauseCond.wait(&pauseMutex);
            locker.relock();
            pauseFlag = false;
            // 唤醒主线程
            pauseCond.wakeAll();
        }
        if(!ASongFFmpeg::getInstance()->invertFlag && DataSink::getInstance()->allowAddFrame(0) ||
                ASongFFmpeg::getInstance()->invertFlag && DataSink::getInstance()->allowAddInvertFrameList(0))
        {
            packet = DataSink::getInstance()->takeNextPacket(0);
            if(nullptr != packet)
            {
                // flushpkt
                if(packet->data == (uint8_t*)packet)
                {
                    avcodec_flush_buffers(pCodecCtx);
                    av_packet_free(&packet);
                    continue;
                }
                int ret = avcodec_send_packet(pCodecCtx, packet);
                // avcodec_send_packet成功
                if(ret == 0)
                {
                    while(1)
                    {
                        AVFrame *frame = av_frame_alloc();
                        ret = avcodec_receive_frame(pCodecCtx, frame);
                        if(ret == 0)
                        {
                            appendFrame(frame);
                        }
                        else
                        {
                            if(ret == AVERROR_EOF)
                            {
                                // 复位解码器
                                avcodec_flush_buffers(pCodecCtx);
                            }
                            av_frame_free(&frame);
                            break;
                        }
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
                            while(1)
                            {
                                AVFrame *frame = av_frame_alloc();
                                ret = avcodec_receive_frame(pCodecCtx, frame);
                                if(ret == 0)
                                {
                                    appendFrame(frame);
                                }
                                else
                                {
                                    if(ret == AVERROR_EOF)
                                    {
                                        // 复位解码器
                                        avcodec_flush_buffers(pCodecCtx);
                                    }
                                    av_frame_free(&frame);
                                    break;
                                }
                            }
                        }
                    }
                }
                // 释放
                av_packet_free(&packet);
            }
            else
            {
                if(ASongFFmpeg::getInstance()->isFinished())
                {
                    stopReq = true;
                }
                else
                {
                    msleep(5);
                }
            }
        }
        else
        {
            msleep(sleepTime);
        }
    }
}

void ASongAudio::appendFrame(AVFrame *frame)
{
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        if(frame->pts * tb < ASongFFmpeg::getInstance()->invertPts)
        {
            invertFrameList->append(frame);
        }
        else
        {
            av_frame_free(&frame);
            if(!invertFrameList->isEmpty())
            {
                DataSink::getInstance()->appendInvertFrameList(0, invertFrameList);
                invertFrameList = new QList<AVFrame*>;
            }
        }
    }
    else
    {
        DataSink::getInstance()->appendFrame(0, frame);
        // 逐帧时只解码到所需帧
        if(ASongFFmpeg::getInstance()->stepSeek
                && frame->pts * tb >= ASongFFmpeg::getInstance()->targetPts - 0.5 * ASongAudioOutput::getInstance()->basePts
                && frame->pts * tb <= ASongFFmpeg::getInstance()->targetPts + 0.6 * ASongAudioOutput::getInstance()->basePts)
        {
            pauseReq = true;
        }
    }
}

void ASongAudio::setVolume(int volume)
{
    curVolume = volume / volTranRate;
    ASongAudioOutput::getInstance()->setVolume(curVolume);
}
qreal ASongAudio::getVolume()
{
    return curVolume;
}
