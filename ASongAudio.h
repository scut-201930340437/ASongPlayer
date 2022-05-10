#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QList>


#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>

extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavformat/avformat.h"
#include "libavformat/version.h"

#include "libavdevice/avdevice.h"


#include "libavutil/time.h"
#include "libavutil/mathematics.h"

    //#include "libswscale/swscale.h"
    //#include "libswresample/swresample.h"
};

//class ASongAudioDevice: public QIODevice
//{
//public:

//    qint64 readData(char *data, qint64 maxlen) override;
//    qint64 writeData(const char *data, qint64 len) override;
//    void start();
//    void stop();
//private:

//};

class ASongAudio: public QThread
{
public:
    // 前端交互
    //    static const int MaxVolume = 100;
    //    static const int MinVolume = 0;
    //    static const int InitVolume = 20;

    // 单例模式，全局访问点
    static ASongAudio* getInstance();
    // thread
    void start(Priority = InheritPriority);
    // 初始化音频设备参数
    void initAndStartDevice(QObject *par);
    // 初始化重采样参数
    //    void initSwr();
    // 初始化元数据，从ffmpeg的load中读取
    void setMetaData(AVFormatContext *_pFormatCtx, AVCodecContext *_pCodecCtx, int _audioIdx);
    /* 访问成员变量*/
    // 获取时钟，用于同步，音频为基准
    double getAudioClock();
    // 设置音频时钟
    //    void setAudioClock(AVPacket *packet);
    void setAudioClock(AVFrame *frame, const double duration);
    // 获取audioOutput的剩余空间(/Byte)
    //    int getAvaiMem();

    /* 播放控制*/
    void closeDevice();
    void pause();
    void stop();
    void setVolume(int volume);
    void unmute();
    void mute();

    //

private:
    ASongAudio() = default;

    // thread 音频解码
    void run() override;

    //    // 重采样
    //    int swrToPCM(uint8_t *outBuffer, AVFrame*frame, AVFormatContext *pFormatCtx);
    //    // 写入音频输出设备
    //    void writeToDevice(QList<AVFrame*>&frame_list);


    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    static QAtomicPointer<ASongAudio> _instance;
    //    static ASongAudio *_instance;
    static QMutex _mutex;

    //    QAtomicPointer<DataSink> dataInstance = nullptr;
    // 当前播放时长
    //    double curPlayPos = 0.0;
    // 时钟，音频为准
    double audioClock = 0.0;
    // 时基
    AVRational tb;
    // 允许解码标志
    bool allowRunAudio = false;

    // stream_index
    int audioIdx = -1;
    //
    AVFormatContext *pFormatCtx = nullptr;
    // 音频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 重采样上下文
    //    SwrContext *pSwrCtx = nullptr;
    // 音频播放设备
    //    QMediaDevices *mediaDevice = nullptr;
    //    QIODevice *audioIO = nullptr;
    //    QAudioSink *audioOutput = nullptr;

    // 音量和转换系数
    qreal preVolume = 0.5;
};

#endif // ASONGAUDIO_H
