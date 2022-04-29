#ifndef ASONGPLAYER_H
#define ASONGPLAYER_H

#include "ASongAudio.h"
#include "ASongVideo.h"
#include <QMediaPlayer>
class ASongPlayer: public QMediaPlayer
{
public:
    //当前播放器打开的视频总长度 s
    static int MaxPosition;
    static int MinPosition;
    static const int positionTranRate = 1000;

    ASongPlayer();
    void init(ASongAudio *asongAudio, ASongVideo *asongVideo);

    //设置播放的位置 value为时长 单位s
    void setPosition(int value);

    //播放器打开视频文件时更新duration MaxPosition
    void onDurationChanged(qint64 _duration);
private:
    //视频总长度 ms
    qint64 duration = 1;
};

#endif // ASONGPLAYER_H
