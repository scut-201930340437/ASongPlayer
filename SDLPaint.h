#ifndef SDLPAINT_H
#define SDLPAINT_H

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
    int init(QWidget *_playWidget);
    // 设置参数
    void setMetaData(const int width, const int height, const int _frameRate, const enum AVPixelFormat _pix_fmt, const AVRational _sar);
    // 将一帧图像转为YUVFrame
    void getFrameYUV();
    // 绘制
    void paint(AVFrame *frameYUV);
    // 播放控制
    void stop();
    void pause();
    void resume();
    void stopTimer();
    void restartTimer();

    double basePts = 0.0;
    double curPts = 0.0;
private:
    void calDisplayRect(SDL_Rect *rect,
                        int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                        int pic_width, int pic_height);

    // 是否暂停
    std::atomic_bool pauseFlag = false;

    QWidget *playWidget = nullptr;

    QTimer *sdlTimer = nullptr;
    // SDL
    SDL_Window *screen = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    SDL_Surface *sdlSurface = nullptr;
    SDL_Rect sdlRect;
    /*ffmpeg*/
    SwsContext *pSwsCtx = nullptr;
    // 上一帧，用于暂停时不停渲染上一帧
    AVFrame *preFrame = nullptr;
    uint8_t *pre_out_buffer = nullptr;

    // 样本格式
    enum AVPixelFormat pix_fmt;

    // 源视频流宽高
    int srcWidth = 0, srcHeight = 0;
    // 窗口宽高
    int lastScreenWidth = 0, lastScreenHeight = 0;
    // 采样纵横比
    AVRational sar;
    // 帧率
    int frameRate = -1;
    // 帧之间的延时
    int frameDelay = 0;
signals:
    void playFinish();
};

#endif // SDLPAINT_H
