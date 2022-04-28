#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QAtomicPointer>
#include <QList>

#include <QAudioOutput>

extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavformat/avformat.h"
#include "libavformat/version.h"

#include "libavdevice/avdevice.h"


#include "libavutil/time.h"
#include <libavutil/mathematics.h>

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

class ASongAudio: public QThread
{
public:
    static const int MaxVolume = 100;
    static const int MinVolume = 0;
    static const int InitVolume = 20;

    // 单例模式，全局访问点
    static ASongAudio* getInstance();
    void init();
    void setVolume(int volume);
    void unmute();
    void mute();
private:

    ASongAudio();

    // 使用QAtomicPointer，对指针保证与平台无关的原子操作
    static QAtomicPointer<ASongAudio>_instance;
    static QMutex _mutex;


    int preVolume;
    int volumeTranRate = 100;
};

#endif // ASONGAUDIO_H
