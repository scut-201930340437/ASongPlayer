#ifndef DATASINK_H
#define DATASINK_H

#include <QMutex>
#include <QSemaphore>
#include <QList>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
class DataSink
{
public:
    ~DataSink();
    static DataSink* getInstance();

    static const qsizetype maxPacketListLength = 5;
    //    static const int maxFrameListLength=100;

    AVPacket* takeNextPacket(int type);
    AVFrame* takeNextFrame();

    void appendPacketList(int type, AVPacket *packet);
    void appendFrameList(AVFrame *frame);

    qsizetype packetListSize(int type);
    qsizetype frameListSize();

    //    bool packListIsEmpty(int type);
    // 信号量
    QSemaphore *audioSem = nullptr, *videoSem = nullptr;
    //    *videoFraSem = nullptr;

private:
    DataSink();


    static QAtomicPointer<DataSink> _instance;
    static QMutex _mutex;


    // packetList
    QList<AVPacket*>aPacketList;
    QList<AVPacket*>vPacketList;

    // frametList
    //    QList<AVFrame>aFrameList;
    QList<AVFrame*>vFrameList;

    // packetList是共享资源，需要加锁
    //    QMutex aPacketListMutex;
    //    QMutex vPacketListMutex;


    // frameList是共享资源，需要加锁
    //    QMutex aFrameListMutex;
    //    QMutex vFrameListMutex;
};

#endif // DATASINK_H
