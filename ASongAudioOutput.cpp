#include "ASongAudioOutput.h"
#include "ASongAudio.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(ASongAudioOutput, asongAudioOutput)

//QAtomicPointer<ASongAudioOutput> ASongAudioOutput::_instance = nullptr;
//QMutex ASongAudioOutput::_mutex;

// 获取单一的实例
ASongAudioOutput* ASongAudioOutput::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    //    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    //        _instance.testAndSetOrdered(nullptr, new ASongAudioOutput);
    //    }
    return asongAudioOutput;
}

void ASongAudioOutput::initAudioPara(const int _channels, const int _sample_rate, const uint64_t _channel_layout, const enum AVSampleFormat _sample_fmt)
{
    channels = _channels;
    sample_rate = _sample_rate;
    channel_layout = _channel_layout;
    sample_fmt = _sample_fmt;
}

// 初始化音频设备参数
void ASongAudioOutput::initAndStartDevice(QObject *par)
{
    // 先关闭当前音频播放
    //    closeDevice();
    mediaDevice = new QMediaDevices(par);
}

// 初始化重采样参数
void ASongAudioOutput::initSwr()
{
    // 音频重采样
    //    pCodecCtx = pFormatCtx->streams[audioIdx]->codec;
    pSwrCtx = swr_alloc();
    // 设置重采样参数
    swr_alloc_set_opts(pSwrCtx,
                       channel_layout, AV_SAMPLE_FMT_S16, sample_rate,
                       channel_layout, sample_fmt, sample_rate,
                       0, nullptr);
    // 初始化
    swr_init(pSwrCtx);
}

// 重采样
int ASongAudioOutput::swrToPCM(uint8_t *outBuffer, AVFrame *frame)
{
    if(!frame || !outBuffer)
    {
        return 0;
    }
    int len = swr_convert(pSwrCtx, &outBuffer, sample_rate,
                          (const uint8_t**)frame->data, frame->nb_samples);
    if(len <= 0)
    {
        return 0;
    }
    return av_samples_get_buffer_size(nullptr, channels,
                                      frame->nb_samples, AV_SAMPLE_FMT_S16,
                                      0);
}

void ASongAudioOutput::closeDevice()
{
    if(nullptr != audioOutput)
    {
        audioOutput->stop();
        audioOutput = nullptr;
    }
    if(nullptr != audioIO)
    {
        audioIO->close();
        audioIO = nullptr;
    }
}

int ASongAudioOutput::getUsedSize()
{
    return audioOutput->bufferSize() - audioOutput->bytesFree();
}

qreal ASongAudioOutput::getVolume()
{
    return audioOutput->volume();
}

void ASongAudioOutput::setVolume(const qreal curVolume)
{
    if(nullptr != audioOutput)
    {
        audioOutput->setVolume(curVolume);
    }
}

void ASongAudioOutput::start(Priority pri)
{
    allowPlay = true;
    QThread::start(pri);
}

void ASongAudioOutput::stop()
{
    if(QThread::isRunning())
    {
        allowPlay = false;
        needPaused = false;
        //                pauseFlag = false;
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    // 关闭重采样上下文
    if(nullptr != pSwrCtx)
    {
        swr_free(&pSwrCtx);
        pSwrCtx = nullptr;
    }
}

void ASongAudioOutput::pause()
{
    if(QThread::isRunning())
    {
        QMutexLocker locker(&_pauseMutex);
        needPaused = true;
        pauseCond.wait(&_pauseMutex);
    }
}

void ASongAudioOutput::resume()
{
    if(QThread::isRunning())
    {
        needPaused = false;
        //        pauseFlag = false;
        pauseCond.wakeAll();
    }
}

void ASongAudioOutput::run()
{
    //    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    QAudioDevice audioDevice = mediaDevice->defaultAudioOutput();
    QAudioFormat format = audioDevice.preferredFormat();
    format.setSampleRate(sample_rate);
    format.setChannelCount(channels);
    audioOutput = new QAudioSink(audioDevice, format);
    audioOutput->setBufferSize(maxFrameSize * 2);
    audioOutput->setVolume(ASongAudio::getInstance()->getVolume());
    audioIO = audioOutput->start();
    //    qDebug() << "audio output thread start";
    //    qDebug() << "";
    while(allowPlay)
    {
        AVFrame *frame = DataSink::getInstance()->takeNextFrame(0);
        if(nullptr != frame)
        {
            // 如果是planar（每个声道数据单独存放），一定要重采样，因为PCM是packed（每个声道数据交错存放）
            if(av_sample_fmt_is_planar(sample_fmt) == 1)
            {
                uint8_t *outBuffer = (uint8_t*)av_malloc(maxFrameSize * 2);
                //            while(!frame_list.empty())
                //            {
                //            frame = frame_list.takeFirst();
                int out_size = swrToPCM(outBuffer, frame);
                //            qDebug() << out_size;
                //            qDebug() << audioOutput->bytesFree();
                // 计算该帧时长
                double duration = 1.0 * out_size / (sample_rate * 4);
                if(audioOutput->bytesFree() < out_size)
                {
                    msleep(ceil(1000.0 * duration));
                }
                while(audioOutput->bytesFree() < out_size)
                {
                    msleep(2);
                }
                // 更新时钟
                ASongAudio::getInstance()->setAudioClock(frame, duration);
                // 写入设备
                audioIO->write((char*)outBuffer, out_size);
                //            }
                // 释放
                av_frame_free(&frame);
                av_free(outBuffer);
            }
            // 否则直接写入设备缓存
            else
            {
                //            while(!frame_list.empty())
                //            {
                //            frame = frame_list.takeFirst();
                int out_size = av_samples_get_buffer_size(nullptr, channels,
                               frame->nb_samples, AV_SAMPLE_FMT_S16,
                               0);
                // 计算该帧时长
                double duration = 1.0 * out_size / (sample_rate * 4);
                if(audioOutput->bytesFree() < out_size)
                {
                    msleep(ceil(1000.0 * duration));
                }
                while(audioOutput->bytesFree() < out_size)
                {
                    msleep(2);
                }
                // 更新时钟
                ASongAudio::getInstance()->setAudioClock(frame, duration);
                // 写入设备
                audioIO->write((char*)frame->data, out_size);
                //            av_frame_unref(frame);
                av_frame_free(&frame);
                //            }
            }
        }
        else
        {
            // 解码线程结束且拿不到frame，说明此时音频播放结束
            if(ASongAudio::getInstance()->isFinished())
            {
                allowPlay = false;
                emit playFinish();
            }
        }
        if(needPaused)
        {
            QMutexLocker locker(&_pauseMutex);
            //            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 线程阻塞
            pauseCond.wait(&_pauseMutex);
        }
    }
    allowPlay = false;
    needPaused = false;
    //    qDebug() << "audio output thread end";
    //    qDebug() << "";
    closeDevice();
}

//bool ASongAudioOutput::isPaused()
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

