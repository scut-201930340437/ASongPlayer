#ifndef ASONGVIDEO_H
#define ASONGVIDEO_H

#include <QVideoWidget>
#include <QKeyEvent>

class ASongVideo: public QVideoWidget
{
public:


    ASongVideo();
    void init();

private slots:
    //1.ESC键退出全屏（resize固定640,480 后续记得修改为自适应）
    void keyPressEvent(QKeyEvent *event);

private:

};

#endif // ASONGVIDEO_H
