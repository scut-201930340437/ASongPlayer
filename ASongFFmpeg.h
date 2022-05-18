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
public:
    // 播放状态锁
    static QMutex _mediaStatusMutex;
    // hasPacket_mutex
    static QMutex _hasPacketMutex;
    static ASongFFmpeg* getInstance();
    //    static int sfp_signal_thread(void* opaque);


    // 加载文件信息，获取媒体元数据
    MediaMetaData* openMediaInfo(QString path, AVFormatContext* pFmtCtx = nullptr);
    void closeMediaInfo(MediaMetaData *mmd);
    int load(QString path);
    // 读取一个packet
    AVPacket* readFrame();
    //    AVFrame* decode(AVPacket *packet);
    //    void decode__(AVPacket *packet, QList<AVFrame*>&frame_list);
    //    double getPts(AVFrame *frame, int streamIdx);

    // 初始化参数
    void initPara();
    /* 获取成员变量*/

    bool hasPakcet();
    int getMediaStatus();
    int getDuration();
    int getCurPlaySec();
    QString getFilepath();
    bool audioHasCover();
    /*播放控制*/
    int play(QObject *par, QString path, void *winID);
    // thread
    void start(Priority = InheritPriority);
    int stop();
    int pause();
    int resume();
    // 获取线程是否暂停
    //    bool isPaused();

    int seek(int posSec);
    // 设置播放器状态
    //    void setMediaStatus(int status);
    // 隐藏光标
    void hideCursor();
    // 显示光标
    void showCursor();

private:
    // thread
    void run() override;
    // 单一实例
    // static QAtomicPointer<ASongFFmpeg>_instance;
    // static QMutex _mutex;
    // sdlPainter
    SDLPaint *painter = nullptr;
    // 播放状态: -1 - 没有文件  0-停止  1-播放   2-暂停
    std::atomic_int curMediaStatus = -1;
    // hasmorePacket
    std::atomic_bool hasMorePacket = false;
    // metaData
    //MediaMetaDate mediaMetaData;
    MediaMetaData *mediaMetaData = nullptr;
    bool hasCover = false;
    // 允许读取数据
    std::atomic_bool allowRead = false;
    // 需要暂停
    std::atomic_bool needPaused = false;
    // 暂停标志
    //    std::atomic_bool pauseFlag = false;
    // 为使线程暂停所用的锁和条件变量
    QMutex _pauseMutex;
    QWaitCondition pauseCond;

    //ffmpeg
    AVFormatContext *pFormatCtx = nullptr;
    int	videoIdx = -1, audioIdx = -1;

    //    uint8_t *out_buffer = nullptr;
};


#endif // ASONGFFMPEG_H
