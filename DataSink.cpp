#include "DataSink.h"
#include "ASongFFmpeg.h"

QAtomicPointer<DataSink> DataSink::_instance = nullptr;
QMutex DataSink::_mutex;

DataSink::DataSink()
{
    audioPackCond = new QWaitCondition;
    videoPackCond = new QWaitCondition;
    //    audioFraSem = new QSemaphore(0);
    audioFraCond = new QWaitCondition;
    videoFraCond = new QWaitCondition;
    //    audioFraEmpSem = new QSemaphore(maxFrameListLength);
    //    videoFraEmpSem = new QSemaphore(maxFrameListLength);
}

DataSink::~DataSink()
{
    delete audioPackCond;
    delete videoPackCond;
    delete audioFraCond;
    delete videoFraCond;
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
        if(!aPacketList.isEmpty())
        {
            return aPacketList.takeFirst();
        }
        else
        {
            QMutexLocker locker(&ASongFFmpeg::_hasPacketMutex);
            bool hasPacket = ASongFFmpeg::getInstance()->hasPakcet();
            if(!hasPacket)
            {
                return nullptr;
            }
            audioPackCond->wait(&ASongFFmpeg::_hasPacketMutex);
            if(aPacketList.isEmpty())
            {
                // 说明没有packet
                return nullptr;
            }
            else
            {
                return aPacketList.takeFirst();
            }
        }
    }
    else
    {
        if(!vPacketList.isEmpty())
        {
            return vPacketList.takeFirst();
        }
        else
        {
            QMutexLocker locker(&ASongFFmpeg::_hasPacketMutex);
            bool hasPacket = ASongFFmpeg::getInstance()->hasPakcet();
            if(!hasPacket)
            {
                return nullptr;
            }
            videoPackCond->wait(&ASongFFmpeg::_hasPacketMutex);
            if(vPacketList.isEmpty())
            {
                // 说明没有packet
                return nullptr;
            }
            else
            {
                return vPacketList.takeFirst();
            }
        }
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
            audioFraCond->wakeOne();
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
            videoFraCond->wakeOne();
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
        if(nullptr != packet)
        {
            aPacketList.append(packet);
        }
        // 先要生产资源，才能唤醒线程
        audioPackCond->wakeOne();
    }
    else
    {
        if(nullptr != packet)
        {
            vPacketList.append(packet);
        }
        // 先要生产资源，才能唤醒线程
        videoPackCond->wakeOne();
    }
}

void DataSink::allowAppendAFrame()
{
    // 上锁
    QMutexLocker locker(&audioFraCond_mutex);
    if(aFrameList.size() > maxFrameListLength)
    {
        audioFraCond->wait(&audioFraCond_mutex);
    }
}

void DataSink::allowAppendVFrame()
{
    // 上锁
    QMutexLocker locker(&videoFraCond_mutex);
    if(vFrameList.size() > maxFrameListLength)
    {
        videoFraCond->wait(&videoFraCond_mutex);
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

// 通过frame条件变量唤醒
void DataSink::wakeAudioWithFraCond()
{
    audioFraCond->wakeOne();
}

void DataSink::wakeVideoWithFraCond()
{
    videoFraCond->wakeOne();
}

// 通过packet条件变量唤醒
//void DataSink::wakeAudioWithPackCond()
//{
//    audioPackCond->wakeOne();
//}

//void DataSink::wakeVideoWithPackCond()
//{
//    videoPackCond->wakeOne();
//}

void DataSink::clearList()
{
    AVFrame *frame = nullptr;
    // 将资源信号量重置为0
    //    audioPackSem->acquire(audioPackSem->available());
    //    videoPackSem->acquire(videoPackSem->available());
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
