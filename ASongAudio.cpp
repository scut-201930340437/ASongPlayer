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
            pCodecCtx->channel_layout, pCodecCtx->sample_fmt);
    ASongAudioOutput::getInstance()->initSwr();
}

/*设置成员变量*/
void ASongAudio::setMetaData(AVFormatContext * _pFormatCtx, AVCodecContext * _pCodecCtx, const int _audioIdx)
{
    pFormatCtx = _pFormatCtx;
    pCodecCtx = _pCodecCtx;
    audioIdx = _audioIdx;
    tb = av_q2d(pFormatCtx->streams[audioIdx]->time_base);
}
// 设置时钟
void ASongAudio::setAudioClock(AVFrame * frame, const double duration)
{
    if(frame->pts != AV_NOPTS_VALUE)
    {
        audioClock = frame->pts * tb;
    }
    audioClock += (duration * ASongFFmpeg::getInstance()->getSpeed());
}
//获取时钟
double ASongAudio::getAudioClock()
{
    //    double neededAudioClock = audioClock;
    //    int bufferDataSize = ASongAudioOutput::getInstance()->getUsedSize();
    //    int bytesPerSec = pCodecCtx->sample_rate * pCodecCtx->channels * 2;
    //    return neededAudioClock - (double)bufferDataSize / bytesPerSec;
    return audioClock;
}
void ASongAudio::start(Priority pri)
{
    stopReq = false;
    pauseReq = false;
    pauseFlag = false;
    stopFlag = false;
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
    //    qDebug() << 2;
}

void ASongAudio::resume()
{
    if(QThread::isFinished())
    {
        start();
    }
}

void ASongAudio::pauseThread()
{
    QMutexLocker locker(&_pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&_pauseMutex);
        locker.relock();
    }
}

void ASongAudio::resumeThread()
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

void ASongAudio::run()
{
    AVPacket *packet = nullptr;
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
        if(DataSink::getInstance()->allowAddAFrame())
        {
            packet = DataSink::getInstance()->takeNextPacket(0);
            if(nullptr != packet)
            {
                //                qDebug() << "audiothread:get";
                // flushpkt
                if(packet == ASongFFmpeg::getInstance()->flushPacket)
                {
                    avcodec_flush_buffers(pCodecCtx);
                    continue;
                }
                // 扔掉小于seek的目标pts的帧
                //                if(ASongFFmpeg::getInstance()->seekAudio)
                //                {
                //                    if(packet->pts * tb < ASongFFmpeg::getInstance()->seekTime)
                //                    {
                //                        av_packet_free(&packet);
                //                        continue;
                //                    }
                //                    else
                //                    {
                //                        ASongFFmpeg::getInstance()->seekAudio = false;
                //                    }
                //                }
                //
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
                            DataSink::getInstance()->appendFrameList(0, frame);
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
                        qDebug() << "eagain";
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
                            //            qDebug() << "decode";
                            DataSink::getInstance()->appendFrameList(0, frame);
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
                                    DataSink::getInstance()->appendFrameList(0, frame);
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
            msleep(30);
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
