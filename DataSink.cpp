<<<<<<< HEAD
﻿#include "DataSink.h"
#include "ASongFFmpeg.h"
=======
﻿#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "ASongAudioOutput.h"
#include "SDLPaint.h"
#include "DataSink.h"

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f

Q_GLOBAL_STATIC(DataSink, dataSink) // 采用qt实现的线程安全的单例模式

//QAtomicPointer<DataSink> DataSink::_instance = nullptr;
//QMutex DataSink::_mutex;

DataSink::DataSink()
{
<<<<<<< HEAD
    audioPackCond = new QWaitCondition;
    videoPackCond = new QWaitCondition;
    //    audioFraSem = new QSemaphore(0);
    audioFraCond = new QWaitCondition;
    videoFraCond = new QWaitCondition;
=======
    //    audioPackCond = new QWaitCondition;
    //    videoPackCond = new QWaitCondition;
    //    //    audioFraSem = new QSemaphore(0);
    //    audioFraCond = new QWaitCondition;
    //    videoFraCond = new QWaitCondition;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //    audioFraEmpSem = new QSemaphore(maxFrameListLength);
    //    videoFraEmpSem = new QSemaphore(maxFrameListLength);
}

DataSink::~DataSink()
{
<<<<<<< HEAD
    delete audioPackCond;
    delete videoPackCond;
    delete audioFraCond;
    delete videoFraCond;
=======
    //    delete audioPackCond;
    //    delete videoPackCond;
    //    delete audioFraCond;
    //    delete videoFraCond;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
<<<<<<< HEAD
        QMutexLocker aPacketListlocker(&aPacketListMutex);
        if(!aPacketList.isEmpty())
        {
            return aPacketList.takeFirst();
        }
        else
        {
            aPacketListlocker.unlock();
            QMutexLocker locker(&ASongFFmpeg::_hasPacketMutex);
            bool hasPacket = ASongFFmpeg::getInstance()->hasPakcet();
            if(!hasPacket)
            {
                return nullptr;
            }
            audioPackCond->wait(&ASongFFmpeg::_hasPacketMutex);
            aPacketListlocker.relock();
            if(aPacketList.isEmpty())
            {
                // 说明没有packet
                return nullptr;
            }
            else
            {
                return aPacketList.takeFirst();
            }
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
    }
    else
    {
<<<<<<< HEAD
        QMutexLocker vPacketListlocker(&vPacketListMutex);
        if(!vPacketList.isEmpty())
        {
            return vPacketList.takeFirst();
        }
        else
        {
            vPacketListlocker.unlock();
            QMutexLocker locker(&ASongFFmpeg::_hasPacketMutex);
            bool hasPacket = ASongFFmpeg::getInstance()->hasPakcet();
            if(!hasPacket)
            {
                return nullptr;
            }
            videoPackCond->wait(&ASongFFmpeg::_hasPacketMutex);
            vPacketListlocker.relock();
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
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    }
}

AVFrame* DataSink::takeNextFrame(int type)
{
    if(type == 0)
    {
        QMutexLocker locker(&aFrameListMutex);
<<<<<<< HEAD
        if(!aFrameList.isEmpty())
        {
            AVFrame *frame = aFrameList.takeFirst();
            audioFraCond->wakeOne();
            return frame;
        }
        else
        {
            return nullptr;
=======
        if(aFrameList.isEmpty())
        {
            return nullptr;
        }
        else
        {
            //            AVFrame *frame = aFrameList.takeFirst();
            //            audioFraCond->wakeAll();
            return aFrameList.takeFirst();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
    }
    else
    {
        QMutexLocker locker(&vFrameListMutex);
<<<<<<< HEAD
        if(!vFrameList.isEmpty())
        {
            AVFrame *frame = vFrameList.takeFirst();
            videoFraCond->wakeOne();
            return frame;
        }
        else
        {
            return nullptr;
=======
        if(vFrameList.isEmpty())
        {
            //            AVFrame *frame = vFrameList.takeFirst();
            //            videoFraCond->wakeAll();
            return nullptr;
        }
        else
        {
            return vFrameList.takeFirst();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
    }
}

void DataSink::appendPacketList(int type, AVPacket *packet)
{
    if(type == 0)
    {
<<<<<<< HEAD
        if(nullptr != packet)
        {
            QMutexLocker locker(&aPacketListMutex);
            aPacketList.append(packet);
        }
        // 先要生产资源，才能唤醒线程
        audioPackCond->wakeOne();
    }
    else
    {
        if(nullptr != packet)
        {
            QMutexLocker locker(&vPacketListMutex);
            vPacketList.append(packet);
        }
        // 先要生产资源，才能唤醒线程
        videoPackCond->wakeOne();
    }
}

void DataSink::allowAppendAFrame()
{
    // 上锁
    QMutexLocker locker(&aFrameListMutex);
    if(aFrameList.size() > maxFrameListLength)
    {
        audioFraCond->wait(&aFrameListMutex);
    }
}

void DataSink::allowAppendVFrame()
{
    // 上锁
    QMutexLocker locker(&vFrameListMutex);
    if(vFrameList.size() > maxFrameListLength)
    {
        videoFraCond->wait(&vFrameListMutex);
    }
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

void DataSink::appendFrameList(int type, AVFrame *frame)
{
    if(type == 0)
    {
<<<<<<< HEAD
        //        audioFraEmpSem->acquire();
        QMutexLocker locker(&aFrameListMutex);
        aFrameList.append(frame);
        //        audioFraSem->release();
    }
    else
    {
        //        videoFraEmpSem->acquire();
=======
        QMutexLocker locker(&aFrameListMutex);
        aFrameList.append(frame);
    }
    else
    {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
<<<<<<< HEAD
void DataSink::wakeAudioWithFraCond()
{
    audioFraCond->wakeOne();
}

void DataSink::wakeVideoWithFraCond()
{
    videoFraCond->wakeOne();
}
=======
//void DataSink::wakeAudioWithFraCond()
//{
//    audioFraCond->wakeAll();
//}

//void DataSink::wakeVideoWithFraCond()
//{
//    videoFraCond->wakeAll();
//}
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f

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
