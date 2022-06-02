#ifndef DATASINK_H
#define DATASINK_H

#include <QDebug>

#include <QMutex>
#include <QWaitCondition>
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

    DataSink();
    static DataSink* getInstance();

    static const qsizetype maxAPacketListLength = 100;
    static const qsizetype maxVPacketListLength = 60;
    // 帧list最大长度
    static const qsizetype maxAFrameListLength = 120;
    static const qsizetype maxVFrameListLength = 60;
    // 倒放二维list最大长度
    static const qsizetype maxAInvertFrameListLength = 12;
    static const qsizetype maxVInvertFrameListLength = 6;
    // 倒放一维list的最大长度
    //    static const qsizetype maxInvertPerFrameListLength = 10;

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

    void frameListIsEmpty(int type);
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
    // 条件变量
    QWaitCondition *audioFraCond = nullptr, *videoFraCond = nullptr;
};

#endif // DATASINK_H
