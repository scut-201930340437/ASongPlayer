#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"
#include <QDebug>

#include<QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    asongFFmpeg = new ASongFFmpeg(ui->screen_widget);
    //播放模式初始为1 顺序
    playMode = 1;
        //定时器
        myTimer = new QTimer();
        myTimer->setInterval(1000); //1秒
        connect(myTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
        myTimer->start();

    connect(this->ui->play_table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onPlayTableCellDoubleClicked(int, int)));
    //启用鼠标拖拽放下操做
    setAcceptDrops(true);
    //上一次进程关闭时保存的文件路径
    readFilePath();
}

MainWindow::~MainWindow()
{
    // 结束线程
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

QString MainWindow::getTimeString(int position){
    QString ph = QString::number(position / 3600);
    if(ph.length() == 1)
    {
        ph = "0" + ph;
    }
    position %= 3600;
    QString pm = QString::number(position / 60);
    if(pm.length() == 1)
    {
        pm = "0" + pm;
    }
    QString ps = QString::number(position % 60);
    if(ps.length() == 1)
    {
        ps = "0" + ps;
    }
    QString posiStr = ph + ":" + pm + ":" + ps ;
    return posiStr;
}

void MainWindow::on_play_button_clicked()
{
    // 定义文件对话框类
    QMutexLocker locker(&ASongFFmpeg::_mediaStatusMutex);
    switch (ASongFFmpeg::getInstance()->getMediaStatus())
    {
        case -1:
        {
            if(filePath!="")
            {
                ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
                ui->play_button->setText("暂停");
                break;
            }
            else
            {
                openFile();
                ui->play_button->setText("暂停");
                break;
            }
        }
        case 1:
            ASongFFmpeg::getInstance()->pause();
            ui->play_button->setText("播放");
            break;
        case 2:
        {
            ASongFFmpeg::getInstance()->_continue(false);
            ui->play_button->setText("暂停");
            break;
        }
        case 0:
        {
            //            QString path = ASongFFmpeg::getInstance()->getFilepath();
            //            ASongFFmpeg::getInstance()->play(path, this->ui->play_widget);
            ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
            ui->play_button->setText("播放");
            break;
        }
    }
}

void MainWindow::on_stop_button_clicked()
{
    this->ui->play_button->setText("播放");
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
    playMode = (playMode + 1) % 4;
    switch (playMode)
    {
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
    if(ui->volume_ctrl->value() == 0)
    {
        ui->volume_ctrl->setOldValue();
        ui->mute_button->setText("静音");
    }
    else
    {
        ui->volume_ctrl->setZeroValue();
        ui->mute_button->setText("解除静音");
    }
    ASongAudio::getInstance()->setVolume(ui->volume_ctrl->value());
}


void MainWindow::on_volume_ctrl_valueChanged(int value)
{
    if(value != 0)
    {
        ui->mute_button->setText("静音");
    }
    if(value == 0)
    {
        ui->mute_button->setText("解除静音");
    }
    ASongAudio::getInstance()->setVolume(ui->volume_ctrl->value());
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




void MainWindow::on_more_button_clicked()
{
    //    QPoint q = QPoint(ui->more_button->x(),ui->more_button->y());
    QPoint q = QPoint(0, 0);
    on_play_widget_customContextMenuRequested(q);
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this,
                   tr("选择音视频文件"),
                   tr("."),
                   tr(
                       "视频文件(*.mp4 *.flv *.avi);;音频文件(*.mp3);;所有文件(*.*)"));
    if(!path.isEmpty())
    {
        filePath = path;
        //        QMutexLocker locker(&ASongFFmpeg::_mediaStatusMutex);
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, path, this->ui->play_widget);
        //        locker.unlock();
        setListFromFilePath();
        saveFilePath();
    }
    else
    {
        qDebug() << "file empty";
    }
}

void MainWindow::setListFromFilePath()
{
    //设置播放列表
    QString path = filePath.section('/', 0, -2);
    QDir Dir(path);
    if(!Dir.exists())
    {
        qDebug("路径出错");
    }
    else
    {
        QFileInfoList _list = Dir.entryInfoList(QDir::Files);
        QFileInfoList neededList;
        qint16 count_row = 0;
        foreach (QFileInfo file, _list)                  			//遍历只加载音视频文件到文件列表
        {
            if(ui->play_table->isNeededFile(file))          //判断进行再次确认是可播放文件
            {
                if(file.absoluteFilePath() == filePath)
                {
                    ui->play_table->playPos = count_row;
                }
                neededList.append(file);
                count_row++;
            }
        }
        ui->play_table->setTable(neededList);
    }
}

void MainWindow::onPlayTableCellDoubleClicked(int row, int column)
{
    QString path = this->ui->play_table->getPath(row);
    if(path == "")
    {
        qDebug() << "视频列表双击空对象";
        return;
    }
    QDir dir(path);
    if(!dir.exists(path))
    {
        qDebug() << "视频已不存在";
        setListFromFilePath();
        return;
    }
    filePath = path;
    saveFilePath();
    ASongFFmpeg::getInstance()->stop();
    ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
    this->ui->play_table->playPos = row; //确认可以播放，记录播放位置
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction(); //能够在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
    {
        return ;
    }
    QFileInfo _fileInfo(urls.first().toString());
    //判断是否属于支持的音视频文件
    if(this->ui->play_table->isNeededFile(_fileInfo))
    {
        filePath = urls.first().toLocalFile();
        setListFromFilePath();
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
        saveFilePath();
    }
    else
    {
        qDebug() << "拖拽文件类型不匹配";
    }
}

void MainWindow::saveFilePath()
{
    //存储播放路径txt
    QFile file(SavePath);
    if(!file.open(QIODevice::WriteOnly))//以写的方式打开文件，如果文件不存在则创建，
    {
        qDebug() << file.errorString();
    }
    //    QString ch_filename=filename.toUtf8();
    QByteArray buf = filePath.toUtf8();
    file.write(buf);
    file.close();
}

void MainWindow::readFilePath()
{
    //读取，设置播放路径
    QFile file(SavePath);
    if(!file.open(QIODevice::ReadOnly))//以读的方式打开文件，如果文件不存在则报错，
    {
        qDebug() << file.errorString();
    }
    QByteArray line = file.readAll();
    filePath = QString(line);
    file.close();
    //通过filename设置播放列表
    setListFromFilePath();
}

void MainWindow::handleTimeout()
{
    int duration = ASongFFmpeg::getInstance()->getDuration();
    int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();

    if(duration == 0) return;  //没有视频就不用动
    //进度条
    int posSlider = 10000.0 * nowSec / duration;
    ui->position_ctrl->setValue(posSlider);
    //进度时间
    ui->position_duration->setText(getTimeString(nowSec)+"/"+getTimeString(duration));
}

void MainWindow::on_fullScreen_button_clicked()
{
    qDebug()<<"全屏";
//    ui->play_widget->showFullScreen();
//    ui->play_widget->resize(this->size());
//    ui->play_widget->move(0, 0);
}

//上一首
void MainWindow::on_last_button_clicked()
{
    qint16 n=ui->play_table->numFile;
    if(n==0){
        on_play_button_clicked();
        return;
    }
    ui->play_table->playPos =(ui->play_table->playPos-1 + n) % n;
    filePath=ui->play_table->getPath(ui->play_table->playPos);
    ASongFFmpeg::getInstance()->stop();
    ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
    saveFilePath();

}

//下一首
void MainWindow::on_next_button_clicked()
{
     qint16 n=ui->play_table->numFile;
    if(n==0){
        on_play_button_clicked();
        return;
    }
    ui->play_table->playPos =(ui->play_table->playPos + 1 + n) % n;
    filePath=ui->play_table->getPath(ui->play_table->playPos);
    ASongFFmpeg::getInstance()->stop();
    ASongFFmpeg::getInstance()->play(this, filePath, this->ui->play_widget);
    saveFilePath();

}

void MainWindow::on_position_ctrl_sliderPressed()
{
    //停下定时器
    this->myTimer->stop();
}


void MainWindow::on_position_ctrl_sliderReleased()
{
    //重开定时器
    this->myTimer->start();
    //切换进度要在松开鼠标后实现
    double percentage = 1.0 * ui->position_ctrl->value() / 10000.0; //精确到小数点后四位
    int posSec = ASongFFmpeg::getInstance()->getDuration() * percentage;
    ASongFFmpeg::getInstance()->seek(posSec);
}

void MainWindow::on_play_widget_customContextMenuRequested(const QPoint &pos)
{
    QMenu *cmenu = new QMenu(ui->title_widget);

    //定义菜单项
    QAction *openFIle = new QAction(tr("打开文件"), this);

    QMenu *playMode = new QMenu(tr("模式"), this);
        //四种播放模式
        QAction *mode0 = new QAction(tr("单次播放"), this);
        QAction *mode1 = new QAction(tr("顺序播放"), this);
        QAction *mode2 = new QAction(tr("随机播放"), this);
        QAction *mode3 = new QAction(tr("单曲循环"), this);

    QAction *play = new QAction(tr(ui->play_button->text().toStdString().c_str()), this);
    QAction *last = new QAction(tr("上一个"), this);
    QAction *next = new QAction(tr("下一个"), this);
    QAction *stop = new QAction(tr("停止"), this);

    QAction *muteOrUnmute = new QAction(tr(ui->mute_button->text().toStdString().c_str()), this);
    QAction *fullScreen = new QAction(tr("全屏"), this);
    //添加菜单项
    cmenu->addAction(openFIle);

    cmenu->addMenu(playMode);
        playMode->addAction(mode0);
        playMode->addAction(mode1);
        playMode->addAction(mode2);
        playMode->addAction(mode3);
    cmenu->addAction(play);
    cmenu->addAction(last);
    cmenu->addAction(next);
    cmenu->addAction(stop);

    cmenu->addAction(muteOrUnmute);
    cmenu->addAction(fullScreen);

    //连接槽
    connect(openFIle, SIGNAL(triggered(bool)), this, SLOT(openFile()));
    connect(mode0, SIGNAL(triggered(bool)), this, SLOT(on_playmode_button_clicked()));
    connect(mode1, SIGNAL(triggered(bool)), this, SLOT(on_playmode_button_clicked()));
    connect(mode2, SIGNAL(triggered(bool)), this, SLOT(on_playmode_button_clicked()));
    connect(mode3, SIGNAL(triggered(bool)), this, SLOT(on_playmode_button_clicked()));
    connect(play, SIGNAL(triggered(bool)), this, SLOT(on_play_button_clicked()));
    connect(last, SIGNAL(triggered(bool)), this, SLOT(on_last_button_clicked()));
    connect(next, SIGNAL(triggered(bool)), this, SLOT(on_next_button_clicked()));
    connect(stop, SIGNAL(triggered(bool)), this, SLOT(on_stop_button_clicked()));
    connect(muteOrUnmute, SIGNAL(triggered(bool)), this, SLOT(on_mute_button_clicked()));
    connect(fullScreen, SIGNAL(triggered(bool)), this, SLOT(on_fullScreen_button_clicked()));



    cmenu->exec(QCursor::pos());
}


