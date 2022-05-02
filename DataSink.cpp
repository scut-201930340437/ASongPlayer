#include "DataSink.h"

QAtomicPointer<DataSink> DataSink::_instance = nullptr;
QMutex DataSink::_mutex;

DataSink::DataSink()
{
    audioSem = new QSemaphore(0);
    videoSem = new QSemaphore(0);
}

DataSink::~DataSink()
{
    delete audioSem;
    delete videoSem;
}

DataSink* DataSink::getInstance()
{
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        _instance.testAndSetOrdered(nullptr, new DataSink);
    }
    return _instance;
}

AVPacket* DataSink::takeNextPacket(int type)
{
    if(type == 0)
    {
        //        QMutexLocker locker(&aPacketListMutex);
        audioSem->acquire();
        return aPacketList.takeFirst();
    }
    else
    {
        //        QMutexLocker locker(&vPacketListMutex);
        videoSem->acquire();
        return vPacketList.takeFirst();
    }
}

AVFrame* DataSink::takeNextFrame()
{
    if(!vFrameList.isEmpty())
    {
        return vFrameList.takeFirst();
    }
    else
    {
        return nullptr;
    }
}

void DataSink::appendPacketList(int type, AVPacket *packet)
{
    if(type == 0)
    {
        //        QMutexLocker locker(&aPacketListMutex);
        audioSem->release();
        aPacketList.append(packet);
    }
    else
    {
        //        QMutexLocker locker(&vPacketListMutex);
        videoSem->release();
        vPacketList.append(packet);
    }
}

void DataSink::appendFrameList(AVFrame *frame)
{
    vFrameList.append(frame);
}

qsizetype DataSink::packetListSize(int type)
{
    if(type == 0)
    {
        return aPacketList.size();
    }
    else
    {
        return vPacketList.size();
    }
}

qsizetype DataSink::frameListSize()
{
    return vFrameList.size();
}
