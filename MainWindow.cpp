#include "MainWindow.h"
#include "ui_MainWindow.h"

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
    if(nullptr != painter)
    {
        delete painter;
    }
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
    //    if(asongFFmpeg->getMediaStatus() == 0)
    //    {
    // 定义文件对话框类
    switch (ASongFFmpeg::getInstance()->getMediaStatus())
    {
        case 0:
        {
            QString path = QFileDialog::getOpenFileName(this,
                           tr("选择音视频文件"),
                           tr("."),
                           tr(
                               "视频文件(*.mp4 *.flv *.avi);;音频文件(*.mp3);;所有文件(*.*)"));
            if(!path.isEmpty())
            {
                // 加载媒体文件信息
                ASongFFmpeg::getInstance()->load(path);
                // 初始化音频设备
                ASongAudio::getInstance()->initAndStartDevice(this);
                // 初始化重采样参数
                ASongAudio::getInstance()->initSwr();
                //        qDebug() << "-----";
                // 获取媒体类型，纯音频还是视频
                int mediaType = ASongFFmpeg::getInstance()->getMediaType();
                //        qDebug() << mediaType;
                ASongFFmpeg::getInstance()->play(mediaType);
                if(mediaType == 2)
                {
                    // SDL初始化
                    painter = SDLPaint::getInstance();
                    int ret = painter->init(this->ui->play_widget);
                    if(ret != 0)
                    {
                        qDebug() << "init sdl failed";
                    }
                }
                //        qDebug() << "btn_clicked";
            }
            else
            {
                qDebug() << "file empty";
            }
            break;
        }
        case 1:
            ASongFFmpeg::getInstance()->pause();
            //            qDebug() << "pause";
            break;
        case 2:
        {
            //            qDebug() << "continue";
            int mediaType = ASongFFmpeg::getInstance()->getMediaType();
            ASongFFmpeg::getInstance()->play(mediaType);
            painter->reStart();
            break;
        }
    }
    //    }
    //    else
    //    {
    //        if(asongFFmpeg->getMediaStatus() == 1)
    //        {
    //            asongFFmpeg->pause();
    //        }
    //        else
    //        {
    //            if(asongFFmpeg->getMediaStatus() == 2)
    //            {
    //                asongFFmpeg->play();
    //            }
    //        }
    //    }
}

