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

    void setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt, const AVRational time_base);
    // 根据输出窗口重设sdl的参数
    //    void setDstWH(const int screenWidth, const int screenHeight);
    //    void createTimer();
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
    //    void paintNextFrame();
    void restartTimer();
    void stopTimer();
    //    int getFrameRate();
    // 是否停止
    //    std::atomic_bool stopFlag = false;
    //    int getCurFrameNumber();
    //
    double basePts = 0;
    double curPts = -1;
    double frameDelay = 0.0;
private:
    void calDisplayRect(SDL_Rect *rect,
                        int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                        int pic_width, int pic_height);

    QTimer *sdlTimer = nullptr;

    //------------SDL----------------
    //    int screenWidth = 0, screenHeight = 0;

    SDL_Window *screen = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    SDL_Surface *sdlSurface = nullptr;
    SDL_Rect sdlRect;
    // ffmpeg
    double tb;
    SwsContext *pSwsCtx = nullptr;
    // 上一帧，用于暂停时不停渲染上一帧
    AVFrame *preFrame = nullptr;
    uint8_t *pre_out_buffer = nullptr;
    // 是否暂停
    std::atomic_bool pauseFlag = false;
    // 样本格式
    enum AVPixelFormat pix_fmt;

    // 源视频流宽高
    int srcWidth, srcHeight;
    // 窗口宽高
    int lastScreenWidth = 0, lastScreenHeight = 0;
    // 采样纵横比
    AVRational sar;
    // 帧率
    int frameRate = -1;
    // 帧之间的延时
    int preDelay = 0;


};

#endif // SDLPAINT_H
