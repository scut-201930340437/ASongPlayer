#ifndef ASONGPLAYER_H
#define ASONGPLAYER_H

#include "asongaudio.h"
#include "asongvideo.h"
#include <QMediaPlayer>
class ASongPlayer: public QMediaPlayer
{
public:
    static int MaxPosition;
    static int MinPosition;
    static const int positionTranRate = 1000;

    ASongPlayer();
    void init(ASongAudio *asongAudio, ASongVideo *asongVideo);
    void setPosition(int value);
    void onDurationChanged(qint64 _duration);
private:
    qint64 duration = 1;
};

#endif // ASONGPLAYER_H
