#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongAudioOutput.h"
#include "DataSink.h"

QAtomicPointer<ASongAudio> ASongAudio::_instance = nullptr;
//ASongAudio *ASongAudio::_instance = nullptr;
QMutex ASongAudio::_mutex;

// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        //        qDebug() << "----";
        _instance.testAndSetOrdered(nullptr, new ASongAudio);
    }
    //    if(nullptr == _instance)
    //    {
    //        _instance = new ASongAudio;
    //    }
    return _instance;
}

// 初始化音频设备参数
void ASongAudio::initAndStartDevice(QObject *par)
{
    ASongAudioOutput::getInstance()->initAudioPara(pCodecCtx->channels, pCodecCtx->sample_rate,
            pCodecCtx->channel_layout, pCodecCtx->sample_fmt);
    ASongAudioOutput::getInstance()->initAndStartDevice(par);
    ASongAudioOutput::getInstance()->initSwr();
    ASongAudioOutput::getInstance()->start();
}

/*thread*/
void ASongAudio::start(Priority pro)
{
    allowRunAudio = true;
    QThread::start(pro);
    ASongAudioOutput::getInstance()->start();
}

void ASongAudio::run()
{
    //    QList<AVFrame*>frame_list;
    AVPacket *packet = nullptr;
    while(allowRunAudio)
    {
        //        if(DataSink::getInstance()->frameListSize(0) >= DataSink::maxFrameListLength)
        //        {
        //            msleep(20);
        //        }
        //        else
        //        {
        DataSink::getInstance()->allowAppendAFrame();
        packet = DataSink::getInstance()->takeNextPacket(0);
        // 设置时钟
        //            setAudioClock(packet);
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
                    //                    qDebug() << "---";
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
                //            qDebug() << "decode";
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
}

/*设置成员变量*/
void ASongAudio::setMetaData(AVFormatContext *_pFormatCtx, AVCodecContext *_pCodecCtx, int _audioIdx)
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

void ASongAudio::setAudioClock(AVFrame *frame, const double duration)
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

//获取时钟
double ASongAudio::getAudioClock()
{
    //    double neededAudioClock = audioClock;
    //    int bufferDataSize = ASongAudioOutput::getInstance()->getUsedSize();
    //    int bytesPerSec = pCodecCtx->sample_rate * pCodecCtx->channels * 2;
    //    return neededAudioClock - (double)bufferDataSize / bytesPerSec;
    return audioClock;
}

void ASongAudio::pause()
{
    // 先结束音频播放线程
    ASongAudioOutput::getInstance()->pause();
    // 再结束音频解码线程
    if(isRunning())
    {
        allowRunAudio = false;
        wait();
    }
}

void ASongAudio::stop()
{
    // 结束音频解码线程和音频播放线程
    pause();
    // 关闭设备
    ASongAudioOutput::getInstance()->stop();
    // 关闭解码器上下文
    if(nullptr != pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
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
