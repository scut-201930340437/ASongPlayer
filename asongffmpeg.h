#ifndef ASONGFFMPEG_H
#define ASONGFFMPEG_H

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_PAUSE_EVENT   (SDL_USEREVENT + 2)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 3)

#include<QString>
#include<QWidget>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL2/SDL.h"
};

class ASongFFmpeg
{
public:
    //    static int thread_exit;

    ASongFFmpeg(QWidget*screen_widget);
    ~ASongFFmpeg();

    static int sfp_signal_thread(void* opaque);


    int load(QString filename);
    int play();
    int pause();
    int stop();

    int getMediaStatus();
    //    void setMediaStatus(int status);

    // 播放状态: 0-没有文件  1-正在播放  2-暂停   3-停止
    static int curMediaStatus;

private:
    AVFormatContext* pFormatCtx = nullptr;
    int				i = 0, videoindex = -1;
    AVCodecContext* pCodecCtx = nullptr;
    AVCodec* pCodec = nullptr;
    AVFrame* pFrame = nullptr, * pFrameYUV = nullptr;
    uint8_t* out_buffer = nullptr;
    AVPacket* packet = nullptr;
    int ret = 0, got_picture = 0;

    //------------SDL----------------
    int screen_w = 0, screen_h = 0;
    SDL_Window* screen = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* sdlTexture = nullptr;
    SDL_Rect sdlRect;
    SDL_Thread* video_tid = nullptr;
    SDL_Event event;

    SwsContext* img_convert_ctx = nullptr;



};


#endif // ASONGFFMPEG_H
