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

    //播放模式初始为1 顺序
    playMode = 1;
    //初始为可切换播放模式
    playModeChangable = true;
    //音量
    this->volumeValueChangable = true;
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


void MainWindow::on_playmode_button_clicked()
{
    //不可切换则不做改变
    if(!playModeChangable) return;
    playMode = (playMode + 1) % 4;
    switch (playMode) {
    case 0:
        ui->playmode_button->setText("单次播放");
        break;
    case 1:
        ui->playmode_button->setText("顺序播放");
        break;
    case 2:
        ui->playmode_button->setText("随机播放");
        break;
    case 3:
        ui->playmode_button->setText("单曲循环");
        break;
    }
    //执行切换命令
    //toBeDone
}


void MainWindow::on_mute_button_clicked()
{
    if(!this->volumeValueChangable) return;
    if(ui->volume_ctrl->value()==0){
        ui->volume_ctrl->setOldValue();
        ui->mute_button->setText("静音");
    }
    else{
        ui->volume_ctrl->setZeroValue();
        ui->mute_button->setText("解除静音");
    }
}


void MainWindow::on_volume_ctrl_valueChanged(int value)
{
    if(value!=0){
        ui->mute_button->setText("静音");
    }
    if(value==0){
        ui->mute_button->setText("解除静音");
    }
}


void MainWindow::on_close_button_clicked()
{
    this->close();
}


void MainWindow::on_minimize_button_clicked()
{
    this->showMinimized();
}


void MainWindow::on_maximize_button_clicked()
{
    this->isMaximized() ? this->showNormal() : this->showMaximized();
}


void MainWindow::on_title_widget_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cmenu = new QMenu(ui->title_widget);

        QAction *action1 = new QAction(tr("关闭"), this);
        QAction *action2 = this->isMaximized() ? new QAction(tr("还原"), this) : new QAction(tr("最大化"), this);
        QAction *action3 = new QAction(tr("最小化"), this);
//        action1->setData(1);
        cmenu->addAction(action1);
        cmenu->addAction(action2);
        cmenu->addAction(action3);
        connect(action1, SIGNAL(triggered(bool)), this, SLOT(close()));
        connect(action2, SIGNAL(triggered(bool)), this, SLOT(on_maximize_button_clicked()));
        connect(action3, SIGNAL(triggered(bool)), this, SLOT(on_minimize_button_clicked()));
        cmenu->exec(QCursor::pos());
}


void MainWindow::on_MainWindow_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cmenu = new QMenu(ui->title_widget);

        QAction *action1 = new QAction(tr("打开文件"), this);
        cmenu->addAction(action1);
        connect(action1, SIGNAL(triggered(bool)), this, SLOT(on_play_button_clicked()));
        cmenu->exec(QCursor::pos());
}


void MainWindow::on_more_button_clicked()
{
//    QPoint q = QPoint(ui->more_button->x(),ui->more_button->y());
    QPoint q = QPoint(0, 0);
    on_MainWindow_customContextMenuRequested(q);
}

