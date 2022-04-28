#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    asongFFmpeg = new ASongFFmpeg(ui->screen_widget);
}

MainWindow::~MainWindow()
{
    delete asongFFmpeg;
    asongFFmpeg = nullptr;
    delete ui;
}


void MainWindow::on_play_btn_clicked()
{
    if(asongFFmpeg->getMediaStatus() == 0)
    {
        // 定义文件对话框类
        QString filename = QFileDialog::getOpenFileName(this,
                           tr("选择音视频文件"),
                           tr("."),
                           tr(
                               "视频文件(*.mp4 *.flv *.avi);;音频文件(*.mp3);;所有文件(*.*)"));
        if(!filename.isEmpty())
        {
            asongFFmpeg->load(filename);
        }
    }
    else
    {
        if(asongFFmpeg->getMediaStatus() == 1)
        {
            asongFFmpeg->pause();
        }
        else
        {
            if(asongFFmpeg->getMediaStatus() == 2)
            {
                asongFFmpeg->play();
            }
        }
    }
}

