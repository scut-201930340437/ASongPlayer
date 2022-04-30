#ifndef ASONGVIDEO_H
#define ASONGVIDEO_H

#include <QVideoWidget>
#include <QKeyEvent>

class ASongVideo: public QVideoWidget
{
    Q_OBJECT
public:


    ASongVideo();
    void init();

private slots:
    //1.ESC键退出全屏（resize固定640,480 后续记得修改为自适应）
    void keyPressEvent(QKeyEvent *event);

private:
    void dragEnterEvent(QDragEnterEvent *e);
    //激发信号dragDropSignal,让MainWindow看到
    void dropEvent(QDropEvent *e);

signals:
    //信号，携带QDdropEvent
    void dragDropSignal(QDropEvent *e);

};

#endif // ASONGVIDEO_H
