#ifndef SDLPAINT_H
#define SDLPAINT_H

//#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
//#define SFM_PAUSE_EVENT   (SDL_USEREVENT + 2)
//#define SFM_BREAK_EVENT  (SDL_USEREVENT + 3)

#include <QWidget>
#include <QTimer>
#include <QDebug>

#include <atomic>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL2/SDL.h"
};
class SDLPaint: public QObject
{
    Q_OBJECT
public:
    static SDLPaint* getInstance();
    ~SDLPaint();
    // 初始化sdl
    int init(void *winID);

    void setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt);
    // 根据输出窗口重设sdl的参数
    //    void setDstWH(const int screenWidth, const int screenHeight);
    // 将一帧图像转储为YUVFrame
    void getFrameYUV();
    // 绘制
    void paint(AVFrame *frameYUV);
    //public slots:
    //    void getFrameYUV();
    //    static int sfp_signal_thread(void* opaque);

    void pause();
    void stop();
    void resume();

    //    int getFrameRate();
private:
    //    SDLPaint() = default;
    //    int got_picture = 0;
    //    static QAtomicPointer<SDLPaint>_instance;
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
    // 上一帧，用于暂停时不停渲染上一帧
    AVFrame *preFrame = nullptr;
    uint8_t *pre_out_buffer = nullptr;
    // 是否暂停
    std::atomic_bool paused = false;
    // 样本格式
    enum AVPixelFormat pix_fmt;

    // 源视频流宽高
    int srcWidth, srcHeight;
    // 帧率
    int frameRate = -1;
    // 帧之间的延时
    int preDelay = 0;
};

#endif // SDLPAINT_H
