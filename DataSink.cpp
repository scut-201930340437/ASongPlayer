#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "ASongAudioOutput.h"
#include "SDLPaint.h"
#include "DataSink.h"


Q_GLOBAL_STATIC(DataSink, dataSink) // 采用qt实现的线程安全的单例模式

//QAtomicPointer<DataSink> DataSink::_instance = nullptr;
//QMutex DataSink::_mutex;

DataSink::DataSink()
{
    //    audioPackCond = new QWaitCondition;
    //    videoPackCond = new QWaitCondition;
    audioFraCond = new QWaitCondition;
    videoFraCond = new QWaitCondition;
}


DataSink* DataSink::getInstance()
{
    //    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    //        //        qDebug() << "----";
    //        _instance.testAndSetOrdered(nullptr, new DataSink);
    //    }
    return dataSink;
}

AVPacket* DataSink::takeNextPacket(int type)
{
    if(type == 0)
    {
        //        QMutexLocker aPacketListLocker(&aPacketListMutex);
        //        while(aPacketList.isEmpty() && !ASongFFmpeg::getInstance()->stopFlag)
        //        {
        //            audioPackCond->wait(&aPacketListMutex);
        //        }
        //        if(!aPacketList.isEmpty())
        //        {
        //            return aPacketList.takeFirst();
        //        }
        //        else
        //        {
        //            return nullptr;
        //        }
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
        //        while(vPacketList.isEmpty() && !ASongFFmpeg::getInstance()->stopFlag)
        //        {
        //            videoPackCond->wait(&vPacketListMutex);
        //        }
        //        if(!vPacketList.isEmpty())
        //        {
        //            return vPacketList.takeFirst();
        //        }
        //        else
        //        {
        //            return nullptr;
        //        }
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
            //            AVFrame *frame = aFrameList.takeFirst();
            //            audioFraCond->wakeAll();
            return aFrameList.takeFirst();
        }
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        if(vFrameList.isEmpty())
        {
            //            AVFrame *frame = vFrameList.takeFirst();
            //            videoFraCond->wakeAll();
            return nullptr;
        }
        else
        {
            return vFrameList.takeFirst();
        }
    }
}

void DataSink::appendPacketList(int type, AVPacket *packet)
{
    if(type == 0)
    {
        QMutexLocker locker(&aPacketListMutex);
        aPacketList.append(packet);
        //            audioPackCond->wakeAll();
        //        else
        //        {
        //            audioPackCond->wakeAll();
        //        }
    }
    else
    {
        QMutexLocker locker(&vPacketListMutex);
        vPacketList.append(packet);
        //            videoPackCond->wakeAll();
        //        else
        //        {
        //            videoPackCond->wakeAll();
        //        }
    }
}

bool DataSink::allowAddAFrame()
{
    QMutexLocker locker(&aFrameListMutex);
    if(aFrameList.size() >= maxAFrameListLength)
    {
        //        ASongAudio::getInstance()->pauseFlag = true;
        //        audioFraCond->wait(&aFrameListMutex);
        //        ASongAudio::getInstance()->pauseFlag = false;
        return false;
    }
    return true;
}

bool DataSink::allowAddVFrame()
{
    QMutexLocker locker(&vFrameListMutex);
    if( vFrameList.size() >= maxVFrameListLength)
    {
        //        ASongVideo::getInstance()->pauseFlag = true;
        //        videoFraCond->wait(&vFrameListMutex);
        //        ASongVideo::getInstance()->pauseFlag = false;
        return false;
    }
    return true;
}

void DataSink::appendFrameList(int type, AVFrame *frame)
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

void DataSink::clearAPacketList()
{
    AVPacket *packet = nullptr;
    //    QMutexLocker locker(&aPacketListMutex);
    while(!aPacketList.isEmpty())
    {
        packet = aPacketList.takeFirst();
        av_packet_free(&packet);
    }
    aPacketList.clear();
}

void DataSink::clearVPacketList()
{
    AVPacket *packet = nullptr;
    //    QMutexLocker locker(&vPacketListMutex);
    while(!vPacketList.isEmpty())
    {
        packet = vPacketList.takeFirst();
        av_packet_free(&packet);
    }
    vPacketList.clear();
}

void DataSink::clearAFrameList()
{
    AVFrame *frame = nullptr;
    // 清理队列
    //    QMutexLocker locker(&aFrameListMutex);
    while(!aFrameList.isEmpty())
    {
        frame = aFrameList.takeFirst();
        av_frame_free(&frame);
    }
    aFrameList.clear();
}

void DataSink::clearVFrameList()
{
    AVFrame *frame = nullptr;
    //    QMutexLocker locker(&vFrameListMutex);
    while(!vFrameList.isEmpty())
    {
        frame = vFrameList.takeFirst();
        av_frame_free(&frame);
    }
    vFrameList.clear();
}

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
