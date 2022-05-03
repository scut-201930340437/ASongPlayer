#ifndef ASONGVIDEO_H
#define ASONGVIDEO_H

#include <cmath>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

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
    ~ASongVideo();
    static ASongVideo* getInstance();

    // thread
    void start(Priority = InheritPriority);

    void setMetaData(AVCodecContext *_pCodecCtx, const int _videoIdx,  const AVRational timeBase);
    //    void resetWH(const int _out_width, const int _out_height);
    // 获取pts
    double getPts(AVFrame *frame);
    // 同步
    double synVideo(const double pts);

    /*播放控制*/
    // 暂停
    void pause();
    void stop();

    //    QList<AVFrame*>frame_list;
private:
    ASongVideo() = default;

    void run() override;

    // 校准pts
    double caliBratePts(AVFrame *frame, double pts);


    static QAtomicPointer<ASongVideo>_instance;
    static QMutex _mutex;



    // 允许线程运行
    bool allowRunVideo = false;
    // videoClock
    double videoClock = 0.0;
    // frameTime
    double frameTime = 0.0;
    // 上一帧pts
    double lastFramePts = 0.0;
    // 上一帧delay
    double lastFrameDelay = 0.0;
    // 时基
    AVRational tb;

    // stream_index
    int videoIdx = -1;

    // 视频解码器上下文
    AVCodecContext *pCodecCtx = nullptr;
    //
    //    int frameRate = 0;
    // 源视频流的宽高
    int srcWidth = 0, srcHeight = 0;

    // 输出宽高
    //    int out_width = 0, out_height = 0;

    // 帧list最大长度
    const int maxFrameListLen = 20;

    // 同步阈值
    const double synLowerBound = 0.01;
    // 非同步阈值
    const double noSynUpperBound = 10.0;

};

#endif // ASONGVIDEO_H
