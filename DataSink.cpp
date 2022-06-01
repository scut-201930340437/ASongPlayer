#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "ASongAudioOutput.h"
#include "SDLPaint.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(DataSink, dataSink) // 采用qt实现的线程安全的单例模式

DataSink::DataSink()
{
    audioFraCond = new QWaitCondition;
    videoFraCond = new QWaitCondition;
}

DataSink* DataSink::getInstance()
{
    return dataSink;
}

AVPacket* DataSink::takeNextPacket(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aPacketListMutex);
        if(aPacketList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            return aPacketList.takeFirst();
        }
    }
    else
    {
        QMutexLocker locker(&vPacketListMutex);
        if(vPacketList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            return vPacketList.takeFirst();
        }
    }
}

AVFrame* DataSink::takeNextFrame(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aFrameListMutex);
        if(aFrameList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            return aFrameList.takeFirst();
        }
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        if(vFrameList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            return vFrameList.takeFirst();
        }
    }
}

AVFrame* DataSink::takeInvertFrame(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aInvertFrameListMutex);
        if(aInvertFrameList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            QList<AVFrame*> *V = aInvertFrameList.first();
            if(V->isEmpty())
            {
                aInvertFrameList.removeFirst();
                if(aInvertFrameList.isEmpty())
                {
                    return nullptr;
                }
                else
                {
                    V = aInvertFrameList.first();
                    return V->takeLast();
                }
            }
            else
            {
                return V->takeLast();
            }
        }
    }
    else
    {
        QMutexLocker locker(&vInvertFrameListMutex);
        if(vInvertFrameList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            QList<AVFrame*> *V = vInvertFrameList.first();
            if(V->isEmpty())
            {
                vInvertFrameList.removeFirst();
                if(vInvertFrameList.isEmpty())
                {
                    return nullptr;
                }
                else
                {
                    V = vInvertFrameList.first();
                    return V->takeLast();
                }
            }
            else
            {
                return V->takeLast();
            }
        }
    }
}

void DataSink::appendPacket(int type, AVPacket *packet)
{
    if(type == 0)
    {
        QMutexLocker locker(&aPacketListMutex);
        aPacketList.append(packet);
    }
    else
    {
        QMutexLocker locker(&vPacketListMutex);
        vPacketList.append(packet);
    }
}

bool DataSink::allowAddFrame(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aFrameListMutex);
        if(aFrameList.size() >= maxAFrameListLength)
        {
            return false;
        }
        return true;
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        if(vFrameList.size() >= maxVFrameListLength)
        {
            return false;
        }
        return true;
    }
}

bool DataSink::allowAddInvertFrameList(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aInvertFrameListMutex);
        if(aInvertFrameList.size() >= maxAInvertFrameListLength)
        {
            return false;
        }
        return true;
    }
    else
    {
        QMutexLocker locker(&vInvertFrameListMutex);
        if(vInvertFrameList.size() >= maxVInvertFrameListLength)
        {
            return false;
        }
        return true;
    }
}

void DataSink::appendFrame(int type, AVFrame *frame)
{
    if(type == 0)
    {
        QMutexLocker locker(&aFrameListMutex);
        aFrameList.append(frame);
        audioFraCond->wakeAll();
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        vFrameList.append(frame);
        videoFraCond->wakeAll();
    }
}

void DataSink::appendInvertFrameList(int type, QList<AVFrame*> *frameList)
{
    if(type == 0)
    {
        QMutexLocker locker(&aInvertFrameListMutex);
        aInvertFrameList.append(frameList);
    }
    else
    {
        QMutexLocker locker(&vInvertFrameListMutex);
        vInvertFrameList.append(frameList);
    }
}

qsizetype DataSink::packetListSize(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aPacketListMutex);
        return aPacketList.size();
    }
    else
    {
        QMutexLocker locker(&vPacketListMutex);
        return vPacketList.size();
    }
}

//void DataSink::clearAPacketList()
//{
//    AVPacket *packet = nullptr;
//    //    QMutexLocker locker(&aPacketListMutex);
//    while(!aPacketList.isEmpty())
//    {
//        packet = aPacketList.takeFirst();
//        av_packet_free(&packet);
//    }
//    aPacketList.clear();
//}

//void DataSink::clearVPacketList()
//{
//    AVPacket *packet = nullptr;
//    //    QMutexLocker locker(&vPacketListMutex);
//    while(!vPacketList.isEmpty())
//    {
//        packet = vPacketList.takeFirst();
//        av_packet_free(&packet);
//    }
//    vPacketList.clear();
//}

//void DataSink::clearAFrameList()
//{
//    AVFrame *frame = nullptr;
//    // 清理队列
//    //    QMutexLocker locker(&aFrameListMutex);
//    while(!aFrameList.isEmpty())
//    {
//        frame = aFrameList.takeFirst();
//        av_frame_free(&frame);
//    }
//    aFrameList.clear();
//}

//void DataSink::clearVFrameList()
//{
//    AVFrame *frame = nullptr;
//    //    QMutexLocker locker(&vFrameListMutex);
//    while(!vFrameList.isEmpty())
//    {
//        frame = vFrameList.takeFirst();
//        av_frame_free(&frame);
//    }
//    vFrameList.clear();
//}

void DataSink::clearList()
{
    AVPacket *packet = nullptr;
    //    QMutexLocker locker(&aPacketListMutex);
    while(!aPacketList.isEmpty())
    {
        packet = aPacketList.takeFirst();
        av_packet_free(&packet);
    }
    aPacketList.clear();
    //
    while(!vPacketList.isEmpty())
    {
        packet = vPacketList.takeFirst();
        av_packet_free(&packet);
    }
    vPacketList.clear();
    AVFrame *frame = nullptr;
    //    QMutexLocker locker(&aFrameListMutex);
    while(!aFrameList.isEmpty())
    {
        frame = aFrameList.takeFirst();
        av_frame_free(&frame);
    }
    aFrameList.clear();
    //    QMutexLocker locker(&vFrameListMutex);
    while(!vFrameList.isEmpty())
    {
        frame = vFrameList.takeFirst();
        av_frame_free(&frame);
    }
    vFrameList.clear();
}

void DataSink::clearInvertList()
{
    AVFrame *frame = nullptr;
    while(!aInvertFrameList.isEmpty())
    {
        QList<AVFrame*> *V = aInvertFrameList.takeFirst();
        while(!V->isEmpty())
        {
            frame = V->takeFirst();
            av_frame_free(&frame);
        }
        V->clear();
    }
    aInvertFrameList.clear();
    while(!vInvertFrameList.isEmpty())
    {
        QList<AVFrame*> *V = vInvertFrameList.takeFirst();
        while(!V->isEmpty())
        {
            frame = V->takeFirst();
            av_frame_free(&frame);
        }
        V->clear();
    }
    vInvertFrameList.clear();
}

void DataSink::frameListIsEmpty(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aFrameListMutex);
        if(aFrameList.isEmpty())
        {
            audioFraCond->wait(&aFrameListMutex);
        }
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        if(vFrameList.isEmpty())
        {
            videoFraCond->wait(&vFrameListMutex);
        }
    }
}

//void DataSink::insertMap(int frameNum, int64_t pts)
//{
//    QMutexLocker locker(&mapMutex);
//    frameNumMap[frameNum] = pts;
//}

//int DataSink::getNumByPts(int64_t pts)
//{
//    QMutexLocker locker(&mapMutex);
//    if(frameNumMap.contains(pts))
//    {
//        return frameNumMap.value(pts);
//    }
//    qDebug() << "seek error pts";
//    return -1;
//}

//int64_t DataSink::getPtsByNum(int frameNum)
//{
//    QMutexLocker locker(&mapMutex);
//    if(frameNumMap.contains(frameNum))
//    {
//        return frameNumMap.value(frameNum);
//    }
//    qDebug() << "seek error frameNum";
//    return -1;
//}
