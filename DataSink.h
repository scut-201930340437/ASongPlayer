#ifndef DATASINK_H
#define DATASINK_H

#include <QDebug>

#include <QMutex>
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
    //    DataSink();
    static DataSink* getInstance();

    static qsizetype maxAPacketListLength;
    static qsizetype maxVPacketListLength;
    // 帧list最大长度
    static qsizetype maxAFrameListLength;
    static qsizetype maxVFrameListLength;
    // 倒放list最大长度
    static qsizetype maxAInvertFrameListLength;
    static qsizetype maxVInvertFrameListLength;

    AVPacket* takeNextPacket(int type);
    AVFrame* takeNextFrame(int type);
    AVFrame* takeInvertFrame(int type);

    void appendPacket(int type, AVPacket *packet);

    bool allowAddFrame(int type);
    bool allowAddInvertFrameList(int type);

    void appendFrame(int type, AVFrame *frame);
    void appendInvertFrameList(int type, QList<AVFrame*> *frameList);

    qsizetype packetListSize(int type);

    void clearList();
    void clearInvertList();
private:
    // packetList
    QList<AVPacket*>aPacketList;
    QList<AVPacket*>vPacketList;
    // frametList
    QList<AVFrame*>aFrameList;
    QList<AVFrame*>vFrameList;
    QList<QList<AVFrame*>*>aInvertFrameList;
    QList<QList<AVFrame*>*>vInvertFrameList;
    // packetList是共享资源，需要加锁
    QMutex aPacketListMutex;
    QMutex vPacketListMutex;
    // frameList是共享资源，需要加锁
    QMutex aFrameListMutex;
    QMutex vFrameListMutex;
    QMutex aInvertFrameListMutex;
    QMutex vInvertFrameListMutex;

    // 当前倒放帧总数
    qsizetype aInvertFrameSum = 0;
    qsizetype vInvertFrameSum = 0;
};

#endif // DATASINK_H
