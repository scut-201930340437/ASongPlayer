#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongAudioOutput.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(ASongAudio, asongAudio)

//QAtomicPointer<ASongAudio> ASongAudio::_instance = nullptr;
//ASongAudio *ASongAudio::_instance = nullptr;
//QMutex ASongAudio::_mutex;

// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    //    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    //        //        qDebug() << "----";
    //        _instance.testAndSetOrdered(nullptr, new ASongAudio);
    //    }
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
void ASongAudio::setMetaData(AVFormatContext * _pFormatCtx, AVCodecContext * _pCodecCtx, int _audioIdx)
{
    pFormatCtx = _pFormatCtx;
    pCodecCtx = _pCodecCtx;
    audioIdx = _audioIdx;
    tb = pFormatCtx->streams[audioIdx]->time_base;
}
// 设置时钟---------------------
//void ASongAudio::setAudioClock(AVPacket *packet)
//{
//    if(packet->pts != AV_NOPTS_VALUE)
//    {
//        audioClock = packet->pts * av_q2d(tb);
//    }
//    // 一个packet可能包含多个frame，可能导致packet->pts<真正播放的pts
//    // 因此需要加上该packet的数据能够播放多长时间
//    audioClock += packet->duration * av_q2d(tb);
//}
void ASongAudio::setAudioClock(AVFrame * frame, const double duration)
{
    if(frame->pts != AV_NOPTS_VALUE)
    {
        audioClock = frame->pts * av_q2d(tb);
    }
    // 一个packet可能包含多个frame，可能导致packet->pts<真正播放的pts
    // 因此需要加上该packet的数据能够播放多长时间
    //    audioClock += packet->duration * av_q2d(tb);
    audioClock += duration;
}
//void ASongAudio::setNeededAudioCode()
//{
//    neededAudioCode = true;
//}
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
    //    needPaused = false;
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
        //        needPaused = false;
        //        pauseFlag = false;
        // 解码线程可能因为未播放frame队列过长而阻塞，先唤醒
        //        DataSink::getInstance()->wakeAudioWithFraCond();
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

//void ASongAudio::pause()
//{
//    // 先阻塞音频播放线程
//    ASongAudioOutput::getInstance()->pause();
//    // 再阻塞音频解码线程
//    QMutexLocker locker(&_pauseMutex);
//    if(!pauseFlag && QThread::isRunning())
//    {
//        needPaused = true;
//        // 解码线程可能因为未播放frame队列过长而阻塞，先唤醒
//        DataSink::getInstance()->wakeAudioWithFraCond();
//        pauseCond.wait(&_pauseMutex);
//        locker.relock();
//    }
//    //    qDebug() << "2";
//}

void ASongAudio::resume()
{
    // 恢复音频解码线程
    //    QMutexLocker locker(&_pauseMutex);
    //    if(pauseFlag && QThread::isRunning())
    //    {
    //        needPaused = false;
    //        pauseCond.wakeAll();
    //        pauseCond.wait(&_pauseMutex);
    //    }
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
    //    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    AVPacket *packet = nullptr;
    //    qDebug() << "audio thread start";
    //    qDebug() << "";
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
                if(ASongFFmpeg::getInstance()->seekAudio)
                {
                    if(packet->pts * av_q2d(tb) < ASongFFmpeg::getInstance()->seekTime)
                    {
                        av_packet_free(&packet);
                        continue;
                    }
                    else
                    {
                        ASongFFmpeg::getInstance()->seekAudio = false;
                    }
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
                //        }
            }
            else
            {
                //                qDebug() << "audiothread:noget";
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
        //        qDebug() << "1";
        //        qDebug() << "end";
    }
    //    qDebug() << "audio thread end";
    //    qDebug() << "";
    //    allowRunAudio = false;
    //    needPaused = false;
}

//bool ASongAudio::isPaused()
//{
//    if(pauseFlag)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}


void ASongAudio::setVolume(int volume)
{
    curVolume = volume / volTranRate;
    ASongAudioOutput::getInstance()->setVolume(curVolume);
}
qreal ASongAudio::getVolume()
{
    return curVolume;
}
