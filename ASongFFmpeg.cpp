#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongAudioOutput.h"
#include "ASongVideo.h"
#include "VideoPreview.h"
#include "DataSink.h"

Q_GLOBAL_STATIC(ASongFFmpeg, asongFFmpeg) // 采用qt实现的线程安全的单例模式

// 全局访问点
ASongFFmpeg* ASongFFmpeg::getInstance()
{
    return asongFFmpeg;
}

MediaMetaData* ASongFFmpeg::openMediaInfo(QString path, AVFormatContext* pFmtCtx)
{
    bool ctx_not_null = pFmtCtx;
    AVFormatContext *ctx = NULL;
    MediaMetaData* ret = NULL;
    //传入非空ctx
    if(ctx_not_null)
    {
        ctx = pFmtCtx;
    }
    else if(!(ctx = avformat_alloc_context()))
    {
        qDebug() << "findMediaInfo: Could not allocate context";
        return ret;
    }
    //打开
    if(avformat_open_input(&ctx, path.toStdString().c_str(), NULL, NULL) < 0)
    {
        qDebug() << "findMediaInfo: open input error";
        return ret;
    }
    //获取媒体信息
    if(avformat_find_stream_info(ctx, NULL) < 0)
    {
        qDebug() << "findMediaInfo: find media info error";
        return ret;
    }
    //构建MediaMetaData
    ret = new MediaMetaData;
    ret->path = path;
    ret->filename = path.split("/").back();
    ret->durationSec = ctx->duration / AV_TIME_BASE;
    //获取title, artist, album信息
    AVDictionaryEntry *tag = NULL;
    ret->title = ret->artist = ret->album = "";
    while((tag = av_dict_get(ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        QString keyString = tag->key;
        if(keyString == "title")
        {
            ret->title = QString::fromUtf8(tag->value);
        }
        else if(keyString == "artist")
        {
            ret->artist = QString::fromUtf8(tag->value);
        }
        else if(keyString == "album")
        {
            ret->album = QString::fromUtf8(tag->value);
        }
    }
    //获取所有streams信息
    ret->aMetaDatas = NULL;
    ret->vMetaDatas = NULL;
    ret->nb_astreams = ret->nb_vstreams = 0;
    for(int i = 0; i < ctx->nb_streams; i++)
    {
        AVStream *as = ctx->streams[i];
        if(AVMEDIA_TYPE_AUDIO == as->codecpar->codec_type)
        {
            ret->nb_astreams++;
        }
        else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type)
        {
            ret->nb_vstreams++;
        }
    }
    if(ret->nb_astreams != 0)
    {
        ret->aMetaDatas = new AudioMetaData[ret->nb_astreams];
    }
    if(ret->nb_vstreams != 0)
    {
        ret->vMetaDatas = new VideoMetaData[ret->nb_vstreams];
    }
    for(int i = 0, ai = 0, vi = 0; i < ctx->nb_streams; i++)
    {
        AVStream *as = ctx->streams[i];
        if(AVMEDIA_TYPE_AUDIO == as->codecpar->codec_type)
        {
            AudioMetaData& amd = ret->aMetaDatas[ai];
            amd.idx = i;
            amd.bit_rate = as->codecpar->bit_rate;
            amd.sample_rate = as->codecpar->sample_rate;
            amd.nb_channels = as->codecpar->channels;
            amd.codec_id = as->codecpar->codec_id;
            ai++;
        }
        else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type)
        {
            VideoMetaData& vmd = ret->vMetaDatas[vi];
            vmd.idx = i;
            vmd.width = as->codecpar->width;
            vmd.height = as->codecpar->height;
            vmd.codec_id = as->codecpar->codec_id;
            if(as->disposition & AV_DISPOSITION_ATTACHED_PIC) //有封面图
            {
                vmd.cover = new QImage;
                AVPacket pkt = as->attached_pic;
                *vmd.cover = QImage::fromData((uchar*)pkt.data, pkt.size);
                vmd.bit_rate = -1;
                vmd.frame_rate = -1;
            }
            else
            {
                vmd.cover = nullptr;
                vmd.bit_rate = as->codecpar->bit_rate;
                vmd.frame_rate = av_q2d(as->avg_frame_rate);
            }
            vi++;
        }
    }
    if(!ctx_not_null)
    {
        avformat_close_input(&ctx);
    }
    return ret;
}

void ASongFFmpeg::closeMediaInfo(MediaMetaData *mmd)
{
    if(mmd->nb_astreams != 0)
    {
        delete []mmd->aMetaDatas;
    }
    if(mmd->nb_vstreams != 0)
    {
        for(int i = 0; i < mmd->nb_vstreams; i++)
        {
            VideoMetaData& vmd = mmd->vMetaDatas[i];
            if(vmd.cover != nullptr)
            {
                delete vmd.cover;
            }
        }
        delete []mmd->vMetaDatas;
    }
    delete mmd;
}

// 加载文件信息
int ASongFFmpeg::load(QString path)
{
    hasCover = false;
    pFormatCtx = avformat_alloc_context();
    mediaMetaData = openMediaInfo(path, pFormatCtx);
    int ret;
    // 分出视频流和音频流
    AVCodecContext *pACodecCtx = nullptr, *pVCodecCtx = nullptr;
    AVCodec *pACodec = nullptr, *pVCodec = nullptr;
    audioIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pACodec, 0);
    // 找到音频流
    if(audioIdx != AVERROR_STREAM_NOT_FOUND)
    {
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[audioIdx]->codecpar;
        for(int i = 0; i < mediaMetaData->nb_astreams; i++)
        {
            if(audioIdx == mediaMetaData->aMetaDatas[i].idx)
            {
                audioIdx = i;
                break;
            }
        }
        if(!pACodec)
        {
            qDebug() << "Couldn't find audio code.";
            return -1;
        }
        // 获取解码器上下文
        pACodecCtx = avcodec_alloc_context3(pACodec);
        // 将pCodecPara中的参数传给pCodecCtx
        ret = avcodec_parameters_to_context(pACodecCtx, pCodecPara);
        if(ret < 0)
        {
            printf("Cannot alloc codec context.\n");
            return -1;
        }
        // 打开解码器
        ret = avcodec_open2(pACodecCtx, pACodec, nullptr);
        if(ret < 0)
        {
            qDebug() << "Couldn't open audio code.";
            return -1;
        }
    }
    // 找视频流
    videoIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVCodec, 0);
    if(videoIdx != AVERROR_STREAM_NOT_FOUND)
    {
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[videoIdx]->codecpar;
        for(int i = 0; i < mediaMetaData->nb_vstreams; i++)
        {
            if(videoIdx == mediaMetaData->vMetaDatas[i].idx)
            {
                videoIdx = i;
                break;
            }
        }
        hasCover = (nullptr != mediaMetaData->vMetaDatas[videoIdx].cover);
        if(!pVCodec)
        {
            qDebug() << "Couldn't find video code.";
            return -1;
        }
        // 获取解码器上下文
        pVCodecCtx = avcodec_alloc_context3(pVCodec);
        // 将pCodecPara中的参数传给pCodecCtx
        ret = avcodec_parameters_to_context(pVCodecCtx, pCodecPara);
        if(ret < 0)
        {
            qDebug() << "Cannot alloc codec context";
            return -1;
        }
        // 打开解码器
        ret = avcodec_open2(pVCodecCtx, pVCodec, nullptr);
        if(ret < 0)
        {
            qDebug() << "Couldn't open vidoe code.";
            return -1;
        }
        // 获取视频流元数据
        // 获取帧率，对于带封面的音频文件，av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate)为nan
    }
    // 找不到音频流和视频流
    if(videoIdx < 0 && audioIdx < 0)
    {
        qDebug() << "file error";
        return -1;
    }
    if(audioIdx >= 0)
    {
        ASongAudio::getInstance()->setMetaData(pFormatCtx, pACodecCtx, mediaMetaData->aMetaDatas[audioIdx].idx);
    }
    if(videoIdx >= 0)
    {
        int idx = mediaMetaData->vMetaDatas[videoIdx].idx;
        ASongVideo::getInstance()->setMetaData(pVCodecCtx, idx,
                                               pFormatCtx->streams[idx]->time_base);
        SDLPaint::getInstance()->setMetaData(mediaMetaData->vMetaDatas[videoIdx].width,
                                             mediaMetaData->vMetaDatas[videoIdx].height,
                                             mediaMetaData->vMetaDatas[videoIdx].frame_rate,
                                             pVCodecCtx->pix_fmt, pFormatCtx->streams[mediaMetaData->vMetaDatas[videoIdx].idx]->time_base);
    }
    pACodec = pVCodec = nullptr;
    return 0;
}

AVPacket* ASongFFmpeg::readFrame()
{
    if(nullptr == pFormatCtx)
    {
        // 文件未打开
        return nullptr;
    }
    AVPacket* packet = av_packet_alloc();
    int ret = av_read_frame(pFormatCtx, packet);
    if(ret < 0)
    {
        // 所有的packet已经读取完毕
        av_packet_free(&packet);
        packet = nullptr;
    }
    return packet;
}

int ASongFFmpeg::getMediaStatus()
{
    return curMediaStatus;
}

int ASongFFmpeg::getDuration()
{
    if(mediaMetaData != nullptr)
    {
        return mediaMetaData->durationSec;
    }
    return 0;
}

int64_t ASongFFmpeg::getCurPlaySec()
{
    return int64_t(ASongAudio::getInstance()->getAudioClock());
}

QString ASongFFmpeg::getFilepath()
{
    if(mediaMetaData != nullptr)
    {
        return mediaMetaData->path;
    }
    return "";
}

float ASongFFmpeg::getSpeed()
{
    return ASongAudioOutput::getInstance()->getSpeed();
}

// 开始播放
int ASongFFmpeg::play(QObject *par, QString path, QWidget *_playWidget)
{
    // 切换为播放态
    curMediaStatus = 1;
    // 加载媒体文件信息,打开解码器
    load(path);
    ASongAudioOutput::getInstance()->createMediaDevice(par);
    // 读取packet线程启动
    start();
    if(videoIdx >= 0)
    {
        SDLPaint::getInstance()->init(_playWidget);
        // 预览线程启动
        VideoPreview::getInstance()->start(path, mediaMetaData->vMetaDatas[videoIdx].idx);
    }
    return 0;
}

void ASongFFmpeg::resetPara()
{
    stopReq = false;
    pauseReq = false;
    pauseFlag = false;
    invertFlag = false;
    invertReq = false;
    seekReq = false;
    stepSeek = false;
    seekAudio = false;
    seekVideo = false;
    needInvertSeek = false;
    targetFrameNum = -1;
    // 进度微调的目标pts
    targetPts = 0.0;
    step = 0;
    invertPts = 0.0;
    seekPos = 0;
    seekRel = 0;
    seekMin = INT64_MIN;
    seekMax = INT64_MAX;
    seekFlag = -1;
    cleared = false;
}

// thread
void ASongFFmpeg::start(Priority pri)
{
    // 重置参数
    resetPara();
    QThread::start(pri);
}

int ASongFFmpeg::stop()
{
    if(nullptr == pFormatCtx)
    {
        return -1;
    }
    // 先结束音频播放线程和关闭音频设备
    ASongAudioOutput::getInstance()->stop();
    // 结束视频渲染
    SDLPaint::getInstance()->stop();
    // 结束解复用线程
    if(QThread::isRunning())
    {
        stopReq = true;
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    // 结束音频解码
    ASongAudio::getInstance()->stop();
    // 结束视频解码、预览
    if(videoIdx >= 0)
    {
        ASongVideo::getInstance()->stop();
        VideoPreview::getInstance()->stop();
    }
    // 清空队列
    DataSink::getInstance()->clearList();
    DataSink::getInstance()->clearInvertList();
    // 关闭文件流上下文
    if(nullptr != pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = nullptr;
    }
    if(nullptr != mediaMetaData)
    {
        closeMediaInfo(mediaMetaData);
        mediaMetaData = nullptr;
    }
    curMediaStatus = 0;
    resetPara();
    return 0;
}

int ASongFFmpeg::pause()
{
    curMediaStatus = 2;
    // 阻塞音频播放线程
    ASongAudioOutput::getInstance()->pause();
    // 暂停sdlpaint
    if(videoIdx >= 0)
    {
        SDLPaint::getInstance()->pause();
    }
    return 0;
}

void ASongFFmpeg::pauseThread()
{
    QMutexLocker locker(&_pauseMutex);
    if(!pauseFlag && QThread::isRunning())
    {
        pauseReq = true;
        pauseCond.wait(&_pauseMutex);
        locker.relock();
    }
}

int ASongFFmpeg::resume()
{
    // 切换为播放态
    curMediaStatus = 1;
    // 音频播放线程恢复
    ASongAudioOutput::getInstance()->resume();
    // 带封面的音频在不是重新播放的情况下不启动视频解码线程和SDL
    if(videoIdx >= 0)
    {
        SDLPaint::getInstance()->resume();
    }
    return 0;
}

void ASongFFmpeg::resumeThread()
{
    QMutexLocker locker(&_pauseMutex);
    if(pauseFlag && QThread::isRunning())
    {
        pauseReq = false;
        pauseCond.wakeAll();
        pauseCond.wait(&_pauseMutex);
    }
    else if(QThread::isFinished())
    {
        start();
    }
}

void ASongFFmpeg::run()
{
    // 初始化音频各参数及设备
    ASongAudio::getInstance()->initParaAndSwr();
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->start();
    ASongAudioOutput::getInstance()->start();
    if(videoIdx >= 0)
    {
        // 视频解码线程启动
        ASongVideo::getInstance()->start();
    }
    for(;;)
    {
        if(stopReq)
        {
            stopReq = false;
            break;
        }
        if(pauseReq)
        {
            QMutexLocker locker(&_pauseMutex);
            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 线程阻塞
            pauseCond.wait(&_pauseMutex);
            locker.relock();
            pauseFlag = false;
            // 唤醒主线程
            pauseCond.wakeAll();
        }
        if(invertReq)
        {
            initInvert();
        }
        if(needInvertSeek)
        {
            handleInvertSeek();
        }
        if(seekReq)
        {
            handleSeek();
        }
        if(DataSink::getInstance()->packetListSize(0) >= DataSink::maxAPacketListLength
                || DataSink::getInstance()->packetListSize(1) >= DataSink::maxVPacketListLength)
        {
            msleep(25);
        }
        else
        {
            AVPacket *packet = readFrame();
            if(nullptr == packet)
            {
                stopReq = true;
                break;
            }
            // 如果是音频
            if(packet->stream_index == mediaMetaData->aMetaDatas[audioIdx].idx)
            {
                DataSink::getInstance()->appendPacket(0, packet);
            }
            else
            {
                if(packet->stream_index == mediaMetaData->vMetaDatas[videoIdx].idx)
                {
                    DataSink::getInstance()->appendPacket(1, packet);
                }
            }
        }
    }
    //    QMutexLocker locker(&stopMutex);
    //    stopFlag = true;
    //    locker.unlock();
    // 解复用结束
}

void ASongFFmpeg::handleSeek()
{
    // 停止音频播放和视频渲染
    if(!stepSeek && !invertFlag)
    {
        pause();
    }
    // 阻塞解码线程
    ASongAudio::getInstance()->pauseThread();
    ASongVideo::getInstance()->pauseThread();
    // 清理正常播放的队列
    DataSink::getInstance()->clearList();
    // 设置清空标志位
    if(stepSeek)
    {
        QMutexLocker locker(&clearListMutex);
        cleared = true;
        clearListCond.wakeAll();
        locker.unlock();
    }
    // 放入flushpkt
    if(audioIdx >= 0)
    {
        AVPacket *flushPacket = av_packet_alloc();
        flushPacket->data = (uint8_t*)flushPacket;
        DataSink::getInstance()->appendPacket(0, flushPacket);
    }
    if(videoIdx >= 0 && !hasCover)
    {
        AVPacket *flushPacket = av_packet_alloc();
        flushPacket->data = (uint8_t*)flushPacket;
        DataSink::getInstance()->appendPacket(1, flushPacket);
    }
    // seek_file
    int ret = avformat_seek_file(pFormatCtx, -1, seekMin, seekPos, seekMax, seekFlag);
    if(ret < 0)
    {
        qDebug() << "seek failed";
    }
    // 唤醒解码线程
    ASongAudio::getInstance()->resumeThread();
    if(videoIdx >= 0 && !hasCover)
    {
        ASongVideo::getInstance()->resumeThread();
    }
    // 重启音频播放线程和渲染timer
    if(!stepSeek)
    {
        resume();
    }
    // seek后，重置req和flag
    seekReq = false;
    seekFlag = -1;
}

void ASongFFmpeg::seek(int64_t posSec)
{
    if(nullptr == pFormatCtx || curMediaStatus <= 0 || invertFlag)
    {
        return;
    }
    // 阻塞解复用线程
    pauseThread();
    // 设置seek请求
    seekPos = posSec * AV_TIME_BASE;
    seekRel = seekPos - getCurPlaySec() * AV_TIME_BASE;
    seekMin = seekRel > 0 ? seekPos - seekRel + 2 : INT64_MIN;
    seekMax = seekRel < 0 ? seekPos - seekRel - 2 : INT64_MAX;
    seekFlag = AVSEEK_FLAG_BACKWARD;
    seekReq = true;
    // 唤醒解复用线程
    resumeThread();
}

// 逐帧
void ASongFFmpeg::step_to_dst_frame(int _step)
{
    if(nullptr == pFormatCtx || curMediaStatus <= 0 || videoIdx < 0 || hasCover || invertFlag)
    {
        return;
    }
    // 停止渲染定时器
    SDLPaint::getInstance()->stopTimer();
    // 设置sdl非暂停态
    SDLPaint::getInstance()->resume();
    // 没有下一帧或下五帧 或没有上一帧或上五帧
    targetFrameNum = SDLPaint::getInstance()->curFrameNum + _step;
    if(targetFrameNum > pFormatCtx->streams[mediaMetaData->vMetaDatas[videoIdx].idx]->nb_frames - 1 || targetFrameNum < 0)
    {
        SDLPaint::getInstance()->restartTimer();
        return;
    }
    curMediaStatus = 2;
    // 暂停音频播放线程
    ASongAudioOutput::getInstance()->pause();
    step = _step;
    seekAudio = true;
    seekVideo = true;
    stepSeek = true;
    // 向前跳
    if(step < 0)
    {
        // 阻塞解复用线程
        pauseThread();
        // 设置seek请求
        targetPts = SDLPaint::getInstance()->curPts + step * SDLPaint::getInstance()->basePts;
        seekPos = FFMAX((targetPts - 1.0 * SDLPaint::getInstance()->basePts) * AV_TIME_BASE, 0);
        seekMin = INT64_MIN;
        seekMax = seekPos;
        seekFlag = AVSEEK_FLAG_BACKWARD;
        seekReq = true;
        // 唤醒解复用线程
        resumeThread();
        // 等待队列清理完成
        QMutexLocker locker(&clearListMutex);
        while(!cleared)
        {
            clearListCond.wait(&clearListMutex);
        }
        locker.unlock();
        cleared = false;
        // 等待帧解码并放入队列
        //        DataSink::getInstance()->frameListIsEmpty(0);
        //        // 等待帧解码并放入队列
        //        DataSink::getInstance()->frameListIsEmpty(1);
    }
    // 播放一帧音频，丢弃不是目标位置的帧
    while(seekAudio)
    {
        ASongAudioOutput::getInstance()->resume();
    }
    // 渲染一帧，丢弃不是目标位置的帧
    while(seekVideo)
    {
        SDLPaint::getInstance()->getFrameYUV();
    }
    // 设置sdl暂停态
    SDLPaint::getInstance()->pause();
    // 重启定时器使sdl不断渲染上一帧
    SDLPaint::getInstance()->restartTimer();
    stepSeek = false;
    step = 0;
}
// 设置速率
void ASongFFmpeg::setSpeed(float _speed)
{
    ASongAudioOutput::getInstance()->setSpeed(_speed);
    // 调整队列最大长度和线程睡眠时长
    if(_speed >= 7.999)
    {
        DataSink::maxAPacketListLength = 200;
        DataSink::maxVPacketListLength = 140;
        DataSink::maxAFrameListLength = 240;
        DataSink::maxVFrameListLength = 140;
        DataSink::maxAInvertFrameListLength = 270;
        DataSink::maxVInvertFrameListLength = 180;
        ASongAudio::getInstance()->setSleepTime(5);
        ASongVideo::getInstance()->setSleepTime(5);
        sleepTime = 5;
    }
    else
    {
        if(_speed >= 3.999)
        {
            DataSink::maxAPacketListLength = 130;
            DataSink::maxVPacketListLength = 100;
            DataSink::maxAFrameListLength = 150;
            DataSink::maxVFrameListLength = 100;
            DataSink::maxAInvertFrameListLength = 160;
            DataSink::maxVInvertFrameListLength = 120;
            ASongAudio::getInstance()->setSleepTime(10);
            ASongVideo::getInstance()->setSleepTime(10);
            sleepTime = 15;
        }
        else
        {
            DataSink::maxAPacketListLength = 110;
            DataSink::maxVPacketListLength = 70;
            DataSink::maxAFrameListLength = 120;
            DataSink::maxVFrameListLength = 70;
            DataSink::maxAInvertFrameListLength = 100;
            DataSink::maxVInvertFrameListLength = 70;
            ASongAudio::getInstance()->setSleepTime(30);
            ASongVideo::getInstance()->setSleepTime(60);
            sleepTime = 25;
        }
    }
}

void ASongFFmpeg::initInvert()
{
    pause();
    invertPts = SDLPaint::getInstance()->curPts;
    // 阻塞解码线程
    ASongAudio::getInstance()->pauseThread();
    ASongVideo::getInstance()->pauseThread();
    if(invertFlag)
    {
        // 清除倒放的残留帧
        DataSink::getInstance()->clearInvertList();
        invertFlag = false;
    }
    else
    {
        invertFlag = true;
    }
    needInvertSeek = true;
    invertReq = false;
}

void ASongFFmpeg::handleInvertSeek()
{
    // 设置seek请求
    seekPos = FFMAX((invertPts - 0.5 * SDLPaint::getInstance()->basePts) * AV_TIME_BASE, 0);
    seekMin = INT64_MIN;
    seekMax = seekPos;
    seekFlag = AVSEEK_FLAG_BACKWARD;
    seekReq = true;
    needInvertSeek = false;
}

void ASongFFmpeg::invertPlay()
{
    if(nullptr == pFormatCtx || curMediaStatus <= 0 || videoIdx < 0 || hasCover)
    {
        return;
    }
    pauseThread();
    // 设置倒放请求
    invertReq = true;
    resumeThread();
}

void ASongFFmpeg::hideCursor()
{
    SDL_ShowCursor(false);
}

void ASongFFmpeg::showCursor()
{
    SDL_ShowCursor(true);
}
