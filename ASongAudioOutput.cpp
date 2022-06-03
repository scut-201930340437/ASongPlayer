#include "ASongFFmpeg.h"
#include "ASongAudioOutput.h"
#include "ASongAudio.h"
#include "DataSink.h"

Q_GLOBAL_STATIC(ASongAudioOutput, asongAudioOutput)
// 获取单一的实例
ASongAudioOutput* ASongAudioOutput::getInstance()
{
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
    soundtouch_putSamples_i16(soundTouch, (short*)outBuffer, frame->nb_samples);
    int numSamples = 2 * frame->nb_samples / speed;
    // 读取处理后的音频采样点数
    int newNumSamples = soundtouch_receiveSamples_i16(soundTouch, (short*)outBuffer, numSamples);
    frame->nb_samples = newNumSamples;
    // 重新计算数据大小
    int resampleDataSize = newNumSamples * channels * av_get_bytes_per_sample(out_sample_fmt);
    return resampleDataSize;
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
    if(nullptr != soundTouch)
    {
        soundtouch_setTempo(soundTouch, _speed);
    }
    speed = _speed;
    // 暂停态下设置倍速不播放，播放态下设置倍速才重新播放
    if(ASongFFmpeg::getInstance()->curMediaStatus == 1)
    {
        resume();
    }
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
    soundTouch = soundtouch_createInstance();
    soundtouch_setSampleRate(soundTouch, sample_rate);
    soundtouch_setChannels(soundTouch, channels);
    //设置倍速
    setSpeed(speed);
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
    // 关闭重采样上下文
    if(nullptr != pSwrCtx)
    {
        swr_free(&pSwrCtx);
        pSwrCtx = nullptr;
    }
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
}

void ASongAudioOutput::resume()
{
    QMutexLocker locker(&_pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        if(!ASongFFmpeg::getInstance()->stepSeek)
        {
            pauseReq = false;
        }
        pauseCond.wakeAll();
        pauseCond.wait(&_pauseMutex);
    }
}

void ASongAudioOutput::run()
{
    QAudioDevice audioDevice = mediaDevice->defaultAudioOutput();
    QAudioFormat format = audioDevice.preferredFormat();
    format.setSampleRate(sample_rate);
    format.setChannelCount(channels);
    audioOutput = new QAudioSink(audioDevice, format);
    audioOutput->setBufferSize(maxFrameSize * 2);
    audioOutput->setVolume(ASongAudio::getInstance()->getVolume());
    audioIO = audioOutput->start();
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
    closeAudioOuput();
}

void ASongAudioOutput::process()
{
    AVFrame *frame = nullptr;
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
        frame = DataSink::getInstance()->takeInvertFrame(0);
    }
    else
    {
        frame = DataSink::getInstance()->takeNextFrame(0);
    }
    if(nullptr != frame)
    {
        // 如果是planar（每个声道数据单独存放），一定要重采样，因为PCM是packed（每个声道数据交错存放）
        if(av_sample_fmt_is_planar(in_sample_fmt) == 1)
        {
            uint8_t *outBuffer = (uint8_t*)av_malloc(maxFrameSize * 2);
            int out_size = swrToPCM(outBuffer, frame);
            // 计算该帧时长
            double duration = 1.0 * out_size / (sample_rate * 2 * channels);
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
            // 交给前端绘制波形图
            char *dataBuffer = (char *)malloc(out_size);
            memcpy((void*)dataBuffer, (const void*)outBuffer, out_size);
            emit playAudio((const char*)dataBuffer, out_size / (2 * channels));
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
            int out_size = av_samples_get_buffer_size(nullptr,
                           channels,
                           frame->nb_samples,
                           out_sample_fmt,
                           0);
            if(out_size < 0)
            {
                qDebug() << "audio frame error";
                av_frame_free(&frame);
                return;
            }
            // 计算该帧时长
            double duration = 1.0 * out_size / (sample_rate * 4);
            // 做倍速处理
            if(speed > 1.00001 || speed < 0.99999)
            {
                out_size = changeSpeed((uint8_t*)frame->data, frame);
            }
            if(out_size < 0)
            {
                qDebug() << "changed speed failed";
                av_frame_free(&frame);
                return;
            }
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
            // 交给前端绘制波形图
            char *dataBuffer = (char *)malloc(out_size);
            memcpy((void*)dataBuffer, (const void*)frame->data, out_size);
            emit playAudio((const char*)dataBuffer, out_size / (2 * channels));
            // 写入设备
            audioIO->write((const char*)frame->data, out_size);
            av_frame_free(&frame);
        }
        // 倒放时如果音频时钟几乎为零说明播放结束
        if(ASongFFmpeg::getInstance()->invertFlag && ASongAudio::getInstance()->getAudioClock() <= 0.005)
        {
            stopReq = true;
            emit playFinish();
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

