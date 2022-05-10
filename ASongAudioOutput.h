#ifndef ASONGAUDIOOUTPUT_H
#define ASONGAUDIOOUTPUT_H

#include <QThread>
#include <QMutex>

#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};


class ASongAudioOutput: public QThread
{
public:
    // 单例模式，全局访问点
    static ASongAudioOutput* getInstance();
    // thread
    void start(Priority = InheritPriority);
    void initAudioPara(const int _channels, const int _sample_rate, const uint64_t _channel_layout, const enum AVSampleFormat _sample_fmt);
    // 初始化音频设备参数
    void initAndStartDevice(QObject *par);
    // 初始化重采样参数
    void initSwr();
    void closeDevice();
    // 获取当前设备缓存已用空间
    int getUsedSize();
    // 获取设备音量
    qreal getVolume();
    // 播放控制
    void pause();
    void stop();
    void setVolume(const qreal curVolume);

    //
    static const int maxFrameSize = 19200;
private:
    ASongAudioOutput() = default;
    // thread 音频解码
    void run() override;
    // 重采样为PCM
    int swrToPCM(uint8_t *outBuffer, AVFrame *frame);

    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    static QAtomicPointer<ASongAudioOutput> _instance;
    //    static ASongAudio *_instance;
    static QMutex _mutex;

    // 允许解码标志
    bool allowPlay = false;
    // 音频参数
    int channels = 0;
    int sample_rate = 0;
    uint64_t channel_layout = 0;
    enum AVSampleFormat sample_fmt;
    // 重采样上下文
    SwrContext *pSwrCtx = nullptr;
    // 音频播放设备
    QMediaDevices *mediaDevice = nullptr;
    QIODevice *audioIO = nullptr;
    QAudioSink *audioOutput = nullptr;

};

#endif // ASONGAUDIOOUTPUT_H
