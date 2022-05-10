#include "ASongAudioOutput.h"
#include "ASongAudio.h"
#include "DataSink.h"

QAtomicPointer<ASongAudioOutput> ASongAudioOutput::_instance = nullptr;
//ASongAudio *ASongAudio::_instance = nullptr;
QMutex ASongAudioOutput::_mutex;

// 获取单一的实例
ASongAudioOutput* ASongAudioOutput::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        //        qDebug() << "----";
        _instance.testAndSetOrdered(nullptr, new ASongAudioOutput);
    }
    return _instance;
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
    closeDevice();
    mediaDevice = new QMediaDevices(par);
    QAudioDevice audioDevice = mediaDevice->defaultAudioOutput();
    QAudioFormat format = audioDevice.preferredFormat();
    format.setSampleRate(sample_rate);
    format.setChannelCount(channels);
    audioOutput = new QAudioSink(audioDevice, format);
    audioOutput->setBufferSize(maxFrameSize * 2);
    audioOutput->setVolume(0.5);
    audioIO = audioOutput->start();
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

void ASongAudioOutput::start(Priority pro)
{
    allowPlay = true;
    QThread::start(pro);
}

void ASongAudioOutput::run()
{
    while(allowPlay)
    {
        // 如果是planar（每个声道数据单独存放），一定要重采样，因为PCM是packed（每个声道数据交错存放）
        AVFrame *frame = DataSink::getInstance()->takeNextFrame(0);
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
                msleep(1);
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
                msleep(1);
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

void ASongAudioOutput::setVolume(int volume)
{
    audioOutput->setVolume(volume / volTranRate);
}

void ASongAudioOutput::pause()
{
    if(isRunning())
    {
        allowPlay = false;
        wait();
    }
}

void ASongAudioOutput::stop()
{
    // 关闭重采样上下文
    if(nullptr != pSwrCtx)
    {
        swr_free(&pSwrCtx);
        pSwrCtx = nullptr;
    }
    closeDevice();
    //    return true;
}
