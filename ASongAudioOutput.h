#ifndef ASONGAUDIOOUTPUT_H
#define ASONGAUDIOOUTPUT_H

#include <atomic>
#include <QThread>
//#include "MyThread.h"
#include <QMutex>
#include <QWaitCondition>

#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>

#include "soundTouch/SoundTouchDLL.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};


class ASongAudioOutput: public QThread
{
    Q_OBJECT
public:
    // 单例模式，全局访问点
    static ASongAudioOutput* getInstance();

    //
    static const int maxFrameSize = 19200;

    void initAudioPara(const int _channels, const int _sample_rate, const uint64_t _channel_layout, const enum AVSampleFormat _sample_fmt);
    // 初始化音频设备参数
    void createMediaDevice(QObject *par);
    // 初始化重采样参数
    void initSwr();
    // 获取当前设备缓存已用空间
    int getUsedSize();
    // 获取设备音量
    qreal getVolume();
    // 获取倍速
    float getSpeed();
    // 播放控制

    void start(Priority = InheritPriority);
    void stop();
    void pause();
    void resume();
    // 设置倍速
    void setSpeed(float _speed);
    // 线程执行的函数主体
    void process();
    // 获取线程是否暂停
    //    bool isPaused();
    void setVolume(const qreal curVolume);


    // 暂停标志
    std::atomic_bool pauseFlag = false;
    // 停止标志
    std::atomic_bool stopFlag = false;
    QMutex stopMutex;
private:
    //    ASongAudioOutput() = default;
    // thread 音频解码
    void run() override;

    // 重采样为PCM
    int swrToPCM(uint8_t *outBuffer, AVFrame *frame);
    // 倍速处理
    int changeSpeed(uint8_t *outBuffer, AVFrame *frame);
    // 关闭设备和soundtouch
    void closeAudioOuput();

    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    //    static QAtomicPointer<ASongAudioOutput> _instance;
    //    static ASongAudio *_instance;
    //    static QMutex _mutex;

    // 允许播放标志
    std::atomic_bool stopReq = false;
    // 需要暂停
    std::atomic_bool pauseReq = false;
    // 为使线程暂停所用的锁和条件变量
    QMutex _pauseMutex;
    QWaitCondition pauseCond;
    // 音频参数
    int channels = 0;
    int sample_rate = 0;
    uint64_t channel_layout = 0;

    enum AVSampleFormat in_sample_fmt;
    const enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    // 重采样上下文
    SwrContext *pSwrCtx = nullptr;
    /*倍速*/
    // sonicStream
    //    sonicStream sonicStream;
    HANDLE soundTouch = nullptr;
    float speed = 1.0;
    //    int speedBufferSize = 0;


    // 音频播放设备
    QMediaDevices *mediaDevice = nullptr;
    QIODevice *audioIO = nullptr;
    QAudioSink *audioOutput = nullptr;

signals:
    void playFinish();
};

#endif // ASONGAUDIOOUTPUT_H
