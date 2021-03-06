#ifndef ASONGFFMPEG_H
#define ASONGFFMPEG_H

#include <atomic>
#include <QWidget>
#include <QDebug>
#include <QString>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include "SDLPaint.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};


struct AudioMetaData
{
    int64_t bit_rate; //比特率
    int sample_rate; //采样率
    int nb_channels; //声道数
    AVCodecID codec_id; //编码格式id, 用avcodec_get_name获取对应名字

    int idx; //流下标

};

struct VideoMetaData
{
    QImage *cover; //封面图，没有封面则为nullptr
    int width = 0; //帧宽
    int height = 0; //帧高
    int64_t bit_rate; //比特率
    double frame_rate; //帧率
    AVCodecID codec_id; //编码格式id, 用avcodec_get_name获取对应名字

    int idx; //流下标
};

struct MediaMetaData
{

    QString path; //路径
    QString filename; //文件名
    QString title; //标题
    QString artist; //艺术家
    QString album; //专辑
    int64_t durationSec; //时长，单位为秒

    int nb_astreams, nb_vstreams; //音频流个数和视频流个数
    AudioMetaData* aMetaDatas; //音频流数组指针，动态分配
    VideoMetaData* vMetaDatas; //视频流数组指针，动态分配
};

class ASongFFmpeg: public QThread
{
    Q_OBJECT
public:
    static ASongFFmpeg* getInstance();

    // 加载文件信息，获取媒体元数据
    MediaMetaData* openMediaInfo(QString path, AVFormatContext* pFmtCtx = nullptr);
    void closeMediaInfo(MediaMetaData *mmd);
    int load(QString path);
    // 读取一个packet
    AVPacket* readFrame();
    /* 获取成员变量*/
    int getDuration();
    int64_t getCurPlaySec();
    QString getFilepath();
    float getSpeed();
    /*播放控制*/
    void resetPara();
    int play(QObject *par, QString path, QWidget *_playWidget);
    // thread
    void start(Priority = InheritPriority);
    int stop();
    int pause();
    int resume();
    // 进度跳转
    void seek(int64_t posSec);
    // 进度微调
    void step_to_dst_frame(int step);
    // 设置倍速
    void setSpeed(float _speed);
    // 倒放
    void invertPlay();
    // 隐藏光标
    void hideCursor();
    // 显示光标
    void showCursor();

    // 播放状态: -1-没有文件 0-停止 1-播放 2-暂停
    std::atomic_int curMediaStatus = -1;
    // 音频是否有封面
    bool hasCover = false;
    // 线程暂停
    std::atomic_bool pauseFlag = false;
    // 有逐帧seek请求
    bool stepSeek = false;
    // 进度微调时的丢帧标志位
    bool seekAudio = false;
    bool seekVideo = false;
    // 进度微调的目标pts
    double targetPts = 0.0;
    // 倒放
    std::atomic_bool invertFlag = false;
    std::atomic_bool needInvertSeek = false;
    double invertPts = 0.0;
private:
    // thread
    void run() override;
    void pauseThread();
    void resumeThread();
    // 处理跳转
    void handleSeek();
    // 处理逐帧
    void handleStepSeek();
    // 处理倒放
    void initInvert();
    void handleInvertSeek();

    /*thread*/
    int sleepTime = 25;
    // 需要停止
    std::atomic_bool stopReq = false;
    // 需要暂停
    std::atomic_bool pauseReq = false;
    // 为使线程暂停所用的锁和条件变量
    QMutex pauseMutex;
    QWaitCondition pauseCond;
    /*ffmpeg*/
    // metaData
    MediaMetaData *mediaMetaData = nullptr;
    int audioIdx = -1, videoIdx = -1;
    AVFormatContext *pFormatCtx = nullptr;
    /*seek*/
    bool seekReq = false;
    int64_t seekPos = 0;
    int64_t seekRel = 0;
    int64_t seekMin = INT64_MIN;
    int64_t seekMax = INT64_MAX;
    int seekFlag = -1;
    /*逐帧*/
    bool stepSeekReq = false;
    // 逐帧所用的条件变量
    QWaitCondition clearListCond;
    QMutex clearListMutex;
    bool cleared = false;
    // 倒放
    bool invertReq = false;
};


#endif // ASONGFFMPEG_H
