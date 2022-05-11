#include "DataSink.h"

QAtomicPointer<DataSink> DataSink::_instance = nullptr;
QMutex DataSink::_mutex;

DataSink::DataSink()
{
    audioPackSem = new QSemaphore(0);
    videoPackSem = new QSemaphore(0);
    //    audioFraSem = new QSemaphore(0);
    audioFraCon = new QWaitCondition;
    videoFraCon = new QWaitCondition;
    //    audioFraEmpSem = new QSemaphore(maxFrameListLength);
    //    videoFraEmpSem = new QSemaphore(maxFrameListLength);
}

DataSink::~DataSink()
{
    delete audioPackSem;
    delete videoPackSem;
    delete audioFraCon;
    delete videoFraCon;
    //    delete audioFraSem;
    //    delete audioFraEmpSem;
    //    delete videoFraEmpSem;
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
        audioPackSem->acquire();
        //        audioEmpSem->release();
        return aPacketList.takeFirst();
    }
    else
    {
        videoPackSem->acquire();
        //        videoEmpSem->release();
        return vPacketList.takeFirst();
    }
}

AVFrame* DataSink::takeNextFrame(int type)
{
    if(type == 0)
    {
        if(!aFrameList.isEmpty())
        {
            //        audioFraEmpSem->release();
            AVFrame *frame = aFrameList.takeFirst();
            audioFraCon->wakeOne();
            return frame;
        }
        //        audioFraSem->acquire();
        else
        {
            return nullptr;
        }
    }
    else
    {
        if(!vFrameList.isEmpty())
        {
            //            AVFrame *frame = vFrameList.takeFirst();
            //            videoFraEmpSem->release();
            AVFrame *frame = vFrameList.takeFirst();
            videoFraCon->wakeOne();
            return frame;
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
        audioPackSem->release();
    }
    else
    {
        //        videoEmpSem->acquire();
        vPacketList.append(packet);
        // 先要生产资源，才能up一下信号量
        videoPackSem->release();
    }
}

void DataSink::allowAppendAFrame()
{
    // 上锁
    QMutexLocker locker(&audioFraCon_mutex);
    if(aFrameList.size() > maxFrameListLength)
    {
        audioFraCon->wait(&audioFraCon_mutex);
    }
}

void DataSink::allowAppendVFrame()
{
    // 上锁
    QMutexLocker locker(&videoFraCon_mutex);
    if(vFrameList.size() > maxFrameListLength)
    {
        videoFraCon->wait(&videoFraCon_mutex);
    }
}

void DataSink::appendFrameList(int type, AVFrame *frame)
{
    if(type == 0)
    {
        //        audioFraEmpSem->acquire();
        aFrameList.append(frame);
        //        audioFraSem->release();
    }
    else
    {
        //        videoFraEmpSem->acquire();
        vFrameList.append(frame);
    }
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

void DataSink::wakeAudio()
{
    audioFraCon->wakeOne();
}

void DataSink::wakeVideo()
{
    videoFraCon->wakeOne();
}

void DataSink::clearList()
{
    AVFrame *frame = nullptr;
    // 将资源信号量重置为0
    audioPackSem->acquire(audioPackSem->available());
    videoPackSem->acquire(videoPackSem->available());
    //    audioFraSem->acquire(audioFraSem->available());
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
    // 将空位信号量置为队列最大长度
    //    audioFraEmpSem->release(maxFrameListLength - audioFraEmpSem->available());
    //    videoFraEmpSem->release(maxFrameListLength - videoFraEmpSem->available());
}
