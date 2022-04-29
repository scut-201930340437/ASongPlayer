#ifndef ASONGFFMPEG_H
#define ASONGFFMPEG_H

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_PAUSE_EVENT   (SDL_USEREVENT + 2)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 3)

#include <QString>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "SDL2/SDL.h"
};

struct AudioMetaData
{
    int sample_fmt;
    int sample_rate;
    int channels;
    // 音视频同步以音频时间为基准
    float curPlayPos = 0.0;
};

struct VideoMetaData
{
    int width;
    int height;
    int frame_rate;
};

struct MediaMetaDate
{
    QString path;
    QString filename;
    QString format;

    int64_t duration;

    AudioMetaData audio_meta_data;
    VideoMetaData video_meta_data;
};

class ASongFFmpeg: public QThread
{
public:
    static ASongFFmpeg* getInstance();

    static int sfp_signal_thread(void* opaque);


    int load(QString path);
    AVPacket* readFrame();
    AVFrame* decode(AVPacket* packet);


    int play();
    int pause();
    int stop();

    int getMediaStatus();

    int getScreenW();
    int getScreenH();

    //    void setMediaStatus(int status);

    // packetList是共享资源，需要加锁
    QMutex audioListMutex;
    QMutex videoListMutex;

    // 播放状态: 0-没有文件  1-正在播放  2-暂停   3-停止
    static int curMediaStatus;

    //
    static int maxPacketListLength;

private:
    ASongFFmpeg();

    static QAtomicPointer<ASongFFmpeg>_instance;
    static QMutex _mutex;

    // metaData
    MediaMetaDate mediaMetaData;
    // packetList
    QList<AVPacket>audioList;
    QList<AVPacket>videoList;

    //ffmpeg
    AVFormatContext* pFormatCtx = nullptr;
    int	videoIdx = -1, audioIdx = -1;
    AVCodecContext* pACodecCtx = nullptr, *pVCodecCtx = nullptr;
    AVCodec* pACodec = nullptr, *pVCodec = nullptr;
    AVFrame* pFrame = nullptr, * pFrameYUV = nullptr;
    uint8_t* out_buffer = nullptr;
    //    AVPacket* packet = nullptr;
    int ret = 0, got_picture = 0;

    //------------SDL----------------
    //    int screenWidth = 0, screenHeight = 0;
    SDL_Window* screen = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* sdlTexture = nullptr;
    SDL_Rect sdlRect;
    SDL_Thread* videoTid = nullptr;
    SDL_Event event;

    SwrContext* pAudioSwrCtx = nullptr;
    SwsContext* pVideoConCtx = nullptr;


};


#endif // ASONGFFMPEG_H
