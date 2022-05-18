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

    void setMetaData(AVCodecContext *_pCodecCtx, const int _videoIdx,  const AVRational timeBase, bool _hasCover);
    //    void resetWH(const int _out_width, const int _out_height);
    void setNeededVideoCode();
    // 获取pts
    double getPts(AVFrame *frame);
    // 同步
    double synVideo(const double pts);

    /*播放控制*/
    // thread
    void start(Priority = InheritPriority);
    void stop();
    void pause();
    void resume();
    // 获取线程是否暂停
    //    bool isPaused();

    void flushBeforeSeek();

    //    QList<AVFrame*>frame_list;
private:
    //    ASongVideo() = default;

    void run() override;

    // 校准pts
    void caliBratePts(AVFrame *frame, double &pts);

    //    static QAtomicPointer<ASongVideo>_instance;
    //    static QMutex _mutex;

    // 允许线程运行
    std::atomic_bool allowRunVideo = false;
    std::atomic_bool neededVideoCode = true;
    // 需要暂停
    std::atomic_bool needPaused = false;
    // 暂停标志
    //    std::atomic_bool pauseFlag = false;
    // 为使线程暂停所用的锁和条件变量
    QMutex _pauseMutex;
    QWaitCondition pauseCond;
    // 音频有封面
    bool hasCover = false;
    // videoClock
    double videoClock = 0.0;
    // frameTime
    double frameTime = 0.0;
    // 上一帧pts
    double lastFramePts = 0.0;
    // 上一帧delay
    double lastFrameDelay = 0.0;
    // av_q2d后的时基
    double tb;

    // stream_index
    int videoIdx = -1;

    // 视频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    // 同步阈值
    const double synLowerBound = 0.01;
    // 非同步阈值
    const double noSynUpperBound = 10.0;
};

#endif // ASONGVIDEO_H
