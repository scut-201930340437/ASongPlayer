#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"

#include<QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    asongFFmpeg = new ASongFFmpeg(ui->screen_widget);
}

MainWindow::~MainWindow()
{
    ASongFFmpeg::getInstance()->stop();
    if(nullptr != ASongAudio::getInstance())
    {
        delete ASongAudio::getInstance();
    }
    if(nullptr != ASongVideo::getInstance())
    {
        delete ASongVideo::getInstance();
    }
    if(nullptr != ASongFFmpeg::getInstance())
    {
        delete ASongAudio::getInstance();
    }
    delete ui;
}

void MainWindow::on_play_button_clicked()
{
    // 定义文件对话框类
    QMutexLocker locker(&ASongFFmpeg::_mediaStatusMutex);
    switch (ASongFFmpeg::getInstance()->getMediaStatus())
    {
        case -1:
        {
            QString path = QFileDialog::getOpenFileName(this,
                           tr("选择音视频文件"),
                           tr("."),
                           tr(
                               "视频文件(*.mp4 *.flv *.avi);;音频文件(*.mp3);;所有文件(*.*)"));
            if(!path.isEmpty())
            {
                ASongFFmpeg::getInstance()->play(path, this->ui->play_widget);
            }
            else
            {
                qDebug() << "file empty";
            }
            break;
        }
        case 1:
            ASongFFmpeg::getInstance()->pause();
            break;
        case 2:
        {
            ASongFFmpeg::getInstance()->_continue(false);
            break;
        }
        case 0:
        {
            //            QString path = ASongFFmpeg::getInstance()->getFilepath();
            //            ASongFFmpeg::getInstance()->play(path, this->ui->play_widget);
            ASongFFmpeg::getInstance()->_continue(true);
            break;
        }
    }
}

void MainWindow::on_stop_button_clicked()
{
    QMutexLocker locker(&ASongFFmpeg::_mediaStatusMutex);
    if(ASongFFmpeg::getInstance()->getMediaStatus() > 0)
    {
        int ret = ASongFFmpeg::getInstance()->stop();
        if(ret == -1)
        {
            qDebug() << "stop failed";
            //            exit(0);
        }
    }
}

