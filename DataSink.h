#ifndef DATASINK_H
#define DATASINK_H

#include <QDebug>

#include <QMutex>
#include <QSemaphore>
#include <QList>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
/*数据池*/
class DataSink
{
public:
    ~DataSink();
    static DataSink* getInstance();

    static const qsizetype maxPacketListLength = 20;
    // 帧list最大长度
    static const qsizetype maxFrameListLength = 20;
    //    static const int maxFrameListLength=100;

    AVPacket* takeNextPacket(int type);
    AVFrame* takeNextFrame(int type);

    void appendPacketList(int type, AVPacket *packet);
    void appendFrameList(int type, AVFrame *frame);
    //    void appendAFrameList(AVFrame *frame);

    qsizetype packetListSize(int type);
    qsizetype frameListSize(int type);

    void clearList();

    //    bool packListIsEmpty(int type);
    // 信号量
    QSemaphore *audioSem = nullptr, *videoSem = nullptr, *audioFraSem = nullptr;
    //    *audioEmpSem = nullptr, *videoEmpSem = nullptr;
    //    *videoFraSem = nullptr;

private:
    DataSink();

    static QAtomicPointer<DataSink> _instance;
    static QMutex _mutex;


    // packetList
    QList<AVPacket*>aPacketList;
    QList<AVPacket*>vPacketList;

    // frametList
    QList<AVFrame*>aFrameList;
    QList<AVFrame*>vFrameList;

    // packetList是共享资源，需要加锁
    //    QMutex aPacketListMutex;
    //    QMutex vPacketListMutex;


    // frameList是共享资源，需要加锁
    //    QMutex aFrameListMutex;
    //    QMutex vFrameListMutex;
};

#endif // DATASINK_H
