#ifndef ASONGVIDEO_H
#define ASONGVIDEO_H

#include <cmath>

#include <atomic>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <QList>
#include <QDebug>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};

class ASongVideo: public QThread
{
public:
    static ASongVideo* getInstance();

    void setMetaData(AVCodecContext *_pCodecCtx, const AVRational timeBase);
    // 获取pts
    double getPts(AVFrame *frame);
    // 同步
    double synVideo(const double pts);

    /*播放控制*/
    void stop();
    // thread
    void start(Priority = InheritPriority);
    void pauseThread();
    void resumeThread();
    void setSleepTime(int _sleepTime);

    // 暂停标志
    std::atomic_bool pauseFlag = false;
private:

    void run() override;
    void appendFrame(AVFrame *frame);
    void resetPara();
    // 校准pts
    void caliBratePts(AVFrame *frame, double &pts);

    int sleepTime = 60;
    //
    std::atomic_bool stopReq = false;
    std::atomic_bool pauseReq = false;

    // 为使线程暂停所用的锁和条件变量
    QMutex pauseMutex;
    QWaitCondition pauseCond;
    // videoClock
    double videoClock = 0.0;
    // 上一帧pts
    double lastFramePts = 0.0;
    // 上一帧delay
    double lastFrameDelay = 0.0;
    // av_q2d后的时基
    double tb;

    // 视频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 同步阈值下限
    const double AV_SYNC_THRESHOLD_MIN = 0.010;
    // 同步阈值上限
    double AV_SYNC_THRESHOLD_MAX = 0.0;
    // 非同步阈值
    const double AV_NOSYNC = 2.0;
    // 单帧最大显示时间
    const double AV_SYNC_FRAMEDUP_THRESHOLD = 0.07;

    QList<AVFrame*> *invertFrameList = nullptr;
};

#endif // ASONGVIDEO_H
