#ifndef DATASINK_H
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

<<<<<<< HEAD
    static const qsizetype maxPacketListLength = 30;
    // 帧list最大长度
    static const qsizetype maxFrameListLength = 20;
=======
    static const qsizetype maxAPacketListLength = 100;
    static const qsizetype maxVPacketListLength = 60;
    // 帧list最大长度
    static const qsizetype maxAFrameListLength = 120;
    static const qsizetype maxVFrameListLength = 60;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //    static const int maxFrameListLength=100;

    AVPacket* takeNextPacket(int type);
    AVFrame* takeNextFrame(int type);

    void appendPacketList(int type, AVPacket *packet);
<<<<<<< HEAD
    void appendFrameList(int type, AVFrame *frame);
    //    void appendAFrameList(AVFrame *frame);
    void allowAppendAFrame();
    void allowAppendVFrame();
=======

    bool allowAddAFrame();
    bool allowAddVFrame();

    void appendFrameList(int type, AVFrame *frame);
    //    void appendAFrameList(AVFrame *frame);

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f

    qsizetype packetListSize(int type);
    //    qsizetype frameListSize(int type);

<<<<<<< HEAD
    void wakeAudioWithFraCond();
    void wakeVideoWithFraCond();
=======
    //    void wakeAudioWithFraCond();
    //    void wakeVideoWithFraCond();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f

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

<<<<<<< HEAD
=======
    // packetList是共享资源，需要加锁
    QMutex aPacketListMutex;
    QMutex vPacketListMutex;
    // frameList是共享资源，需要加锁
    QMutex aFrameListMutex;
    QMutex vFrameListMutex;

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 信号量
    //    QSemaphore *audioPackSem = nullptr, *videoPackSem = nullptr;
    //    *audioFraSem = nullptr;
    // 条件变量
<<<<<<< HEAD
    QWaitCondition *audioPackCond = nullptr, *videoPackCond = nullptr, *audioFraCond = nullptr, *videoFraCond = nullptr;
    // 条件变量对应的锁
    //    QMutex audioPackCond_mutex, videoPackCond_mutex;
    QMutex audioFraCond_mutex, videoFraCond_mutex;

    // packetList是共享资源，需要加锁
    QMutex aPacketListMutex;
    QMutex vPacketListMutex;


    // frameList是共享资源，需要加锁
    QMutex aFrameListMutex;
    QMutex vFrameListMutex;
=======
    //    QWaitCondition *audioPackCond = nullptr, *videoPackCond = nullptr, *audioFraCond = nullptr, *videoFraCond = nullptr;
    // 条件变量对应的锁
    //    QMutex audioPackCond_mutex, videoPackCond_mutex;
    //    QMutex audioFraCond_mutex, videoFraCond_mutex;


>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
};

#endif // DATASINK_H
