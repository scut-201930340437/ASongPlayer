#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <atomic>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QList>

#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"
};

class ASongAudio: public QThread
{
public:
    // 单例模式，全局访问点
    static ASongAudio* getInstance();
    // 初始化音频设备参数
    void initParaAndSwr();
    // 初始化元数据，从ffmpeg的load中读取
    void setMetaData(AVCodecContext * _pCodecCtx, const AVRational timeBase);
    // 设置音频时钟
    void setAudioClock(AVFrame *frame, const double duration);
    /* 访问成员变量*/
    // 获取时钟，用于同步，音频为基准
    double getAudioClock();

    /* 播放控制*/
    void start(Priority = InheritPriority);
    void pauseThread();
    void resumeThread();
    //
    void stop();
    void setVolume(int volume);
    qreal getVolume();

    void setSleepTime(int _sleepTime);

    // 暂停标志
    std::atomic_bool pauseFlag = false;
private:
    // thread 音频解码
    void run() override;
    void appendFrame(AVFrame *frame);
    void resetPara();

    int sleepTime = 30;

    // 停止请求标志位
    std::atomic_bool stopReq = false;
    // 需要暂停
    std::atomic_bool pauseReq = false;

    // 为使线程暂停所用的锁和条件变量
    QMutex pauseMutex;
    QWaitCondition pauseCond;

    // 时钟，音频为准
    double audioClock = 0.0;
    // 时基
    double tb;
    // 音频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 音量
    qreal curVolume = 0.3;
    // 音量转换系数
    const qreal volTranRate = 100.0;
    // 每次倒放seek解码得到的一段时间间隔的帧队列
    QList<AVFrame*> *invertFrameList = nullptr;
};

#endif // ASONGAUDIO_H
