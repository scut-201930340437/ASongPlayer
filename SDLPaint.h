#ifndef SDLPAINT_H
#define SDLPAINT_H

//#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
//#define SFM_PAUSE_EVENT   (SDL_USEREVENT + 2)
//#define SFM_BREAK_EVENT  (SDL_USEREVENT + 3)

#include "ASongFFmpeg.h"
#include "ASongVideo.h"

#include "DataSink.h"

#include <QWidget>
#include <QTimer>
#include <QDebug>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "SDL2/SDL.h"
};
class SDLPaint: public QObject
{
    Q_OBJECT
public:
    static SDLPaint* getInstance();
    ~SDLPaint();
    int init(QWidget *screenWidget);

    void setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt);

    void resetWHPara();

    void getFrameYUV();

    void paint(AVFrame *frameYUV);
    //public slots:
    //    void getFrameYUV();
    //    static int sfp_signal_thread(void* opaque);

    void pause();

    //    int getFrameRate();
private:
    SDLPaint() = default;
    //    int got_picture = 0;
    static QAtomicPointer<SDLPaint>_instance;
    //    static QMutex _mutex;
    QTimer *sdlTimer = nullptr;

    //------------SDL----------------
    //    int screenWidth = 0, screenHeight = 0;
    SDL_Window *screen = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    //    SDL_Rect sdlRect;
    //    SDL_Thread *videoTid = nullptr;
    //    SDL_Event event;
    // thread
    //    SDL_Thread *tid;
    // ffmpeg
    SwsContext *pSwsCtx = nullptr;
    enum AVPixelFormat pix_fmt;

    // 源视频流宽高
    int srcWidth, srcHeight;
    // 源视频流宽高比
    float srcRate = 0.0;
    // 输出widget
    QWidget* srceenWidget = nullptr;
    // 输出图像的宽高
    int dstWidth, dstHeight;
    // 帧率
    int frameRate = 1;
};

#endif // SDLPAINT_H
