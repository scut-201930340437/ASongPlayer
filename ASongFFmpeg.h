#ifndef ASONGFFMPEG_H
#define ASONGFFMPEG_H


#include <QWidget>
#include <QDebug>
#include <QString>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "SDLPaint.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

struct AudioMetaData
{
    enum AVSampleFormat sample_fmt;
    int sample_rate = 0;
    int channels = 0;
    uint64_t channel_layout = 0;
    // 是否有封面
    bool hasCover = false;

};

struct VideoMetaData
{
    int width = 0;
    int height = 0;
    int frame_rate = 0;

    enum AVPixelFormat pix_fmt;

};

struct MediaMetaDate
{
    int mediaType = 0;
    bool hasPacket = false;
    //    bool findAudio = false;
    QString path = "";
    QString filename = "";
    QString format = "";

    int64_t durationMSec = 0;
    int durationSec = 0;

    AudioMetaData audio_meta_data;
    VideoMetaData video_meta_data;
};

class ASongFFmpeg: public QThread
{
public:
    // 播放状态锁
    static QMutex _mediaStatusMutex;
    // hasPacket_mutex
    static QMutex _hasPacketMutex;
    ~ASongFFmpeg();
    static ASongFFmpeg* getInstance();
    //    static int sfp_signal_thread(void* opaque);

    // thread
    void start(Priority = InheritPriority);
    // 加载文件信息，获取媒体元数据
    int load(QString path);
    // 读取一个packet
    AVPacket* readFrame();
    //    AVFrame* decode(AVPacket *packet);
    //    void decode__(AVPacket *packet, QList<AVFrame*>&frame_list);
    //    double getPts(AVFrame *frame, int streamIdx);

    // 初始化参数
    void initPara();
    /* 获取成员变量*/
    //    AVFormatContext* getFormatCtx();
    //    AVCodecContext* getACodecCtx();
    //    AVCodecContext* getVCodecCtx();
    //    SwrContext* getSwrCtx();
    int getMediaType();
    bool hasPakcet();
    int getMediaStatus();
    int getDuration();
    int getCurPlaySec();
    QString getFilepath();
    bool audioHasCover();
    //    int getSampleRate();
    //    int getChannels();
    //    int getSrcWidth();
    //    int getSrcHeight();
    //    int getFrameRate();
    //    enum AVPixelFormat getPixFmt();
    /*播放控制*/
    int play(QObject *par, QString path, void *winID);
    int pause();
    //    int _continue(bool isReplay);
    int _continue();
    int stop();
    int seek(int posSec);
    // 设置播放器状态
    //    void setMediaStatus(int status);

private:
    ASongFFmpeg() = default;

    // thread
    void run() override;
    // 单一实例
    static QAtomicPointer<ASongFFmpeg>_instance;
    static QMutex _mutex;
    // sdlPainter
    SDLPaint *painter = nullptr;
    // 播放状态: -1 - 没有文件  0-停止  1-播放   2-暂停
    int curMediaStatus = -1;
    // metaData
    MediaMetaDate mediaMetaData;

    // 允许读取数据
    bool allowRead = false;

    //ffmpeg
    AVFormatContext *pFormatCtx = nullptr;
    int	videoIdx = -1, audioIdx = -1;

    //    uint8_t *out_buffer = nullptr;
};


#endif // ASONGFFMPEG_H
