#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "ASongAudioOutput.h"
#include "SDLPaint.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(DataSink, dataSink) // 采用qt实现的线程安全的单例模式

qsizetype DataSink::maxAPacketListLength = 120;
qsizetype DataSink::maxVPacketListLength = 100;
// 帧list最大长度
qsizetype DataSink::maxAFrameListLength = 120;
qsizetype DataSink::maxVFrameListLength = 100;
qsizetype DataSink::maxAInvertFrameListLength = 120;
qsizetype DataSink::maxVInvertFrameListLength = 100;

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
                    --aInvertFrameSum;
                    return V->takeLast();
                }
            }
            else
            {
                --aInvertFrameSum;
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
                    --vInvertFrameSum;
                    return V->takeLast();
                }
            }
            else
            {
                --vInvertFrameSum;
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
        if(aInvertFrameSum >= maxAFrameListLength)
        {
            return false;
        }
        return true;
    }
    else
    {
        QMutexLocker locker(&vInvertFrameListMutex);
        if(vInvertFrameSum >= maxVFrameListLength)
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
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        vFrameList.append(frame);
    }
}

void DataSink::appendInvertFrameList(int type, QList<AVFrame*> *frameList)
{
    if(type == 0)
    {
        QMutexLocker locker(&aInvertFrameListMutex);
        aInvertFrameList.append(frameList);
        aInvertFrameSum += frameList->size();
    }
    else
    {
        QMutexLocker locker(&vInvertFrameListMutex);
        vInvertFrameList.append(frameList);
        vInvertFrameSum += frameList->size();
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

void DataSink::clearList()
{
    AVPacket *packet = nullptr;
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
    while(!aFrameList.isEmpty())
    {
        frame = aFrameList.takeFirst();
        av_frame_free(&frame);
    }
    aFrameList.clear();
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
    aInvertFrameSum = vInvertFrameSum = 0;
}
