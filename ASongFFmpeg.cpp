#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"

#include "SDLPaint.h"

#include "DataSink.h"

#include <QDebug>

QAtomicPointer<ASongFFmpeg> ASongFFmpeg::_instance = nullptr;
QMutex ASongFFmpeg::_mutex;

ASongFFmpeg::~ASongFFmpeg()
{
    avformat_close_input(&pFormatCtx);
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

// 加载文件信息
int ASongFFmpeg::load(QString path)
{
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
        //        if(mediaMetaData.findAudio)
        //        {
        //            continue;
        //        }
        //        else
        //        {
        //            mediaMetaData.findAudio = true;
        //        }
        //        if(mediaMetaData.mediaType == 0)
        //        {
        mediaMetaData.mediaType = 1;
        //        }
        // 设置streamIdx
        //        audioIdx = (int)i;
        //            ASongAudio::getInstance()->setAudioIdx(audioIdx);
        //            // 设置时基
        //            ASongAudio::getInstance()->setTimeBase(pFormatCtx->streams[i]->time_base);
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
        // 获取帧率
        mediaMetaData.video_meta_data.frame_rate = ceil(av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate));
        mediaMetaData.video_meta_data.width = pCodecPara->width;
        mediaMetaData.video_meta_data.height = pCodecPara->height;
        mediaMetaData.video_meta_data.pix_fmt = pVCodecCtx->pix_fmt;
    }
    //    for (size_t i = 0; i < pFormatCtx->nb_streams; ++i)
    //    {
    //        // 如果是音频流
    //        AVCodecParameters *pCodecPara = pFormatCtx->streams[i]->codecpar;
    //        if(pCodecPara->codec_type == AVMEDIA_TYPE_AUDIO)
    //        {
    //            qDebug() << "audio";
    //        }
    //        else
    //        {
    //            if(pCodecPara->codec_type == AVMEDIA_TYPE_VIDEO)
    //            {
    //            }
    //        }
    //    }
    ASongAudio::getInstance()->setMetaData(pFormatCtx, pACodecCtx, audioIdx);
    if(mediaMetaData.mediaType == 2)
    {
        ASongVideo::getInstance()->setMetaData(pVCodecCtx, videoIdx,
                                               pFormatCtx->streams[videoIdx]->time_base);
        SDLPaint::getInstance()->setMetaData(mediaMetaData.video_meta_data.width,
                                             mediaMetaData.video_meta_data.height,
                                             mediaMetaData.video_meta_data.frame_rate,
                                             mediaMetaData.video_meta_data.pix_fmt);
    }
    pACodec = pVCodec = nullptr;
    //    qDebug() << "load finish";
    return 0;
}

// 启动各线程
int ASongFFmpeg::play(int mediaType)
{
    start();
    ASongAudio::getInstance()->start();
    if(mediaType == 2)
    {
        ASongVideo::getInstance()->start();
    }
    //    qDebug() << "start";
    return 0;
}

//thread
void ASongFFmpeg::start(Priority pro)
{
    allowRead = true;
    // 切换为播放态
    curMediaStatus = 1;
    //
    //    qDebug() << "vIdx" << videoIdx << ' ' << audioIdx;
    QThread::start(pro);
}

void ASongFFmpeg::run()
{
    bool hasMedia = true;
    while(allowRead)
    {
        if(DataSink::getInstance()->packetListSize(0) >= DataSink::maxPacketListLength
                && DataSink::getInstance()->packetListSize(1) >= DataSink::maxPacketListLength)
        {
            msleep(20);
        }
        else
        {
            AVPacket *packet = readFrame();
            if(!packet)
            {
                qDebug() << "Couldn't open file.";
                hasMedia = false;
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
    if(hasMedia)
    {
        curMediaStatus = 3;
    }
    // 没有文件
    else
    {
        curMediaStatus = 0;
    }
}

AVPacket* ASongFFmpeg::readFrame()
{
    QMutexLocker locker(&_mutex);
    if(!pFormatCtx)
    {
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

void ASongFFmpeg::setMediaStatus(int status)
{
    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = status;
}

int ASongFFmpeg::getMediaType()
{
    return mediaMetaData.mediaType;
}

int ASongFFmpeg::getMediaStatus()
{
    QMutexLocker locker(&_mediaStatusMutex);
    return curMediaStatus;
}

int ASongFFmpeg::pause()
{
    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 2;
    allowRead = false;
    ASongAudio::getInstance()->pause();
    ASongVideo::getInstance()->pause();
    return 0;
}


int ASongFFmpeg::stop()
{
    QMutexLocker locker(&_mediaStatusMutex);
    curMediaStatus = 3;
    allowRead = false;
    return 0;
}





//AVFormatContext* ASongFFmpeg::getFormatCtx()
//{
//    return pFormatCtx;
//}

//int ASongFFmpeg::getSampleRate()
//{
//    return mediaMetaData.audio_meta_data.sample_rate;
//}

//int ASongFFmpeg::getChannels()
//{
//    return mediaMetaData.audio_meta_data.channels;
//}

//int ASongFFmpeg::getSrcWidth()
//{
//    return mediaMetaData.video_meta_data.width;
//}

//int ASongFFmpeg::getSrcHeight()
//{
//    return mediaMetaData.video_meta_data.height;
//}

//enum AVPixelFormat ASongFFmpeg::getPixFmt()
//{
//    return mediaMetaData.video_meta_data.pix_fmt;
//}


//int ASongFFmpeg::getFrameRate()
//{
//    return mediaMetaData.video_meta_data.frame_rate;
//}

//void ASongFFmpeg::setMediaStatus(int status)
//{
//    curMediaStatus=status;
//}
