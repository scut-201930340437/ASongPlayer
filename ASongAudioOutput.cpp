//#include "ASongFFmpeg.h"
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
    in_sample_fmt = _sample_fmt;
}

void ASongAudioOutput::createMediaDevice(QObject *par)
{
    mediaDevice = new QMediaDevices(par);
}

// 初始化重采样参数
void ASongAudioOutput::initSwr()
{
    // 音频重采样
    pSwrCtx = swr_alloc();
    // 设置重采样参数
    swr_alloc_set_opts(pSwrCtx,
                       channel_layout, out_sample_fmt, sample_rate,
                       channel_layout, in_sample_fmt, sample_rate,
                       0, nullptr);
    // 初始化
    swr_init(pSwrCtx);
    // 初始化sonic，包括倍速等参数
    //    sonicInit();
    //    sonicSetSpeed(2.0);
    //    sonicSetVolume(ASongAudio::getInstance()->getVolume());
}

// 重采样
int ASongAudioOutput::swrToPCM(uint8_t *outBuffer, AVFrame *frame)
{
    if(!frame || !outBuffer)
    {
        return -1;
    }
    int len = swr_convert(pSwrCtx, &outBuffer, sample_rate,
                          (const uint8_t**)frame->data, frame->nb_samples);
    if(len < 0)
    {
        return -1;
    }
    return av_samples_get_buffer_size(nullptr,
                                      channels,
                                      frame->nb_samples,
                                      out_sample_fmt,
                                      0);
}
// 倍速处理
int ASongAudioOutput::changeSpeed(uint8_t *outBuffer, AVFrame *frame)
{
    //
    //    int ret = sonicWriteShortToStream(sonicStream, (short*)outBuffer, frame->nb_samples);
    //    int ret = sonicWriteUnsignedCharToStream(sonicStream, outBuffer, frame->nb_samples);
    soundtouch_putSamples_i16(soundTouch, (short*)outBuffer, frame->nb_samples);
    //    if(ret > 0)
    //    {
    // 乘2保证调用一次sonicReadShortFromStream可以读取全部数据
    int numSamples = 2 * frame->nb_samples / speed;
    //        qDebug() << numSamples;
    //        if(speed < 1)
    //        {
    //            int size = numSamples * channels * av_get_bytes_per_sample(out_sample_fmt);
    //            if(speedBufferSize < size)
    //            {
    //                speedBuffer = av_realloc(speedBuffer, size);
    //                speedBufferSize = size;
    //            }
    //            outBuffer =
    //        }
    // 读取处理后的音频采样点数
    //    int newNumSamples = sonicReadShortFromStream(sonicStream, (short*)outBuffer, numSamples);
    int newNumSamples = soundtouch_receiveSamples_i16(soundTouch, (short*)outBuffer, numSamples);
    //    qDebug() << newNumSamples;
    //        int newNumSamples = sonicReadUnsignedCharFromStream(sonicStream, outBuffer, numSamples);
    frame->nb_samples = newNumSamples;
    // 重新计算数据大小
    int resampleDataSize = newNumSamples * channels * av_get_bytes_per_sample(out_sample_fmt);
    //        qDebug() << resampleDataSize;
    return resampleDataSize;
    //    }
    //    else
    //    {
    //        qDebug() << "sonicWriteShortToStream failed";
    //        return -1;
    //    }
}

void ASongAudioOutput::closeAudioOuput()
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
    //    if (nullptr != sonicStream)
    //    {
    //        sonicDestroyStream(sonicStream);
    //        sonicStream = nullptr;
    //    }
    if(nullptr != soundTouch)
    {
        soundtouch_destroyInstance(soundTouch);
        soundTouch = nullptr;
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

void ASongAudioOutput::setSpeed(float _speed)
{
    if(_speed <= 0)
    {
        qDebug() << "speed<=0";
        return;
    }
    // 先暂停音频播放线程，防止通过sonicSetSpeed后音频播放线程不能及时更新速率
    pause();
    //设置倍速
    //    if(nullptr != sonicStream)
    //    {
    //        sonicSetSpeed(sonicStream, _speed);
    //    }
    if(nullptr != soundTouch)
    {
        soundtouch_setTempo(soundTouch, _speed);
    }
    speed = _speed;
    resume();
}

float ASongAudioOutput::getSpeed()
{
    return speed;
}

void ASongAudioOutput::start(Priority pri)
{
    stopReq = false;
    pauseReq = false;
    pauseFlag = false;
    stopFlag = false;
    //初始化sonicStream
    //    sonicStream = sonicCreateStream(sample_rate, channels);
    soundTouch = soundtouch_createInstance();
    soundtouch_setSampleRate(soundTouch, sample_rate);
    soundtouch_setChannels(soundTouch, channels);
    //设置倍速
    setSpeed(speed);
    // 设置质量
    //    sonicSetQuality(sonicStream, 0);
    QThread::start(pri);
}

void ASongAudioOutput::stop()
{
    if(QThread::isRunning())
    {
        stopReq = true;
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    //    QMutexLocker locker(&DataSink::getInstance()->aFrameListMutex);
    //    stopFlag = true;
    //    DataSink::getInstance()->wakeAudioWithFraCond();
    //    locker.unlock();
    // 关闭重采样上下文
    if(nullptr != pSwrCtx)
    {
        swr_free(&pSwrCtx);
        pSwrCtx = nullptr;
    }
    //    qDebug() << "122";
}

void ASongAudioOutput::pause()
{
    QMutexLocker locker(&_pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&_pauseMutex);
        locker.relock();
    }
    //    qDebug() << "1";
}

void ASongAudioOutput::resume()
{
    QMutexLocker locker(&_pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        pauseReq = false;
        pauseCond.wakeAll();
        pauseCond.wait(&_pauseMutex);
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
    for(;;)
    {
        if(stopReq)
        {
            break;
        }
        if(pauseReq)
        {
            QMutexLocker locker(&_pauseMutex);
            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 线程阻塞
            pauseCond.wait(&_pauseMutex);
            locker.relock();
            pauseFlag = false;
            // 唤醒主线程
            pauseCond.wakeAll();
        }
        process();
    }
    //    allowPlay = false;
    //    needPaused = false;
    //    qDebug() << "audio output thread end";
    //    qDebug() << "";
    closeAudioOuput();
    QMutexLocker locker(&stopMutex);
    stopFlag = true;
    locker.unlock();
}

void ASongAudioOutput::process()
{
    AVFrame *frame = DataSink::getInstance()->takeNextFrame(0);
    if(nullptr != frame)
    {
        // 如果是planar（每个声道数据单独存放），一定要重采样，因为PCM是packed（每个声道数据交错存放）
        if(av_sample_fmt_is_planar(in_sample_fmt) == 1)
        {
            uint8_t *outBuffer = (uint8_t*)av_malloc(maxFrameSize * 2);
            int out_size = swrToPCM(outBuffer, frame);
            if(out_size < 0)
            {
                qDebug() << "swr failed";
                // 释放
                av_frame_free(&frame);
                av_free(outBuffer);
                return;
            }
            //做倍速处理
            if(speed > 1.00001 || speed < 0.99999)
            {
                out_size = changeSpeed(outBuffer, frame);
            }
            if(out_size < 0)
            {
                qDebug() << "changed speed failed";
                // 释放
                av_frame_free(&frame);
                av_free(outBuffer);
                return;
            }
            // 计算该帧时长
            double duration = 1.0 * out_size / (sample_rate * 4);
            if(audioOutput->bytesFree() < out_size)
            {
                msleep(1000.0 * duration + 0.5);
            }
            while(audioOutput->bytesFree() < out_size)
            {
                msleep(1);
            }
            // 更新时钟
            ASongAudio::getInstance()->setAudioClock(frame, duration);
            // 写入设备
            audioIO->write((const char*)outBuffer, out_size);
            //            }
            av_free(outBuffer);
            // 释放
            av_frame_free(&frame);
        }
        // 否则不需要重采样
        else
        {
            //            qDebug() << "packet audio";
            int out_size = -1;
            // 做倍速处理
            if(speed > 1.00001 || speed < 0.99999)
            {
                out_size = changeSpeed((uint8_t*)frame->data, frame);
            }
            // 不需要倍速
            else
            {
                out_size = av_samples_get_buffer_size(nullptr,
                                                      channels,
                                                      frame->nb_samples,
                                                      out_sample_fmt,
                                                      0);
            }
            if(out_size < 0)
            {
                qDebug() << "changed speed failed";
                av_frame_free(&frame);
                return;
            }
            // 计算该帧时长
            double duration = 1.0 * out_size / (sample_rate * 4);
            if(audioOutput->bytesFree() < out_size)
            {
                msleep(1000.0 * duration + 0.5);
            }
            while(audioOutput->bytesFree() < out_size)
            {
                msleep(1);
            }
            // 更新时钟
            ASongAudio::getInstance()->setAudioClock(frame, duration);
            // 写入设备
            audioIO->write((const char*)frame->data, out_size);
            av_frame_free(&frame);
        }
    }
    else
    {
        // 解码线程结束且拿不到frame，说明此时音频播放结束
        if(ASongAudio::getInstance()->isFinished())
        {
            stopReq = true;
            emit playFinish();
        }
    }
}

//bool ASongAudioOutput::isPaused()
//{
//    QMutexLocker locker(&_pauseMutex);
//    if(pauseFlag)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

