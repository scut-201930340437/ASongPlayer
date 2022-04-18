#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QDebug>
#include <QMediaMetaData>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player=new QMediaPlayer;

    // 绑定mediaplayer信号事件，如获取总时长
    connect(player, SIGNAL(metaDataAvailableChanged(bool)), this, SLOT(OnMetaDataAvailableChanged(bool)));
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onOriginalStateChanged(QMediaPlayer::State)));




    audioOutput=new QAudioOutput;
    videoWidget=new QVideoWidget;

    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoWidget);
    player->setParent(ui->play_widget);
    videoWidget->setParent(ui->play_widget);

//    videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);



//    this->setCentralWidget(ui->play_widget);

    // 给videoWidget加上黑色背景
    QPalette *palette=new QPalette;
    videoWidget->setPalette(*palette);
    videoWidget->setAutoFillBackground(true);

    delete palette;

    // 设置音量控制块初始参数
    ui->volume_ctrl->setMaximum(100);
    ui->volume_ctrl->setMinimum(0);
    ui->volume_ctrl->setSingleStep(2);
    ui->volume_ctrl->setValue(20);
    ui->volume_ctrl->setTracking(true);
    // 设置进度条初始参数

    ui->position_ctrl->setMinimum(0);
    ui->position_ctrl->setValue(0);
    ui->position_ctrl->setTracking(false);

    // 设置初始音量为20%
    audioOutput->setVolume(0.2);

    // 进度条定时器
    position_timer=new QTimer(this);
    connect(position_timer,&QTimer::timeout,[=](){
        this->ui->position_ctrl->setValue(int(player->position()/1000));
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

// 获取总时长的响应事件
void MainWindow::onDurationChanged(qint64 _duration)
{
    duration=_duration;
    ui->position_ctrl->setMaximum(duration/1000);
}

// 打开文件
void MainWindow::on_action_openFile_triggered()
{
    // 定义文件对话框类
    filename = QFileDialog::getOpenFileName(this,
                                                        tr("选择音视频文件"),
                                                        tr("."),
                                                        tr(
                                                            "音频文件(*.mp3);;视频文件(*.mp4 *.flv *.avi);;所有文件(*.*)"));
    if(!filename.isEmpty()){
        player->setSource(QUrl::fromLocalFile(filename));
        videoWidget->show();
        ui->position_ctrl->setValue(0);

        player->play();

        position_timer->start(1000);

        isPlaying=true;
        existFile=true;
        this->ui->play_button->setText("暂停");



    }else{
        qDebug("打开文件失败！");
    }


}

// 播放按钮
void MainWindow::on_play_button_clicked()
{
    if(!existFile){
        // 定义文件对话框类
        filename = QFileDialog::getOpenFileName(this,
                                                            tr("选择音视频文件"),
                                                            tr("."),
                                                            tr(
                                                                "音频文件(*.mp3);;视频文件(*.mp4 *.flv *.avi);;所有文件(*.*)"));
        if(!filename.isEmpty()){
            player->setSource(QUrl::fromLocalFile(filename));
            videoWidget->show();

            ui->position_ctrl->setValue(0);

//            player->setPlaybackRate(8.0);

            player->play();

            position_timer->start(1000);

            isPlaying=true;
            existFile=true;
            this->ui->play_button->setText("暂停");



        }else{
            qDebug("打开文件失败！");
        }

    }
    else{
        if(isPlaying){
            player->pause();
            isPlaying=false;
            this->ui->play_button->setText("播放");
        }else{
            player->play();
            isPlaying=true;
            this->ui->play_button->setText("暂停");
        }
    }


}

// 停止按钮
void MainWindow::on_stop_button_clicked()
{

        player->stop();
//        position_timer->stop();
        isPlaying=false;
        existFile=false;
        this->ui->play_button->setText("播放");

}

// 静音按钮
void MainWindow::on_mute_button_clicked()
{
    if(isMute){
        audioOutput->setMuted(false);
        audioOutput->setVolume((float)preVolume/100);
        isMute=false;
        this->ui->mute_button->setText("静音");
    }else{
        audioOutput->setMuted(true);
        isMute=true;
        this->ui->mute_button->setText("解除静音");
        preVolume=this->ui->volume_ctrl->value();
    }
}

// 音量控制块
void MainWindow::on_volume_ctrl_valueChanged(int value)
{
    audioOutput->setVolume((float)value/100);
    if(isMute&&value>0){
        audioOutput->setMuted(false);
        isMute=false;
        this->ui->mute_button->setText("静音");
    }
    if(value==0){
        audioOutput->setMuted(true);
        isMute=true;
        this->ui->mute_button->setText("解除静音");
    }
}

// 拖动进度条的响应事件
void MainWindow::on_position_ctrl_valueChanged(int value)
{
    if(positionCtrlPressed&&existFile){
        positionCtrlPressed=false;
        player->setPosition(duration*((float)value/this->ui->position_ctrl->maximum()));
        position_timer->start(1000);
    }
}


void MainWindow::on_position_ctrl_sliderPressed()
{
    positionCtrlPressed=true;
    position_timer->stop();
}

