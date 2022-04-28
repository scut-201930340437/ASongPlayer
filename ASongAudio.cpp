#include "asongaudio.h"

QAtomicPointer<ASongAudio> ASongAudio::_instance = 0;
QMutex ASongAudio::_mutex;

// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(0, 0))
    {
        _instance.testAndSetOrdered(0, new ASongAudio);
    }
    return _instance;
}

void ASongAudio::init()
{
    //    preVolume = InitVolume;
    //    QAudioOutput::setVolume((float)InitVolume / volumeTranRate);
}
void ASongAudio::setVolume(int volume)
{
    //    preVolume = QAudioOutput::volume() * volumeTranRate;
    //    QAudioOutput::setVolume((float)volume / volumeTranRate);
}
void ASongAudio::unmute()
{
    //    setMuted(false);
    //    setVolume((float)preVolume / volumeTranRate);
}
void ASongAudio::mute()
{
    //    preVolume = QAudioOutput::volume() * volumeTranRate;
    //    setMuted(true);
}
