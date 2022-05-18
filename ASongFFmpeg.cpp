#include "ASongFFmpeg.h"
#include "ASongAudio.h"
<<<<<<< HEAD
=======
#include "ASongAudioOutput.h"
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
#include "ASongVideo.h"
#include "VideoPreview.h"
#include "DataSink.h"

<<<<<<< HEAD


=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
Q_GLOBAL_STATIC(ASongFFmpeg, asongFFmpeg) // 采用qt实现的线程安全的单例模式

//QAtomicPointer<ASongFFmpeg> ASongFFmpeg::_instance = nullptr;
//QMutex ASongFFmpeg::_mutex;
QMutex ASongFFmpeg::_mediaStatusMutex;
<<<<<<< HEAD
QMutex ASongFFmpeg::_hasPacketMutex;
=======
//QMutex ASongFFmpeg::_hasPacketMutex;

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f


// 全局访问点
ASongFFmpeg* ASongFFmpeg::getInstance()
{
    //    QMutexLocker locker(&_mutex);
    //    if(_instance.testAndSetOrdered(nullptr, nullptr))
    //    {
    //        //        qDebug() << "----";
    //        _instance.testAndSetOrdered(nullptr, new ASongFFmpeg);
    //    }
    return asongFFmpeg;
}

<<<<<<< HEAD
MediaMetaData* ASongFFmpeg::openMediaInfo(QString path, AVFormatContext* pFmtCtx){
=======
MediaMetaData* ASongFFmpeg::openMediaInfo(QString path, AVFormatContext* pFmtCtx)
{
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    bool ctx_not_null = pFmtCtx;
    AVFormatContext *ctx = NULL;
    MediaMetaData* ret = NULL;
    //传入非空ctx
<<<<<<< HEAD
    if(ctx_not_null){
        ctx = pFmtCtx;
    }
    else if(!(ctx = avformat_alloc_context())){
        qDebug()<<"findMediaInfo: Could not allocate context";
        return ret;
    }
    //打开
    if(avformat_open_input(&ctx, path.toStdString().c_str(), NULL, NULL) < 0){
        qDebug()<<"findMediaInfo: open input error";
        return ret;
    }
    //获取媒体信息
    if(avformat_find_stream_info(ctx, NULL) < 0){
        qDebug()<<"findMediaInfo: find media info error";
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
<<<<<<< HEAD
    while((tag = av_dict_get(ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))){
        QString keyString = tag->key;
        if(keyString == "title"){
            ret->title = QString::fromUtf8(tag->value);
        }
        else if(keyString == "artist"){
            ret->artist = QString::fromUtf8(tag->value);
        }
        else if(keyString == "album"){
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            ret->album = QString::fromUtf8(tag->value);
        }
    }
    //获取所有streams信息
<<<<<<< HEAD
    ret->aMetaDatas = NULL; ret->vMetaDatas = NULL;
    ret->nb_astreams = ret->nb_vstreams = 0;
    for(int i = 0; i < ctx->nb_streams; i++){
        AVStream *as = ctx->streams[i];
        if(AVMEDIA_TYPE_AUDIO == as->codecpar->codec_type){
            ret->nb_astreams++;
        }
        else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type){
            ret->nb_vstreams++;
        }
    }
    if(ret->nb_astreams != 0){
        ret->aMetaDatas = new AudioMetaData[ret->nb_astreams];
    }
    if(ret->nb_vstreams != 0){
        ret->vMetaDatas = new VideoMetaData[ret->nb_vstreams];
    }
    for(int i = 0, ai = 0, vi = 0; i < ctx->nb_streams; i++){
        AVStream *as = ctx->streams[i];
        if(AVMEDIA_TYPE_AUDIO == as->codecpar->codec_type){
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            AudioMetaData& amd = ret->aMetaDatas[ai];
            amd.idx = i;
            amd.bit_rate = as->codecpar->bit_rate;
            amd.sample_rate = as->codecpar->sample_rate;
            amd.nb_channels = as->codecpar->channels;
            amd.codec_id = as->codecpar->codec_id;
            ai++;
        }
<<<<<<< HEAD
        else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type){
=======
        else if(AVMEDIA_TYPE_VIDEO == as->codecpar->codec_type)
        {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            VideoMetaData& vmd = ret->vMetaDatas[vi];
            vmd.idx = i;
            vmd.width = as->codecpar->width;
            vmd.height = as->codecpar->height;
            vmd.codec_id = as->codecpar->codec_id;
<<<<<<< HEAD
            if(as->disposition & AV_DISPOSITION_ATTACHED_PIC){//有封面图
=======
            if(as->disposition & AV_DISPOSITION_ATTACHED_PIC) //有封面图
            {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                vmd.cover = new QImage;
                AVPacket pkt = as->attached_pic;
                *vmd.cover = QImage::fromData((uchar*)pkt.data, pkt.size);
                vmd.bit_rate = -1;
                vmd.frame_rate = -1;
            }
<<<<<<< HEAD
            else{
=======
            else
            {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                vmd.cover = nullptr;
                vmd.bit_rate = as->codecpar->bit_rate;
                vmd.frame_rate = av_q2d(as->avg_frame_rate);
            }
            vi++;
        }
    }
<<<<<<< HEAD
    if(!ctx_not_null){
=======
    if(!ctx_not_null)
    {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        avformat_close_input(&ctx);
    }
    return ret;
}

<<<<<<< HEAD
void ASongFFmpeg::closeMediaInfo(MediaMetaData *mmd){
    if(mmd->nb_astreams != 0){
        delete []mmd->aMetaDatas;
    }
    if(mmd->nb_vstreams != 0){
        for(int i = 0; i<mmd->nb_vstreams; i++){
            VideoMetaData& vmd = mmd->vMetaDatas[i];
            if(vmd.cover != nullptr){
=======
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
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
    //initPara();
    // 加锁，保证_instance同一时间只能被一个线程使用
    //        QMutexLocker locker(&_mutex);
    pFormatCtx = avformat_alloc_context();
    mediaMetaData = openMediaInfo(path, pFormatCtx);
<<<<<<< HEAD
    if(mediaMetaData == nullptr){
        return -1;
    }
=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    int ret;
    // 分出视频流和音频流
    //    AVCodecParameters *pCodecPara = nullptr;
    AVCodecContext *pACodecCtx = nullptr, *pVCodecCtx = nullptr;
    AVCodec *pACodec = nullptr, *pVCodec = nullptr;
    audioIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pACodec, 0);
    // 找到音频流
    if(audioIdx != AVERROR_STREAM_NOT_FOUND)
    {
        //mediaMetaData.mediaType = 1;
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[audioIdx]->codecpar;
<<<<<<< HEAD
        for(int i = 0; i<mediaMetaData->nb_astreams; i++){
            if(audioIdx == mediaMetaData->aMetaDatas[i].idx){
=======
        for(int i = 0; i < mediaMetaData->nb_astreams; i++)
        {
            if(audioIdx == mediaMetaData->aMetaDatas[i].idx)
            {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                audioIdx = i;
                break;
            }
        }
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
    }
    // 找视频流
    videoIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVCodec, 0);
    if(videoIdx != AVERROR_STREAM_NOT_FOUND)
    {
<<<<<<< HEAD

=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        //        qDebug() << "video";
        // 如果是视频流
        //mediaMetaData.mediaType = 2;
        // 获取解码器
        AVCodecParameters *pCodecPara = pFormatCtx->streams[videoIdx]->codecpar;
<<<<<<< HEAD
        for(int i = 0; i<mediaMetaData->nb_vstreams; i++){
            if(videoIdx == mediaMetaData->vMetaDatas[i].idx){
=======
        for(int i = 0; i < mediaMetaData->nb_vstreams; i++)
        {
            if(videoIdx == mediaMetaData->vMetaDatas[i].idx)
            {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                videoIdx = i;
                break;
            }
        }
        hasCover = (mediaMetaData->vMetaDatas[videoIdx].cover != nullptr);
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
<<<<<<< HEAD
//        double tmpFrameRate = av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate);
//        if(tmpFrameRate == tmpFrameRate)// 判断是否为nan
//        {
//            // 不为nan
//            mediaMetaData.video_meta_data.frame_rate = ceil(tmpFrameRate);
//        }
//        else
//        {
//            // 为nan
//            mediaMetaData.audio_meta_data.hasCover = true;
//            mediaMetaData.video_meta_data.frame_rate = -1;
//        }
//        //        mediaMetaData.video_meta_data.frame_rate = ceil(av_q2d(pFormatCtx->streams[videoIdx]->avg_frame_rate));
//        mediaMetaData.video_meta_data.width = pCodecPara->width;
//        mediaMetaData.video_meta_data.height = pCodecPara->height;
//        mediaMetaData.video_meta_data.pix_fmt = pVCodecCtx->pix_fmt;
    }
    ASongAudio::getInstance()->setMetaData(pFormatCtx, pACodecCtx, mediaMetaData->aMetaDatas[audioIdx].idx);
    if(videoIdx >= 0)
    {   int idx = mediaMetaData->vMetaDatas[videoIdx].idx;
=======
    }
    ASongAudio::getInstance()->setMetaData(pFormatCtx, pACodecCtx, mediaMetaData->aMetaDatas[audioIdx].idx);
    if(videoIdx >= 0)
    {
        int idx = mediaMetaData->vMetaDatas[videoIdx].idx;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        ASongVideo::getInstance()->setMetaData(pVCodecCtx, idx,
                                               pFormatCtx->streams[idx]->time_base, hasCover);
        SDLPaint::getInstance()->setMetaData(mediaMetaData->vMetaDatas[videoIdx].width,
                                             mediaMetaData->vMetaDatas[videoIdx].height,
                                             mediaMetaData->vMetaDatas[videoIdx].frame_rate,
                                             pVCodecCtx->pix_fmt);
    }
    pACodec = pVCodec = nullptr;
<<<<<<< HEAD
    hasMorePacket = true;
=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    return 0;
}

AVPacket* ASongFFmpeg::readFrame()
{
    // QMutexLocker locker(&_mutex);
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
    //    qDebug() << "read pakcet";
    return packet;
}

<<<<<<< HEAD
bool ASongFFmpeg::hasPakcet()
{
    if(hasMorePacket)
    {
        return true;
    }
    else
    {
        return false;
    }
}

=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
int ASongFFmpeg::getMediaStatus()
{
    return curMediaStatus;
}

int ASongFFmpeg::getDuration()
{
    if(mediaMetaData != nullptr)
<<<<<<< HEAD
        return mediaMetaData->durationSec;
    return 0;
}

int ASongFFmpeg::getCurPlaySec()
{
    return int(ASongAudio::getInstance()->getAudioClock());
=======
    {
        return mediaMetaData->durationSec;
    }
    return 0;
}

int64_t ASongFFmpeg::getCurPlaySec()
{
    return int64_t(ASongAudio::getInstance()->getAudioClock());
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

QString ASongFFmpeg::getFilepath()
{
    if(mediaMetaData != nullptr)
<<<<<<< HEAD
        return mediaMetaData->path;
    return "";
}

=======
    {
        return mediaMetaData->path;
    }
    return "";
}

float ASongFFmpeg::getSpeed()
{
    return ASongAudioOutput::getInstance()->getSpeed();
}

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
bool ASongFFmpeg::audioHasCover()
{
    return hasCover;
}

<<<<<<< HEAD
// 开始播放
int ASongFFmpeg::play(QObject *par, QString path, void *winID)
{
    // 切换为播放态
    curMediaStatus = 1;
    // 加载媒体文件信息
    load(path);
    // 初始化音频各参数及设备
    ASongAudio::getInstance()->initAndStartDevice(par);
    // 读取packet线程启动
    start();
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->start();
    if(videoIdx >= 0)
    {
        // 视频解码线程启动
        ASongVideo::getInstance()->start();
        // SDL初始化并开启sdl渲染定时器
        painter = SDLPaint::getInstance();
        int ret = painter->init(winID);
        if(ret != 0)
        {
            qDebug() << "init sdl failed";
        }
        //启动视频预览线程
        VideoPreview::getInstance()->start(path, this->mediaMetaData->vMetaDatas[videoIdx].idx);
    }
=======

// 开始播放
int ASongFFmpeg::play(QObject *par, QString _path, void *winID)
{
    // 切换为播放态
    curMediaStatus = 1;
    path = _path;
    ASongAudioOutput::getInstance()->createMediaDevice(par);
    // 读取packet线程启动
    start();
    SDLPaint::getInstance()->init(winID);
    //    SDLPaint::getInstance()->createTimer();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //    qDebug() << "play";
    return 0;
}

<<<<<<< HEAD
//int ASongFFmpeg::_continue(bool isReplay)
//{
//    // 切换为播放态
//    curMediaStatus = 1;
//    // 读取packet线程启动
//    start();
//    // 音频解码线程和播放线程启动
//    ASongAudio::getInstance()->start();
//    // 带封面的音频在不是重新播放的情况下不启动视频解码线程和SDL
//    if(mediaMetaData.mediaType == 2 && (!mediaMetaData.audio_meta_data.hasCover || mediaMetaData.audio_meta_data.hasCover && isReplay))
//    {
//        // 视频解码线程启动
//        ASongVideo::getInstance()->start();
//        // SDL重启
//        painter->reStart();
//    }
//    //    qDebug() << "start";
//    return 0;
//}

// thread
void ASongFFmpeg::start(Priority pri)
{
    allowRead = true;
=======
// thread
void ASongFFmpeg::start(Priority pri)
{
    stopReq = false;
    pauseReq = false;
    //    needPaused = false;
    pauseFlag = false;
    stopFlag = false;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    QThread::start(pri);
}

int ASongFFmpeg::stop()
{
    if(nullptr == pFormatCtx)
    {
        return -1;
    }
    curMediaStatus = 0;
<<<<<<< HEAD
    // 先结束音频解码和音频播放线程
    ASongAudio::getInstance()->stop();
    if(videoIdx >= 0)
    {
        ASongVideo::getInstance()->stop();
        SDLPaint::getInstance()->stop();
=======
    // 先结束音频播放线程和关闭音频设备
    ASongAudioOutput::getInstance()->stop();
    // 再结束音频解码
    ASongAudio::getInstance()->stop();
    if(videoIdx >= 0)
    {
        SDLPaint::getInstance()->stop();
        ASongVideo::getInstance()->stop();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        VideoPreview::getInstance()->stop();
    }
    if(QThread::isRunning())
    {
<<<<<<< HEAD
        allowRead = false;
        needPaused = false;
=======
        //        allowRead = false;
        stopReq = true;
        //        needPaused = false;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        //        pauseFlag = false;
        pauseCond.wakeAll();
        QThread::quit();
        QThread::wait();
    }
    // 清空队列
    DataSink::getInstance()->clearList();
    // 关闭文件流上下文
    if(nullptr != pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = nullptr;
    }
<<<<<<< HEAD
    if(nullptr != mediaMetaData){
        closeMediaInfo(mediaMetaData);
        mediaMetaData = nullptr;
    }
    // 通过seek_frame移至开头
    //    int ret = av_seek_frame(pFormatCtx, -1, 0, AVSEEK_FLAG_ANY);
    //    if(ret < 0)
    //    {
    //        return -1;
    //    }
=======
    if(nullptr != mediaMetaData)
    {
        closeMediaInfo(mediaMetaData);
        mediaMetaData = nullptr;
    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    return 0;
}

int ASongFFmpeg::pause()
{
    curMediaStatus = 2;
<<<<<<< HEAD
    // 阻塞各线程
    ASongAudio::getInstance()->pause();
    if(videoIdx >= 0)
    {
        ASongVideo::getInstance()->pause();
        SDLPaint::getInstance()->pause();
    }
    if(QThread::isRunning())
    {
        QMutexLocker locker(&_pauseMutex);
        needPaused = true;
        pauseCond.wait(&_pauseMutex);
    }
    return 0;
}

=======
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

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
int ASongFFmpeg::resume()
{
    // 切换为播放态
    curMediaStatus = 1;
<<<<<<< HEAD
    // 读取packet线程重启
    if(QThread::isRunning())
    {
        needPaused = false;
        //        pauseFlag = false;
        pauseCond.wakeAll();
    }
    else
    {
        if(QThread::isFinished())
        {
            start();
        }
    }
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->resume();
    // 带封面的音频在不是重新播放的情况下不启动视频解码线程和SDL
    if(videoIdx >= 0 && !hasCover)
    {
        // 视频解码线程启动
        ASongVideo::getInstance()->resume();
        // SDL重启
        painter->resume();
=======
    // 音频播放线程恢复
    ASongAudioOutput::getInstance()->resume();
    // 带封面的音频在不是重新播放的情况下不启动视频解码线程和SDL
    if(videoIdx >= 0)
    {
        // 重设video frameTimer
        ASongVideo::getInstance()->resume();
        SDLPaint::getInstance()->resume();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    }
    return 0;
}

<<<<<<< HEAD
void ASongFFmpeg::run()
{
    //    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    //    qDebug() << "unref start";
    //    qDebug() << "";
    while(allowRead)
    {
        if(DataSink::getInstance()->packetListSize(0) >= DataSink::maxPacketListLength
                || DataSink::getInstance()->packetListSize(1) >= DataSink::maxPacketListLength)
        {
            msleep(20);
=======
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
    // 加载媒体文件信息,打开解码器
    load(path);
    // 初始化音频各参数及设备
    ASongAudio::getInstance()->initParaAndSwr();
    // 音频解码线程和播放线程启动
    ASongAudio::getInstance()->start();
    ASongAudioOutput::getInstance()->start();
    if(videoIdx >= 0)
    {
        // 视频解码线程启动
        ASongVideo::getInstance()->start();
        // 预览线程启动
        VideoPreview::getInstance()->start(path, mediaMetaData->vMetaDatas[videoIdx].idx);
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
        if(seekReq)
        {
            seekReq = 0;
            handleSeek();
        }
        if(DataSink::getInstance()->packetListSize(0) >= DataSink::maxAPacketListLength
                || DataSink::getInstance()->packetListSize(1) >= DataSink::maxVPacketListLength)
        {
            msleep(25);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
        else
        {
            AVPacket *packet = readFrame();
            if(nullptr == packet)
            {
<<<<<<< HEAD
                QMutexLocker locker(&_hasPacketMutex);
                //                qDebug() << "Couldn't open file.";
                hasMorePacket = false;
                locker.unlock();
                // 唤醒可能阻塞的解码线程
                DataSink::getInstance()->appendPacketList(0, nullptr);
                DataSink::getInstance()->appendPacketList(1, nullptr);
                allowRead = false;
=======
                stopReq = true;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                break;
            }
            // 如果是音频
            if(packet->stream_index == mediaMetaData->aMetaDatas[audioIdx].idx)
            {
                //                qDebug() << 0;
                DataSink::getInstance()->appendPacketList(0, packet);
            }
            else
            {
                if(packet->stream_index == mediaMetaData->vMetaDatas[videoIdx].idx)
                {
                    //                    qDebug() << 1;
                    DataSink::getInstance()->appendPacketList(1, packet);
                }
            }
        }
<<<<<<< HEAD
        if(needPaused)
        {
            //            av_read_pause()
            QMutexLocker locker(&_pauseMutex);
            //            pauseFlag = true;
            // 唤醒主线程，此时主线程知道音频解码线程阻塞
            pauseCond.wakeAll();
            // 音频解码线程阻塞
            pauseCond.wait(&_pauseMutex);
        }
    }
    allowRead = false;
    needPaused = false;
=======
    }
    QMutexLocker locker(&stopMutex);
    //                qDebug() << "Couldn't open file.";
    stopFlag = true;
    // 唤醒可能阻塞的解码线程
    locker.unlock();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 解复用结束
    //    qDebug() << "unref thread quit";
    //    qDebug() << "";
}

<<<<<<< HEAD
//bool ASongFFmpeg::isPaused()
//{
//    if(pauseFlag)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

int ASongFFmpeg::seek(int posSec)
{
    if(nullptr == pFormatCtx || curMediaStatus <= 0)
    {
        return -1;
    }
    //    qDebug() << posSec;
    // 如果当前是播放态，先阻塞各线程，但是各上下文不关闭
    if(curMediaStatus == 1)
    {
        pause();
    }
    // 清空队列
=======
void ASongFFmpeg::handleSeek()
{
    // 停止音频播放和视频渲染
    pause();
    // 阻塞解码线程
    ASongAudio::getInstance()->pauseThread();
    ASongVideo::getInstance()->pauseThread();
    // 清理队列
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    DataSink::getInstance()->clearList();
    // 解码线程可能已经退出，需要设置相应标志使它们能够重启
    //    ASongAudio::getInstance()->setNeededAudioCode();
    int ret = 0;
    // seek I frame
    if(videoIdx >= 0 && !hasCover)
    {
<<<<<<< HEAD
        ASongVideo::getInstance()->setNeededVideoCode();
        // 清理解码器缓存，否则可能出现花屏的现象(仅对视频)
        ASongVideo::getInstance()->flushBeforeSeek();
    }
    //    else
    //    {
    //        ret = av_seek_frame(pFormatCtx, -1,
    //                            (int64_t)posSec * AV_TIME_BASE,
    //                            AVSEEK_FLAG_ANY);
    //    }
    ret = av_seek_frame(pFormatCtx, -1,
                        (int64_t)posSec * AV_TIME_BASE,
                        AVSEEK_FLAG_BACKWARD);
    if(ret < 0)
    {
        return -1;
    }
    // 重启各线程
    resume();
    return 0;
}

=======
        // 清理解码器缓存，否则可能出现花屏的现象(仅对视频)
        ASongVideo::getInstance()->flushBeforeSeek();
    }
    int64_t seekTarget = seekPos;
    int64_t seekMin    = seekRel > 0 ? seekTarget - seekRel + 2 : INT64_MIN;
    int64_t seekMax    = seekRel < 0 ? seekTarget - seekRel - 2 : INT64_MAX;
    ret = avformat_seek_file(pFormatCtx, -1, seekMin, seekTarget, seekMax, 0);
    if(ret < 0)
    {
        qDebug() << "seek failed";
    }
    // 重启
    ASongAudio::getInstance()->resumeThread();
    if(videoIdx >= 0)
    {
        ASongVideo::getInstance()->resumeThread();
    }
    resume();
}

int ASongFFmpeg::seek(int64_t posSec)
{
    //    qDebug() << posSec;
    if(nullptr == pFormatCtx || curMediaStatus <= 0)
    {
        return -1;
    }
    // 停止解复用线程
    pauseThread();
    // 设置seek请求
    seekReq = 1;
    seekPos = posSec * AV_TIME_BASE;
    seekRel = seekPos - getCurPlaySec() * AV_TIME_BASE;
    // 唤醒解复用线程
    resumeThread();
    return 0;
}

// 逐帧
void ASongFFmpeg::step_to_next_frame()
{
    // 暂停音频播放线程
    if(!ASongAudioOutput::getInstance()->pauseFlag)
    {
        ASongAudioOutput::getInstance()->pause();
    }
    // 停止渲染定时器
    SDLPaint::getInstance()->stopTimer();
    // 设置非暂停态
    SDLPaint::getInstance()->resume();
    // 播放一帧音频
    ASongAudioOutput::getInstance()->process();
    // 渲染一帧
    SDLPaint::getInstance()->getFrameYUV();
    // 设置暂停态
    SDLPaint::getInstance()->pause();
    // 重启定时器使sdl不断渲染上一帧
    SDLPaint::getInstance()->restartTimer();
}
// 设置速率
void ASongFFmpeg::setSpeed(float _speed)
{
    ASongAudioOutput::getInstance()->setSpeed(_speed);
}

>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
void ASongFFmpeg::hideCursor()
{
    SDL_ShowCursor(false);
}

void ASongFFmpeg::showCursor()
{
    SDL_ShowCursor(true);
}
