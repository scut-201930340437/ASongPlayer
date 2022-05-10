#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include "DataSink.h"


QAtomicPointer<ASongFFmpeg> ASongFFmpeg::_instance = nullptr;
QMutex ASongFFmpeg::_mutex;
QMutex ASongFFmpeg::_mediaStatusMutex;

ASongFFmpeg::~ASongFFmpeg()
{
    if(nullptr != DataSink::getInstance())
    {
        delete DataSink::getInstance();
    }
}
// 全局访问点
ASongFFmpeg* ASongFFmpeg::getInstance()
{
    QMutexLocker locker(&_mutex);
    if(_instance.testAndSetOrdered(nullptr, nullptr))
    {
        //        qDebug() << "----";
        _instance.testAndSetOrdered(nullptr, new ASongFFmpeg);
    }
    return _instance;
}

void ASongFFmpeg::initPara()
{
    mediaMetaData.mediaType = 0;
    mediaMetaData.path = "";
    mediaMetaData.filename = "";
    mediaMetaData.format = "";
    mediaMetaData.durationSec = 0;
    mediaMetaData.durationMSec = 0;
    // 音频
    mediaMetaData.audio_meta_data.sample_rate = 0;
    mediaMetaData.audio_meta_data.channels = 0;
    mediaMetaData.audio_meta_data.channel_layout = 0;
    mediaMetaData.audio_meta_data.hasCover = false;
    // 视频
    mediaMetaData.video_meta_data.frame_rate = 0;
    mediaMetaData.video_meta_data.width = 0;
    mediaMetaData.video_meta_data.height = 0;
}

// 加载文件信息
int ASongFFmpeg::load(QString path)
{
    initPara();
    // 加锁，保证_instance同一时间只能被一个线程使用
    //        QMutexLocker locker(&_mutex);
    pFormatCtx = avformat_alloc_context();
    // 获取文件路径
    mediaMetaData.path = path;
    QStringList list = path.split("/");
    // 获取文件名
    mediaMetaData.filename = list[list.size() - 1];
    // 获取文件格式
    list = mediaMetaData.filename.split(".");
    mediaMetaData.format = list[list.size() - 1];
    // 获取总时长
    mediaMetaData.durationSec = pFormatCtx->duration / AV_TIME_BASE;
    mediaMetaData.durationMSec = mediaMetaData.durationSec * 1000;
    //    curMediaStatus = 1;
    std::string str = path.toStdString();
    // 打开文件
    int ret = avformat_open_input(&pFormatCtx, str.c_str(), nullptr, nullptr);
    if(ret < 0)
    {
        qDebug() << "Couldn't open input stream.";
        return -1;
    }
    // 读取文件信息
    ret = avformat_find_stream_info(pFormatCtx, nullptr);
    if(ret < 0)
    {
        qDebug() << "Couldn't find stream information.";
        return -1;
    }
    // 打印封装格式数据
    av_dump_format(pFormatCtx, 0, str.c_str(), 0);
    // 分出视频流和音频流
    //    AVCodecParameters *pCodecPara = nullptr;
    AVCodecContext *pACodecCtx = nullptr, *pVCodecCtx = nullptr;
    AVCodec *pACodec = nullptr, *pVCodec = nullptr;
    audioIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pACodec, 0);
    // 找到音频流
    if(audioIdx != AVERROR_STREAM_NOT_FOUND)
    {
        mediaMetaData.mediaType = 1;
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[audioIdx]->codecpar;
        //        pCodec = avcodec_find_decoder(pCodecPara->codec_id);
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
        // 获取音频流元数据
        mediaMetaData.audio_meta_data.sample_fmt = pACodecCtx->sample_fmt;
        mediaMetaData.audio_meta_data.sample_rate = pACodecCtx->sample_rate;
        mediaMetaData.audio_meta_data.channels = pACodecCtx->channels;
        mediaMetaData.audio_meta_data.channel_layout = pACodecCtx->channel_layout;
    }
    // 找视频流
    videoIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVCodec, 0);
    if(videoIdx != AVERROR_STREAM_NOT_FOUND)
    {
        //        qDebug() << "video";
        // 如果是视频流
        mediaMetaData.mediaType = 2;
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[videoIdx]->codecpar;
        // 设置streamIdx
        //        videoIdx = (int) i;
        // 获取解码器
        //        pCodec = avcodec_find_decoder(pCodecPara->codec_id);
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
        //            pCodecCtx->pkt_timebase=pFormatCtx->
        // 打开解码器
        ret = avcodec_open2(pVCodecCtx, pVCodec, nullptr);
        if(ret < 0)
        {
            qDebug() << "Couldn't open vidoe code.";
            return -1;
        }
        // 获取视频流元数据
        // 获取帧率，对于带封面的音频文件，av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate)为nan
        double tmpFrameRate = av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate);
        if(tmpFrameRate == tmpFrameRate)// 判断是否为nan
        {
            // 不为nan
            mediaMetaData.video_meta_data.frame_rate = ceil(tmpFrameRate);
        }
        else
        {
            // 为nan
            mediaMetaData.audio_meta_data.hasCover = true;
            mediaMetaData.video_meta_data.frame_rate = -1;
        }
        //        mediaMetaData.video_meta_data.frame_rate = ceil(av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate));
        mediaMetaData.video_meta_data.width = pCodecPara->width;
        mediaMetaData.video_meta_data.height = pCodecPara->height;
        mediaMetaData.video_meta_data.pix_fmt = pVCodecCtx->pix_fmt;
    }
    ASongAudio::getInstance()->setMetaData(pFormatCtx, pACodecCtx, audioIdx);
    if(mediaMetaData.mediaType == 2)
    {
        ASongVideo::getInstance()->setMetaData(pVCodecCtx, videoIdx,
                                               pFormatCtx->streams[videoIdx]->time_base, mediaMetaData.audio_meta_data.hasCover);
        SDLPaint::getInstance()->setMetaData(mediaMetaData.video_meta_data.width,
                                             mediaMetaData.video_meta_data.height,
                                             mediaMetaData.video_meta_data.frame_rate,
                                             mediaMetaData.video_meta_data.pix_fmt);
        //        qDebug() << mediaMetaData.video_meta_data.frame_rate;
    }
    pACodec = pVCodec = nullptr;
    //    qDebug() << "load finish";
    return 0;
}

//thread
void ASongFFmpeg::start(Priority pro)
{
    allowRead = true;
    //
    //    qDebug() << "vIdx" << videoIdx << ' ' << audioIdx;
    QThread::start(pro);
}

void ASongFFmpeg::run()
{
    while(allowRead)
    {
        if(DataSink::getInstance()->packetListSize(0) >= DataSink::maxPacketListLength
                && DataSink::getInstance()->packetListSize(1) >= DataSink::maxPacketListLength)
        {
            msleep(80);
        }
        else
        {
            AVPacket *packet = readFrame();
            if(nullptr == packet)
            {
                qDebug() << "Couldn't open file.";
                //                hasMedia = false;
                allowRead = false;
                break;
            }
            // 如果是音频
            if(packet->stream_index == audioIdx)
            {
                // 如果当前未被解码的packet过多，阻塞该读取线程
                DataSink::getInstance()->appendPacketList(0, packet);
                //                audioList.append(*packet);
            }
            else
            {
                if(packet->stream_index == videoIdx)
                {
                    //                    QMutexLocker locker(&videoListMutex);
                    // 如果当前未被解码的packet过多，阻塞该读取线程
                    DataSink::getInstance()->appendPacketList(1, packet);
                    //                    videoList.append(*packet);
                }
            }
        }
    }
    // 文件结束
    //    qDebug() << "thread quit";
}

AVPacket* ASongFFmpeg::readFrame()
{
    QMutexLocker locker(&_mutex);
    if(!pFormatCtx)
    {
        //        qDebug() << "pFormat";
        return nullptr;
    }
    AVPacket* packet = av_packet_alloc();
    int ret = av_read_frame(pFormatCtx, packet);
    if(ret < 0)
    {
        av_packet_free(&packet);
        packet = nullptr;
    }
    //    qDebug() << "read pakcet";
    return packet;
}

int ASongFFmpeg::getMediaType()
{
    return mediaMetaData.mediaType;
}

int ASongFFmpeg::getMediaStatus()
{
    //    QMutexLocker locker(&_mediaStatusMutex);
    //    qDebug() << curMediaStatus;
    return curMediaStatus;
}

int ASongFFmpeg::getCurPlaySec()
{
    return (int)ASongAudio::getInstance()->getAudioClock();
}

QString ASongFFmpeg::getFilepath()
{
    return mediaMetaData.path;
}

bool ASongFFmpeg::audioHasCover()
{
    return mediaMetaData.audio_meta_data.hasCover;
}

// 开始播放
int ASongFFmpeg::play(QString path, QWidget *_screenWidget)
{
    // 切换为播放态
    //    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 1;
    // 加载媒体文件信息
    load(path);
    // 初始化音频各参数及设备
    ASongAudio::getInstance()->initAndStartDevice(this);
    // 读取packet线程启动
    start();
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->start();
    //    qDebug() << mediaMetaData.mediaType;
    if(mediaMetaData.mediaType == 2)
    {
        //        qDebug() << "start";
        // SDL初始化
        painter = SDLPaint::getInstance();
        int ret = painter->init(_screenWidget);
        if(ret != 0)
        {
            qDebug() << "init sdl failed";
        }
        // 视频解码线程启动
        ASongVideo::getInstance()->start();
    }
    //    qDebug() << "start";
    return 0;
}

int ASongFFmpeg::_continue(bool isReplay)
{
    // 切换为播放态
    //    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 1;
    // 读取packet线程启动
    start();
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->start();
    // 带封面的音频在不是重新播放的情况下不启动视频解码线程和SDL
    if(mediaMetaData.mediaType == 2 && (!mediaMetaData.audio_meta_data.hasCover || mediaMetaData.audio_meta_data.hasCover && isReplay))
    {
        // 视频解码线程启动
        ASongVideo::getInstance()->start();
        // SDL重启
        painter->reStart();
    }
    //    qDebug() << "start";
    return 0;
}

int ASongFFmpeg::pause()
{
    //    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 2;
    // 结束各线程
    ASongAudio::getInstance()->pause();
    if(mediaMetaData.mediaType == 2)
    {
        ASongVideo::getInstance()->pause();
        SDLPaint::getInstance()->pause();
    }
    if(isRunning())
    {
        allowRead = false;
        wait();
    }
    return 0;
}

int ASongFFmpeg::stop()
{
    //    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 0;
    // 结束各线程
    ASongAudio::getInstance()->stop();
    if(mediaMetaData.mediaType == 2)
    {
        ASongVideo::getInstance()->stop();
        SDLPaint::getInstance()->stop();
    }
    if(isRunning())
    {
        allowRead = false;
        wait();
    }
    // 清空队列
    DataSink::getInstance()->clearList();
    if(nullptr == pFormatCtx)
    {
        return -1;
    }
    // 通过seek_frame移至开头
    int ret = av_seek_frame(pFormatCtx, -1, 0, AVSEEK_FLAG_ANY);
    if(ret < 0)
    {
        return -1;
    }
    //    qDebug()<<""
    return 0;
}

int ASongFFmpeg::seek(int posSec)
{
    // 先结束各线程，但是各上下文不关闭
    pause();
    // 清空队列
    DataSink::getInstance()->clearList();
    //    QMutexLocker locker(&_mutex);
    if(nullptr == pFormatCtx)
    {
        return -1;
    }
    int ret = av_seek_frame(pFormatCtx, -1,
                            (int64_t)posSec * AV_TIME_BASE,
                            AVSEEK_FLAG_ANY);
    if(ret < 0)
    {
        return -1;
    }
    _continue(false);
    return 0;
}
