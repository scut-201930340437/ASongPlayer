#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H
#include<QThread>
#include<QImage>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include "libavutil/imgutils.h"
#include<SDL2/SDL_mutex.h>
};

class VideoPreview: public QThread{
public:
    static VideoPreview* getInstance();
public:
    void start(QString path, int videoIndex, Priority = InheritPriority);//开启线程
    void stop();//停止线程

    void resume();//继续线程，鼠标进入进度条时调用
    void pause(); //阻塞线程，鼠标离开进度条时调用
    QImage getPreviewImg(); //获取预览图
    void setPosSec(int64_t posSec); //设置预览位置，单位为秒

protected:
    void run() override;

private:
    VideoPreview() = default;

    bool thread_fail = false;
    QString path;
    int vidx;
    SDL_cond* continue_preview_thread = NULL;
    bool abort_req;
    bool preview_req;
    bool img_up_to_date;
    int64_t posSec;
    QImage preview_img;

};

#endif // VIDEOPREVIEW_H
