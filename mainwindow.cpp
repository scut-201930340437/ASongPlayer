#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player=new QMediaPlayer;
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



}

MainWindow::~MainWindow()
{
    delete ui;
}


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
        player->play();
        isPlaying=true;
        this->ui->play_button->setText("暂停");
    }else{
        qDebug("打开文件失败！");
    }
}


void MainWindow::on_play_button_clicked()
{
    if(isPlaying){
        player->pause();
        isPlaying=false;
        this->ui->play_button->setText("播放");
    }else{
        isPlaying=true;
        player->play();
        this->ui->play_button->setText("暂停");
    }
}


void MainWindow::on_stop_button_clicked()
{

        player->stop();
        isPlaying=false;
        this->ui->play_button->setText("播放");

}

