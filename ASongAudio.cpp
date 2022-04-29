#include "ASongAudio.h"

QAtomicPointer<ASongAudio> ASongAudio::_instance = nullptr;
QMutex ASongAudio::_mutex;

// 获取单一的实例
ASongAudio* ASongAudio::getInstance()
{
    // QMutexLocker 在构造对象时加锁，在析构时解锁
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        _instance.testAndSetOrdered(nullptr, new ASongAudio);
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
