#ifndef ASONGAUDIO_H
#define ASONGAUDIO_H

#include <QAudioOutput>
class ASongAudio: public QAudioOutput
{
public:
    static const int MaxVolume = 100;
    static const int MinVolume = 0;
    static const int InitVolume = 20;
    ASongAudio();
    void init();
    void setVolume(int volume);
    void unmute();
    void mute();
private:
    int preVolume;
    int volumeTranRate = 100;
};

#endif // ASONGAUDIO_H
