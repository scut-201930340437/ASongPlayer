#ifndef ASONGFFMPEG_H
#define ASONGFFMPEG_H



#include <QString>
#include <QDebug>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"


    //#include "libswresample/swresample.h"

};

struct AudioMetaData
{
    enum AVSampleFormat sample_fmt;
    int sample_rate = 0;
    int channels = 0;
    uint64_t channel_layout = 0;
    // 音视频同步以音频时间为基准
    double curPlayPos = 0.0;
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
    ~ASongFFmpeg();
    static ASongFFmpeg* getInstance();
    //    static int sfp_signal_thread(void* opaque);

    // thread
    void start(Priority = InheritPriority);

    int load(QString path);
    AVPacket* readFrame();
    //    AVFrame* decode(AVPacket *packet);
    //    void decode__(AVPacket *packet, QList<AVFrame*>&frame_list);
    //    double getPts(AVFrame *frame, int streamIdx);


    // 获取成员变量
    //    AVFormatContext* getFormatCtx();
    //    AVCodecContext* getACodecCtx();
    //    AVCodecContext* getVCodecCtx();
    //    SwrContext* getSwrCtx();
    int getMediaType();
    int getMediaStatus();
    //    int getSampleRate();
    //    int getChannels();
    //    int getSrcWidth();
    //    int getSrcHeight();
    //    int getFrameRate();
    //    enum AVPixelFormat getPixFmt();

    int play(int media_type);
    int pause();
    int stop();

    void setMediaStatus(int status);
private:
    ASongFFmpeg() = default;

    // thread
    void run() override;
    // 单一实例
    static QAtomicPointer<ASongFFmpeg>_instance;
    static QMutex _mutex;
    // 播放状态锁
    QMutex _mediaStatusMutex;
    // 播放状态: 0-没有文件  1-正在播放  2-暂停   3-停止
    int curMediaStatus = 0;
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
