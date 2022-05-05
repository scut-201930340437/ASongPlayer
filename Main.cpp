#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>


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
    //打上样式
    QFile file(":/qss/AMOLED.qss");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString styleSheet = filetext.readAll();
    qApp->setStyleSheet(styleSheet);
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
