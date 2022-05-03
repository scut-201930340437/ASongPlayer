#include "ASongFFmpeg.h"
#include "ASongAudio.h"

//QAtomicPointer<ASongAudio> ASongAudio::_instance = nullptr;
ASongAudio *ASongAudio::_instance = nullptr;
QMutex ASongAudio::_mutex;

ASongAudio::~ASongAudio()
{
    avcodec_close(pCodecCtx);
    swr_free(&pSwrCtx);
    audioOutput->stop();
    audioIO->close();
}
// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        qDebug() << "----";
    //        _instance.testAndSetOrdered(nullptr, new ASongAudio);
    //    }
    if(nullptr == _instance)
    {
        _instance = new ASongAudio;
    }
    return _instance;
}

// 初始化
// 初始化音频设备参数
void ASongAudio::initAndStartDevice(QObject *par)
{
    // 先关闭当前音频播放
    closeDevice();
    mediaDevice = new QMediaDevices(par);
    QAudioDevice audioDevice = mediaDevice->defaultAudioOutput();
    QAudioFormat format = audioDevice.preferredFormat();
    format.setSampleRate(pCodecCtx->sample_rate);
    format.setChannelCount(pCodecCtx->channels);
    audioOutput = new QAudioSink(audioDevice, format);
    audioOutput->setBufferSize(maxFrameSize * 2);
    audioIO = audioOutput->start();
}

// 初始化重采样参数
void ASongAudio::initSwr()
{
    // 音频重采样
    //    pCodecCtx = pFormatCtx->streams[audioIdx]->codec;
    pSwrCtx = swr_alloc();
    // 设置重采样参数
    swr_alloc_set_opts(pSwrCtx,
                       pCodecCtx->channel_layout, AV_SAMPLE_FMT_S16, pCodecCtx->sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate,
                       0, nullptr);
    // 初始化
    swr_init(pSwrCtx);
}

/*thread*/
void ASongAudio::start(Priority pro)
{
    allowRunAudio = true;
    QThread::start(pro);
}

void ASongAudio::run()
{
    QList<AVFrame*>frame_list;
    AVPacket *packet = nullptr;
    while(allowRunAudio)
    {
        //        if(DataSink::getInstance()->packetListSize(0) > 0)
        //        {
        //            QMutexLocker locker(&(ASongFFmpeg::getInstance()->audioListMutex));
        packet = DataSink::getInstance()->takeNextPacket(0);
        //            locker.unlock();
        //            timestamp = ASongFFmpeg::getInstance()->getPts(packet);
        // 解码
        //    qDebug() << pCodecCtx->codec_id;
        // 设置时钟
        setAudioClock(packet);
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
                    frame_list.append(frame);
                }
                else
                {
                    if(ret == AVERROR_EOF)
                    {
                        // 复位解码器
                        avcodec_flush_buffers(pCodecCtx);
                    }
                    av_frame_unref(frame);
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
                        av_frame_unref(frame);
                        av_frame_free(&frame);
                        // 复位解码器
                        avcodec_flush_buffers(pCodecCtx);
                        break;
                    }
                    //            qDebug() << "decode";
                    frame_list.append(frame);
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
                            frame_list.append(frame);
                        }
                        else
                        {
                            if(ret == AVERROR_EOF)
                            {
                                // 复位解码器
                                avcodec_flush_buffers(pCodecCtx);
                            }
                            av_frame_unref(frame);
                            av_frame_free(&frame);
                            break;
                        }
                    }
                }
            }
        }
        // 释放
        av_packet_unref(packet);
        av_packet_free(&packet);
        //        qDebug() << packet->data[0];
        // 将framelist中的数据送到音频设备
        //        qDebug() << frame_list.size();
        if(nullptr != audioIO)
        {
            writeToDevice(frame_list);
        }
        // 清空list
        frame_list.clear();
        //        }
        //        else
        //        {
        //            msleep(1);
        //        }
    }
}

// 重采样
int ASongAudio::swrToPCM(uint8_t *outBuffer, AVFrame *frame, AVFormatContext *pFormatCtx)
{
    if(!pFormatCtx || !pCodecCtx || !frame || !outBuffer)
    {
        return 0;
    }
    int len = swr_convert(pSwrCtx, &outBuffer, pCodecCtx->sample_rate,
                          (const uint8_t**)frame->data, frame->nb_samples);
    if(len <= 0)
    {
        return 0;
    }
    return av_samples_get_buffer_size(nullptr, pCodecCtx->channels,
                                      frame->nb_samples, AV_SAMPLE_FMT_S16,
                                      0);
}

void ASongAudio::writeToDevice(QList<AVFrame*>&frame_list)
{
    // 如果是planar（每个声道数据单独存放），一定要重采样，因为PCM是packed（每个声道数据交错存放）
    AVFrame *frame = nullptr;
    if(av_sample_fmt_is_planar(pCodecCtx->sample_fmt) == 1)
    {
        uint8_t *outBuffer = (uint8_t*)av_malloc(maxFrameSize * 2);
        while(!frame_list.empty())
        {
            frame = frame_list.takeFirst();
            int out_size = swrToPCM(outBuffer, frame, pFormatCtx);
            //            qDebug() << out_size;
            av_frame_unref(frame);
            av_frame_free(&frame);
            //            qDebug() << audioOutput->bytesFree();
            if(audioOutput->bytesFree() < out_size)
            {
                msleep(ceil(1000.0 * out_size / (pCodecCtx->sample_rate * 4)));
            }
            //            qDebug() << audioOutput->bytesFree();
            while(audioOutput->bytesFree() < out_size)
            {
                msleep(1);
            }
            audioIO->write((char*)outBuffer, out_size);
        }
        av_free(outBuffer);
    }
    // 否则直接写入设备缓存
    else
    {
        while(!frame_list.empty())
        {
            frame = frame_list.takeFirst();
            int out_size = av_samples_get_buffer_size(nullptr, pCodecCtx->channels,
                           frame->nb_samples, AV_SAMPLE_FMT_S16,
                           0);
            if(audioOutput->bytesFree() < out_size)
            {
                msleep(ceil(1000.0 * out_size / (pCodecCtx->sample_rate * 4)));
            }
            while(audioOutput->bytesFree() < out_size)
            {
                msleep(1);
            }
            audioIO->write((char*)frame->data, out_size);
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
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
void ASongAudio::setAudioClock(AVPacket *packet)
{
    if(packet->pts != AV_NOPTS_VALUE)
    {
        audioClock = packet->pts * av_q2d(tb);
    }
    // 一个packet可能包含多个frame，可能导致packet->pts<真正播放的pts
    // 因此需要算出该packet的数据能够播放多长时间
    audioClock += packet->duration * av_q2d(tb);
}

/*获取成员变量*/
double ASongAudio::getAudioClock()
{
    double neededAudioClock = audioClock;
    int bufferDataSize = audioOutput->bufferSize() - audioOutput->bytesFree();
    int bytesPerSec = pCodecCtx->sample_rate * pCodecCtx->channels * 2;
    return neededAudioClock - (double)bufferDataSize / bytesPerSec;
}

void ASongAudio::closeDevice()
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

void ASongAudio::pause()
{
    allowRunAudio = false;
}

void ASongAudio::stop()
{
    allowRunAudio = false;
    closeDevice();
}

void ASongAudio::setVolume(int volume)
{
    //    preVolume = QAudioOutput::volume() * volumeTranRate;
    //    QAudioOutput::setVolume((float)volume / volumeTranRate);
}
void ASongAudio::unmute()
{
    //    setMuted(false);
    //    setVolume((float)preVolume / volumeTranRate);
}
void ASongAudio::mute()
{
    //    preVolume = QAudioOutput::volume() * volumeTranRate;
    //    setMuted(true);
}
