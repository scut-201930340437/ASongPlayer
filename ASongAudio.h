#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <atomic>

//#include "MyThread.h"
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

<<<<<<< HEAD

    // 初始化音频设备参数
    void initAndStartDevice(QObject *par);
=======
    // 初始化音频设备参数
    void initParaAndSwr();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f

    // 初始化元数据，从ffmpeg的load中读取
    void setMetaData(AVFormatContext *_pFormatCtx, AVCodecContext *_pCodecCtx, int _audioIdx);
    // 设置音频时钟
    void setAudioClock(AVFrame *frame, const double duration);
    // 设置需要解码
<<<<<<< HEAD
    void setNeededAudioCode();
=======
    //    void setNeededAudioCode();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    /* 访问成员变量*/
    // 获取时钟，用于同步，音频为基准
    double getAudioClock();


    /* 播放控制*/
    void start(Priority = InheritPriority);
<<<<<<< HEAD
    void stop();
    void pause();
    void resume();
    // 获取线程是否暂停
    //    bool isPaused();

    void flushBeforeSeek();
    void setVolume(int volume);
    qreal getVolume();
=======
    void pauseThread();
    void resumeThread();
    //
    void stop();
    //    void pause();
    void resume();
    // 查看线程是否暂停
    //    bool isPaused();
    void setVolume(int volume);
    qreal getVolume();

    std::atomic_bool stopFlag = false;
    // 暂停标志
    std::atomic_bool pauseFlag = false;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //

private:
    //    ASongAudio() = default;

    // thread 音频解码
    void run() override;
<<<<<<< HEAD
=======

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    //    static QAtomicPointer<ASongAudio> _instance;
    //    static QMutex _mutex;
    // 当前播放时长
    //    double curPlayPos = 0.0;

    // 允许解码标志
<<<<<<< HEAD
    std::atomic_bool allowRunAudio = false;
    // 还需不需要解码
    std::atomic_bool neededAudioCode = true;
    // 需要暂停
    std::atomic_bool needPaused = false;
    // 暂停标志
    //    std::atomic_bool pauseFlag = false;
=======
    std::atomic_bool stopReq = false;
    // 还需不需要解码
    std::atomic_bool neededAudioCode = true;
    // 需要暂停
    std::atomic_bool pauseReq = false;

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 为使线程暂停所用的锁和条件变量
    QMutex _pauseMutex;
    QWaitCondition pauseCond;
    // 等待线程阻塞的条件变量
    //    QWaitCondition waitPauseCond;

    // 时钟，音频为准
    double audioClock = 0.0;
    // 时基
    AVRational tb;

    // stream_index
    int audioIdx = -1;
    //
    AVFormatContext *pFormatCtx = nullptr;
    // 音频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 音量
<<<<<<< HEAD
    qreal curVolume = 0.3;
=======
    qreal curVolume = 1.0;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 音量转换系数
    const qreal volTranRate = 100.0;
};

#endif // ASONGAUDIO_H
