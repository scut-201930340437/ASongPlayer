
﻿#include "ASongFFmpeg.h"
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
    //    //    audioFraSem = new QSemaphore(0);
    //    audioFraCond = new QWaitCondition;
    //    videoFraCond = new QWaitCondition;
    //    audioFraEmpSem = new QSemaphore(maxFrameListLength);
    //    videoFraEmpSem = new QSemaphore(maxFrameListLength);
}

DataSink::~DataSink()
{
    //    delete audioPackCond;
    //    delete videoPackCond;
    //    delete audioFraCond;
    //    delete videoFraCond;
    //    delete audioFraSem;
    //    delete audioFraEmpSem;
    //    delete videoFraEmpSem;
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
        QMutexLocker aPacketListLocker(&aPacketListMutex);
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
        QMutexLocker vPacketListLocker(&vPacketListMutex);
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
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
        vFrameList.append(frame);
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

// 通过frame条件变量唤醒
//void DataSink::wakeAudioWithFraCond()
//{
//    audioFraCond->wakeAll();
//}

//void DataSink::wakeVideoWithFraCond()
//{
//    videoFraCond->wakeAll();
//}

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
