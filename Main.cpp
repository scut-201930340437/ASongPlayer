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
    QFile file(":/qss/black.qss");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString styleSheet = filetext.readAll();
    qApp->setStyleSheet(styleSheet);
    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint);  // 去掉标题栏
    w.show();
    return a.exec();
}
