#include "mainwindow.h"

#include <QApplication>
#include<QDebug>


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "SDL2/SDL.h"
}
#define main SDL_main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //这里简单的输出一个版本号
    //    if(SDL_Init(SDL_INIT_VIDEO))
    //    {
    //        qDebug() << "can not int SDL error！！！";
    //        return -1;
    //    }
    //    else
    //    {
    //        qDebug() << "success！！！";
    //    }
    MainWindow w;
    w.show();
    return a.exec();
}
