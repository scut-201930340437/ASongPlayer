#include "DataSink.h"

QAtomicPointer<DataSink> DataSink::_instance = nullptr;
QMutex DataSink::_mutex;

DataSink::DataSink()
{
    audioSem = new QSemaphore(0);
    videoSem = new QSemaphore(0);
    audioFraSem = new QSemaphore(0);
    //    audioEmpSem = new QSemaphore(maxPacketListLength);
    //    videoEmpSem = new QSemaphore(maxPacketListLength);
}

DataSink::~DataSink()
{
    delete audioSem;
    delete videoSem;
    delete audioFraSem;
}

DataSink* DataSink::getInstance()
{
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        //        qDebug() << "----";
        _instance.testAndSetOrdered(nullptr, new DataSink);
    }
    return _instance;
}

AVPacket* DataSink::takeNextPacket(int type)
{
    if(type == 0)
    {
        // 先down一下信号量，再拿资源
        audioSem->acquire();
        //        audioEmpSem->release();
        return aPacketList.takeFirst();
    }
    else
    {
        videoSem->acquire();
        //        videoEmpSem->release();
        return vPacketList.takeFirst();
    }
}

AVFrame* DataSink::takeNextFrame(int type)
{
    if(type == 0)
    {
        audioFraSem->acquire();
        return aFrameList.takeFirst();
    }
    else
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
}

void DataSink::appendPacketList(int type, AVPacket *packet)
{
    if(type == 0)
    {
        //        audioEmpSem->acquire();
        aPacketList.append(packet);
        // 先要生产资源，才能up一下信号量
        audioSem->release();
    }
    else
    {
        //        videoEmpSem->acquire();
        vPacketList.append(packet);
        // 先要生产资源，才能up一下信号量
        videoSem->release();
    }
}

void DataSink::appendFrameList(int type, AVFrame *frame)
{
    if(type == 0)
    {
        aFrameList.append(frame);
        audioFraSem->release();
    }
    else
    {
        vFrameList.append(frame);
    }
}

void DataSink::clearList()
{
    AVFrame *frame = nullptr;
    // 将信号量重置为0
    audioSem->acquire(audioSem->available());
    videoSem->acquire(videoSem->available());
    audioFraSem->acquire(audioFraSem->available());
    // 清理队列
    while(!aFrameList.isEmpty())
    {
        //        audioFraSem->acquire();
        frame = aFrameList.takeFirst();
        av_frame_free(&frame);
    }
    aFrameList.clear();
    //
    while(!vFrameList.isEmpty())
    {
        frame = vFrameList.takeFirst();
        av_frame_free(&frame);
    }
    vFrameList.clear();
    frame = nullptr;
    //
    AVPacket *packet = nullptr;
    while(!aPacketList.isEmpty())
    {
        //        audioSem->acquire();
        packet = aPacketList.takeFirst();
        av_packet_free(&packet);
    }
    aPacketList.clear();
    while(!vPacketList.isEmpty())
    {
        //        videoSem->acquire();
        packet = vPacketList.takeFirst();
        av_packet_free(&packet);
    }
    vPacketList.clear();
    packet = nullptr;
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

qsizetype DataSink::frameListSize(int type)
{
    if(type == 0)
    {
        return aFrameList.size();
    }
    else
    {
        return vFrameList.size();
    }
}
