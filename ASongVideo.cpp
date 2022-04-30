#include "ASongVideo.h"



ASongVideo::ASongVideo()
{
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


