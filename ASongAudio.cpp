#include "ASongAudio.h"

ASongAudio::ASongAudio()
{
}

void ASongAudio::init()
{
    preVolume = InitVolume;
    QAudioOutput::setVolume((float)InitVolume / volumeTranRate);
}
void ASongAudio::setVolume(int volume)
{
    preVolume = QAudioOutput::volume() * volumeTranRate;
    QAudioOutput::setVolume((float)volume / volumeTranRate);
}
void ASongAudio::unmute()
{
    setMuted(false);
    setVolume((float)preVolume / volumeTranRate);
}
void ASongAudio::mute()
{
    preVolume = QAudioOutput::volume() * volumeTranRate;
    setMuted(true);
}

