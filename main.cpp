#include "MainWindow.h"



#include <QApplication>
#include <iostream>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //这里简单的输出一个版本号
    //    cout << "Hello FFmpeg!" << endl;
    //    unsigned version = avcodec_version();
    //    cout << "version is:" << version;
    MainWindow w;
    w.show();
    return a.exec();
}
