#include "ASongVideo.h"



ASongVideo::ASongVideo()
{
    setAcceptDrops(true);//启用鼠标拖拽放下操做;
}
void ASongVideo::init()
{
}
void ASongVideo::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape)
    {
        this->setFullScreen(false);
        this->resize(640,480);
    }

}
void ASongVideo::dragEnterEvent(QDragEnterEvent *e)
{
      e->acceptProposedAction(); //能够在这个窗口部件上拖放对象
      qDebug()<<"enter_son";
}

void ASongVideo::dropEvent(QDropEvent *e)
{
    emit dragDropSignal(e);
    qDebug()<<"drop_son";
}



