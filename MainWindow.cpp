#include "MainWindow.h"
#include "ui_MainWindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    asongPlayer = new ASongPlayer;
    asongAudio = new ASongAudio;
    asongVideo = new ASongVideo;
    asongPlayer->init(asongAudio, asongVideo);
    asongPlayer->setParent(this);
    asongVideo->setParent(this->ui->play_widget);
    asongVideo->setAutoFillBackground(true);
    //    videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    //    this->setCentralWidget(ui->play_widget);
    // 设置音量控制块初始参数
    ui->volume_ctrl->setMaximum(ASongAudio::MaxVolume);
    ui->volume_ctrl->setMinimum(ASongAudio::MinVolume);
    //    ui->volume_ctrl->setSingleStep(10);
    ui->volume_ctrl->setValue(ASongAudio::InitVolume);
    ui->volume_ctrl->setTracking(true);
    // 设置进度条初始参数
    ui->position_ctrl->setMinimum(ASongPlayer::MinPosition);
    ui->position_ctrl->setValue(ASongPlayer::MinPosition);
    ui->position_ctrl->setTracking(false);
    // 进度条定时器
    positionTimer = new QTimer(this);
    connect(positionTimer, &QTimer::timeout, [ = ]()
    {
        // 每隔1秒设置更新一次进度条位置
        this->ui->position_ctrl->setValue(int(asongPlayer->position() / ASongPlayer::positionTranRate));
        // 更新已播放时长显示
        qint64 position = asongPlayer->position() / ASongPlayer::positionTranRate;
        QString posi_str=getTimeString(position);
        QString posi_dura_str = posi_str + "/" + durationStr;
        this->ui->position_duration->setText(posi_dura_str);
    });
    // 快捷键
    // 播放/暂停
    ui->play_button->setShortcut(QKeySequence(tr("Space")));
    // 全屏
    ui->fullScreen_button->setShortcut(QKeySequence(tr("Ctrl+F")));
    // 绑定mediaplayer信号事件，如获取总时长
    //    connect(player, SIGNAL(metaDataAvailableChanged(bool)), this, SLOT(OnMetaDataAvailableChanged(bool)));
    connect(asongPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
    //    connect(asongPlayer, SIGNAL(errorOccurred(QMediaPlayer::Error error, const QString & errorString)), this, SLOT(onerrorOccurred(QMediaPlayer::Error error, const QString & errorString)));
    //    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onOriginalStateChanged(QMediaPlayer::State)));
}

MainWindow::~MainWindow()
{
    delete asongAudio;
    asongAudio = nullptr;
    delete asongVideo;
    asongVideo = nullptr;
    delete asongPlayer;
    asongPlayer = nullptr;
    delete ui;
}

//void MainWindow::onerrorOccurred(QMediaPlayer::Error error, const QString & errorString)
//{
//    qDebug() << errorString;
//}
QString MainWindow::getTimeString(qint64 position){
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

// 获取总时长的响应事件
void MainWindow::onDurationChanged(qint64 _duration)
{
    qDebug() << _duration;
    asongPlayer->onDurationChanged(_duration);
    // 将进度条最大值设置为时长（s）
    ui->position_ctrl->setMaximum(_duration / ASongPlayer::positionTranRate);
    // 更新时长显示
    qint64 tmp_duration = _duration / ASongPlayer::positionTranRate;
    durationStr = getTimeString(tmp_duration);
    QString posi_dura_str = "00:00:00/" + durationStr;
    this->ui->position_duration->setText(posi_dura_str);
}

void MainWindow::openFile(){
    // 定义文件对话框类
    filename = QFileDialog::getOpenFileName(this,
                                            tr("选择音视频文件"),
                                            tr("."),
                                            tr(
                                                    "视频文件(*.mp4 *.flv *.avi);;音频文件(*.mp3);;所有文件(*.*)"));
    if(!filename.isEmpty())
    {
        asongPlayer->setSource(QUrl::fromLocalFile(filename));
        asongVideo->show();
        ui->position_ctrl->setValue(0);
        asongPlayer->play();
        positionTimer->start(1000);
        this->ui->play_button->setText("暂停");
    }
    else
    {
        qDebug("打开文件失败！");
    }
}

// 打开文件信号槽
void MainWindow::on_action_openFile_triggered()
{
    //打开文件
    openFile();
}

// 播放按钮
void MainWindow::on_play_button_clicked()
{
    if(asongPlayer->mediaStatus() == QMediaPlayer::NoMedia)
    {
        //打开文件
        openFile();
    }
    else
    {
        qDebug() << asongPlayer->playbackState();
        if(asongPlayer->playbackState() == QMediaPlayer::PlayingState)
        {
            asongPlayer->pause();
            this->ui->play_button->setText("播放");
        }
        else
        {
            asongPlayer->play();
            this->ui->play_button->setText("暂停");
        }
    }
}

// 停止按钮
void MainWindow::on_stop_button_clicked()
{
    asongPlayer->stop();
    this->ui->play_button->setText("播放");
}

// 静音按钮
void MainWindow::on_mute_button_clicked()
{
    if(asongAudio->isMuted())
    {
        asongAudio->unmute();
        this->ui->mute_button->setText("静音");
    }
    else
    {
        asongAudio->mute();
        this->ui->mute_button->setText("解除静音");
    }
}

// 音量控制块
void MainWindow::on_volume_ctrl_valueChanged(int value)
{
    asongAudio->setVolume(value);
    if(asongAudio->isMuted() && value > 0)
    {
        asongAudio->setMuted(false);
        this->ui->mute_button->setText("静音");
    }
    if(value == 0)
    {
        asongAudio->setMuted(true);
        this->ui->mute_button->setText("解除静音");
    }
}

// 拖动进度条的响应事件
void MainWindow::on_position_ctrl_valueChanged(int value)
{
    if(positionCtrlPressed && asongPlayer->mediaStatus() != QMediaPlayer::NoMedia)
    {
        positionCtrlPressed = false;
        // 设置播放时间点
        asongPlayer->setPosition(value);
        // 更新已播放时长显示
        qint64 position = asongPlayer->position() / ASongPlayer::positionTranRate;
        QString posi_str=getTimeString(position);
        QString posi_dura_str = posi_str + "/" + durationStr;
        this->ui->position_duration->setText(posi_dura_str);
        // 重启定时器
        positionTimer->start(1000);
    }
}


void MainWindow::on_position_ctrl_sliderPressed()
{
    positionCtrlPressed = true;
    // 按下进度条时，进度条定时器停止
    positionTimer->stop();
}

// 全屏按钮
void MainWindow::on_fullScreen_button_clicked()
{
    if(asongVideo->isFullScreen())
    {
        this->ui->fullScreen_button->setText("全屏");
        asongVideo->setFullScreen(false);
    }
    else
    {
        asongVideo->setFullScreen(true);
    }
}

// 快捷键
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Up)
    {
        ui->volume_ctrl->setValue(fmin(ui->volume_ctrl->value() + 10, ui->volume_ctrl->maximum()));
    }
    else
    {
        if(event->key() == Qt::Key_Down)
        {
            ui->volume_ctrl->setValue(fmax(ui->volume_ctrl->value() - 10, ui->volume_ctrl->minimum()));
        }
        else
        {
            if(event->key() == Qt::Key_Right)
            {
                ui->position_ctrl->setValue(fmin(ui->position_ctrl->value() + 5, ui->position_ctrl->maximum()));
            }
            else
            {
                if(event->key() == Qt::Key_Left)
                {
                    ui->position_ctrl->setValue(fmax(ui->position_ctrl->value() - 5, ui->position_ctrl->minimum()));
                }
            }
        }
    }
}


//
//void MainWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString)
//{
//    qDebug() << errorString;
//}
