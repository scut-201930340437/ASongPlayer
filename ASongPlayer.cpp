#include "asongplayer.h"

int ASongPlayer::MaxPosition = 0;
int ASongPlayer::MinPosition = 0;

ASongPlayer::ASongPlayer()
{
}

void ASongPlayer::init(ASongAudio *asongAudio, ASongVideo *asongVideo)
{
    QMediaPlayer::setAudioOutput(asongAudio);
    QMediaPlayer::setVideoOutput(asongVideo);
    asongAudio->init();
    asongVideo->init();
}
void ASongPlayer::setPosition(int value)
{
    QMediaPlayer::setPosition(duration * ((float)value / MaxPosition));
}

void ASongPlayer::onDurationChanged(qint64 _duration)
{
    duration = _duration;
    MaxPosition = duration / 1000;
}

