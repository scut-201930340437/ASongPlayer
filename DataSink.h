﻿#ifndef DATASINK_H
#define DATASINK_H

#include <QDebug>

#include <QMutex>
//#include <QSemaphore>
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
    ~DataSink();
    static DataSink* getInstance();

    static const qsizetype maxAPacketListLength = 100;
    static const qsizetype maxVPacketListLength = 60;
    // 帧list最大长度
    static const qsizetype maxAFrameListLength = 120;
    static const qsizetype maxVFrameListLength = 60;
    //    static const int maxFrameListLength=100;

    AVPacket* takeNextPacket(int type);
    AVFrame* takeNextFrame(int type);

    void appendPacketList(int type, AVPacket *packet);

    bool allowAddAFrame();
    bool allowAddVFrame();

    void appendFrameList(int type, AVFrame *frame);
    //    void appendAFrameList(AVFrame *frame);


    qsizetype packetListSize(int type);
    //    qsizetype frameListSize(int type);

    //    void wakeAudioWithFraCond();
    //    void wakeVideoWithFraCond();

    //    void wakeAudioWithPackCond();
    //    void wakeVideoWithPackCond();

    void clearList();

    //    bool packListIsEmpty(int type);

    //    *audioFraEmpSem = nullptr, *videoFraEmpSem = nullptr;
    //    *videoFraSem = nullptr;

private:
    //    DataSink();

    //    static QAtomicPointer<DataSink> _instance;
    //    static QMutex _mutex;


    // packetList
    QList<AVPacket*>aPacketList;
    QList<AVPacket*>vPacketList;

    // frametList
    QList<AVFrame*>aFrameList;
    QList<AVFrame*>vFrameList;

    // packetList是共享资源，需要加锁
    QMutex aPacketListMutex;
    QMutex vPacketListMutex;
    // frameList是共享资源，需要加锁
    QMutex aFrameListMutex;
    QMutex vFrameListMutex;

    // 信号量
    //    QSemaphore *audioPackSem = nullptr, *videoPackSem = nullptr;
    //    *audioFraSem = nullptr;
    // 条件变量
    //    QWaitCondition *audioPackCond = nullptr, *videoPackCond = nullptr, *audioFraCond = nullptr, *videoFraCond = nullptr;
    // 条件变量对应的锁
    //    QMutex audioPackCond_mutex, videoPackCond_mutex;
    //    QMutex audioFraCond_mutex, videoFraCond_mutex;

};

#endif // DATASINK_H
