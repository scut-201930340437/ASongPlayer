#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QList>


#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>
#include <QAtomicPointer>

extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavformat/avformat.h"
#include "libavformat/version.h"

#include "libavdevice/avdevice.h"


#include "libavutil/time.h"
#include "libavutil/mathematics.h"

    //#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
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
    ~ASongAudio();
    // 前端交互
    //    static const int MaxVolume = 100;
    //    static const int MinVolume = 0;
    //    static const int InitVolume = 20;

    // 单例模式，全局访问点
    static ASongAudio* getInstance();
    // thread
    void start(Priority = InheritPriority);
    //
    void initAndStartDevice(QObject *par);
    void initSwr();
    //
    void setMetaData(AVFormatContext *_pFormatCtx, AVCodecContext *_pCodecCtx, int _audioIdx);
    // 访问成员变量
    // 获取时钟
    double getAudioClock();
    // 获取pts
    void setAudioClock(AVPacket *packet);
    // 获取audioOutput的剩余空间(/Byte)
    //    int getAvaiMem();
    // 播放控制

    void stopPlay();
    void pause();
    void setVolume(int volume);
    void unmute();
    void mute();

    //
    static const int maxFrameSize = 19200;

private:

    ASongAudio() = default;

    // thread 解码
    void run() override;

    // 重采样
    int swrToPCM(uint8_t *outBuffer, AVFrame*frame, AVFormatContext *pFormatCtx);
    // 写入音频输出设备
    void writeToDevice(QList<AVFrame*>&frame_list);


    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    static QAtomicPointer<ASongAudio> _instance;
    static QMutex _mutex;

    // 时钟，音频为准
    double audioClock = 0.0;
    // 时基
    AVRational tb;
    // 允许解码
    bool allowRunAudio = false;

    // stream_index
    int audioIdx = -1;
    //
    AVFormatContext *pFormatCtx = nullptr;
    // 音频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 重采样上下文
    SwrContext *pSwrCtx = nullptr;
    // 音频播放设备
    QMediaDevices *mediaDevice = nullptr;
    QIODevice *audioIO = nullptr;
    QAudioSink *audioOutput = nullptr;

    //    int preVolume;
    //    int volumeTranRate = 100;
};

#endif // ASONGAUDIO_H
