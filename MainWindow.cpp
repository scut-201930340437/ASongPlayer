#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ASongAudioOutput.h"
#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"

#include <QDebug>

#include<QFileDialog>
#include "MyMessageWidget.h"
//定时器
<<<<<<< HEAD
QTimer* myTimer= new QTimer();
=======
QTimer* myTimer = new QTimer();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//注冊异常捕获函数
    ui->setupUi(this);
    //    asongFFmpeg = new ASongFFmpeg(ui->screen_widget);
    //定时器
    myTimer->setInterval(100); //0.1秒
    connect(myTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    myTimer->start();
    //鼠标移动相关
    old_mouse_value = QCursor().pos();
    sustain = 0;
    //倍速窗口
    multipleWidget = nullptr;
    //倍速按钮组
    m_pButtonGroup = nullptr;
<<<<<<< HEAD

=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    connect(this->ui->play_table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onPlayTableCellDoubleClicked(int, int)));
    //启用鼠标拖拽放下操做
    setAcceptDrops(true);
    //上一次进程关闭时保存的文件路径
    readFilePath();
<<<<<<< HEAD
    connect(ASongAudioOutput::getInstance(),SIGNAL(playFinish()),this,SLOT(playFinishSlot()));
=======
    connect(ASongAudioOutput::getInstance(), SIGNAL(playFinish()), this, SLOT(playFinishSlot()));
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

MainWindow::~MainWindow()
{
    // 结束线程
    ASongFFmpeg::getInstance()->stop();
    //    if(nullptr != ASongAudio::getInstance())
    //    {
    //        delete ASongAudio::getInstance();
    //    }
    //    if(nullptr != ASongVideo::getInstance())
    //    {
    //        delete ASongVideo::getInstance();
    //    }
    //    if(nullptr != ASongFFmpeg::getInstance())
    //    {
    //        delete ASongFFmpeg::getInstance();
    //    }
    delete ui;
}

QString MainWindow::getTimeString(int position)
{
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
    switch (ASongFFmpeg::getInstance()->getMediaStatus())
    {
        case -1:
        {
            if(filePath != "")
            {
                QDir dir(filePath);
                if(!dir.exists(filePath))
                {
<<<<<<< HEAD
                    filePath="";
=======
                    filePath = "";
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                    break;
                }
                ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
                ui->play_button->setStyleSheet("#play_button{\
<<<<<<< HEAD
                                                   image: url(:/img/pause.png);\
                                               }\
                                               #play_button::hover{\
                                                   image: url(:/img/pause2.png);\
                                               }");
=======
                           image: url(:/img/pause.png);\
                       }\
                       #play_button::hover{\
                           image: url(:/img/pause2.png);\
                       }");
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                ui->play_table->showHighLight();
                break;
            }
            else
            {
                openFile();
                ui->play_button->setStyleSheet("#play_button{\
<<<<<<< HEAD
                                                   image: url(:/img/pause.png);\
                                               }\
                                               #play_button::hover{\
                                                   image: url(:/img/pause2.png);\
                                               }");
=======
                           image: url(:/img/pause.png);\
                       }\
                       #play_button::hover{\
                           image: url(:/img/pause2.png);\
                       }");
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                break;
            }
        }
        case 1:
            ASongFFmpeg::getInstance()->pause();
            ui->play_button->setStyleSheet("#play_button{\
                                               image: url(:/img/play.png);\
                                           }\
                                           #play_button::hover{\
                                               image: url(:/img/play2.png);\
                                           }");
            break;
        case 2:
        {
            ASongFFmpeg::getInstance()->resume();
            ui->play_button->setStyleSheet("#play_button{\
<<<<<<< HEAD
                                               image: url(:/img/pause.png);\
                                           }\
                                           #play_button::hover{\
                                               image: url(:/img/pause2.png);\
                                           }");
=======
                       image: url(:/img/pause.png);\
                   }\
                   #play_button::hover{\
                       image: url(:/img/pause2.png);\
                   }");
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            break;
        }
        case 0:
        {
            ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
            ui->play_button->setStyleSheet("#play_button{\
<<<<<<< HEAD
                                               image: url(:/img/play.png);\
                                           }\
                                           #play_button::hover{\
                                               image: url(:/img/play2.png);\
                                           }");
=======
                       image: url(:/img/play.png);\
                   }\
                   #play_button::hover{\
                       image: url(:/img/play2.png);\
                   }");
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            break;
        }
    }
}

void MainWindow::on_stop_button_clicked()
{
    ui->play_button->setStyleSheet("#play_button{\
                                       image: url(:/img/play.png);\
                                   }\
                                   #play_button::hover{\
                                       image: url(:/img/play2.png);\
                                   }");
    QMutexLocker locker(&ASongFFmpeg::_mediaStatusMutex);
    if(ASongFFmpeg::getInstance()->getMediaStatus() > 0)
    {
        int ret = ASongFFmpeg::getInstance()->stop();
        if(ret == -1)
        {
            qDebug() << "stop failed";
        }
    }
}


void MainWindow::on_playmode_button_clicked()
{
    //播放模式 0 只播当前 1 顺序 2 随机 3 单个循环
    qint16 playMode = (ui->play_table->playMode + 1) % 4;
    //CJJ把它封装出去以便复用
    setPlayMode(playMode);
}


void MainWindow::on_mute_button_clicked()
{
    if(ui->volume_ctrl->value() == 0)
    {
        ui->volume_ctrl->setOldValue();
        ui->mute_button->setStyleSheet("#mute_button{\
                                           image: url(:/img/volume.png);\
                                       }\
                                       #mute_button::hover{\
                                           image: url(:/img/volume2.png);\
                                       }");
    }
    else
    {
        ui->volume_ctrl->setZeroValue();
        ui->mute_button->setStyleSheet("#mute_button{\
                                           image: url(:/img/mute.png);\
                                       }\
                                       #mute_button::hover{\
                                           image: url(:/img/mute2.png);\
                                       }");
    }
    ASongAudio::getInstance()->setVolume(ui->volume_ctrl->value());
}


void MainWindow::on_volume_ctrl_valueChanged(int value)
{
    if(value != 0)
    {
        ui->mute_button->setStyleSheet("#mute_button{\
                                           image: url(:/img/volume.png);\
                                       }\
                                       #mute_button::hover{\
                                           image: url(:/img/volume2.png);\
                                       }");
    }
    if(value == 0)
    {
<<<<<<< HEAD
    ui->mute_button->setStyleSheet("#mute_button{\
=======
        ui->mute_button->setStyleSheet("#mute_button{\
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
                                       image: url(:/img/mute.png);\
                                   }\
                                   #mute_button::hover{\
                                       image: url(:/img/mute2.png);\
                                   }");
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


void MainWindow::on_title_widget_customContextMenuRequested(const QPoint &/*pos*/)
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
    QPoint q = QPoint(10, 10);
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
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, path, (void*)ui->play_widget->winId());
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
<<<<<<< HEAD
        QList<QString> neededList;
=======
        QFileInfoList neededList;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        qint16 count_row = 0;
        foreach (QFileInfo file, _list)                  			//遍历只加载音视频文件到文件列表
        {
            if(ui->play_table->isNeededFile(file))          //判断进行再次确认是可播放文件
            {
                if(file.absoluteFilePath() == filePath)
                {
                    ui->play_table->playPos = count_row;
                }
<<<<<<< HEAD
                neededList.append(file.absoluteFilePath());
=======
                neededList.append(file);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
    if(checkIfExist(path))
    {
        filePath = path;
        saveFilePath();
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
        this->ui->play_table->playPos = row; //确认可以播放，记录播放位置
    }
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
        ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
        saveFilePath();
    }
    else
    {
        qDebug() << "拖拽文件类型不匹配";
    }
}

void MainWindow::saveFilePath()
{
    //存储播放路径
<<<<<<< HEAD
    QSettings *iniWriter=new QSettings(SavePath,QSettings::IniFormat);
    if(iniWriter)
    {
        iniWriter->setValue(curPathKey,filePath);
        iniWriter->setValue(pathListKey,ui->play_table->orderInfoList);
    }

=======
    QSettings *iniWriter = new QSettings(SavePath, QSettings::IniFormat);
    if(iniWriter)
    {
        iniWriter->setValue(SavePath, filePath);
    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    delete iniWriter;
}

void MainWindow::readFilePath()
{
<<<<<<< HEAD
    QSettings *iniReader=new QSettings(SavePath,QSettings::IniFormat);
    if(iniReader)
    {
        filePath = iniReader->value(curPathKey).toString();
        //判断filePath是否存在
        QDir dir(filePath);
        if(!dir.exists(filePath))
        {
            filePath="";
            return;
        }

//        QFileInfoList *fileInfoList = new QFileInfoList(iniReader->value("fileInfoList").value<QFileInfoList>());
        QList<QString> filePathList=iniReader->value(pathListKey).value<QList<QString>>();
        if(filePathList.empty())
=======
    QSettings *iniReader = new QSettings(SavePath, QSettings::IniFormat);
    if(iniReader)
    {
        filePath = iniReader->value(SavePath).toString();
        QDir dir(filePath);
        if(!dir.exists(filePath))
        {
            filePath = "";
            return;
        }
        QFileInfoList *fileInfoList = new QFileInfoList(iniReader->value("fileInfoList").value<QFileInfoList>());
        if(fileInfoList->empty())
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        {
            //通过filePath设置播放列表
            setListFromFilePath();
        }
        else
        {
<<<<<<< HEAD
            ui->play_table->setTable(filePathList);
        }
=======
            ui->play_table->setTable(*fileInfoList);
        }
        delete fileInfoList;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    }
}

void MainWindow::handleTimeout()
{
    int duration = ASongFFmpeg::getInstance()->getDuration();
    int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
<<<<<<< HEAD
//    if(duration == 0)
//    {
//        return;    //没有视频就不用动
//    }
=======
    //    if(duration == 0)
    //    {
    //        return;    //没有视频就不用动
    //    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //停止后的进度条
    if(ASongFFmpeg::getInstance()->getMediaStatus() == 0)
    {
        ui->position_ctrl->setValue(0);
        duration = 0;
        nowSec = 0;
    }
    //播放时的进度条
    else
    {
        int posSlider = 10000.0 * nowSec / duration;
        ui->position_ctrl->setValue(posSlider);
    }
    //进度时间
    ui->position_duration->setText(getTimeString(nowSec) + "/" + getTimeString(duration));
    //鼠标隐藏计时
<<<<<<< HEAD
    if(QCursor().pos() == old_mouse_value) sustain = sustain + 1 <= 30 ? sustain + 1 : 30;
    else{
=======
    if(QCursor().pos() == old_mouse_value)
    {
        sustain = sustain + 1 <= 30 ? sustain + 1 : 30;
    }
    else
    {
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        sustain = 0;
        old_mouse_value = QCursor().pos();
    }
    //鼠标隐藏实现
<<<<<<< HEAD
    if(ui->play_widget->width()==ui->centralwidget->width())
=======
    if(ui->play_widget->width() == ui->centralwidget->width())
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    {
        if(sustain == 30)
        {
            this->setCursor(QCursor(Qt::BlankCursor));
            ASongFFmpeg::getInstance()->hideCursor();
            //顺便隐藏这个控制栏
            ui->control_widget->hide();
        }
        else
        {
            this->setCursor(QCursor(Qt::ArrowCursor));
            ASongFFmpeg::getInstance()->showCursor();
        }
    }
    //图标更新
    if(ASongFFmpeg::getInstance()->getMediaStatus() == 1)
    {
        ui->play_button->setStyleSheet("#play_button{\
                                           image: url(:/img/pause.png);\
                                       }\
                                       #play_button::hover{\
                                           image: url(:/img/pause2.png);\
                                       }");
    }
    else
    {
        ui->play_button->setStyleSheet("#play_button{\
                                   image: url(:/img/play.png);\
                               }\
                               #play_button::hover{\
                                   image: url(:/img/play2.png);\
                               }");
    }
<<<<<<< HEAD
=======
    //    //倍速窗口调整位置，避免他打开是进行别的操作导致小bug，好像没有用耶
    //    if(multipleWidget != nullptr)
    //    {
    //        //位置偏差
    //        QPoint *p = new QPoint(-0.3*multipleWidget->size().width(),-1*multipleWidget->size().height());
    //        multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
    //        multipleWidget->move(0,0);
    //    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

void MainWindow::on_fullScreen_button_clicked()
{
    //倍速窗口关闭
<<<<<<< HEAD
    if(multipleWidget!=nullptr)
=======
    if(multipleWidget != nullptr)
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    {
        multipleWidget->close();
        multipleWidget = nullptr;
    }
<<<<<<< HEAD

=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    if(ui->play_widget->size() == this->size())
    {
        //控制窗口回归
        ui->central_layout->addWidget(ui->control_widget);
        //鼠标回归
        this->setCursor(QCursor(Qt::ArrowCursor));
        ASongFFmpeg::getInstance()->showCursor();
        ui->title_widget->show();
        ui->play_table->show();
        ui->control_widget->show();
        this->showNormal();
        ui->play_widget->showNormal();
        ui->fullScreen_button->setStyleSheet("#fullScreen_button{\
                                           image: url(:/img/fullScreen.png);\
                                       }\
                                       #fullScreen_button::hover{\
                                           image: url(:/img/fullScreen2.png);\
                                       }");
    }
    else
    {
        ui->title_widget->hide();
        ui->play_table->hide();
        ui->control_widget->hide();
        this->showFullScreen();
        ui->play_widget->resize(this->size());
        ui->play_widget->move(0, 0);
        ui->fullScreen_button->setStyleSheet("#fullScreen_button{\
                                           image: url(:/img/fullScreenCancel.png);\
                                       }\
                                       #fullScreen_button::hover{\
                                           image: url(:/img/fullScreenCancel2.png);\
                                       }");
        //控制窗口独立出去，但是内存里面仍然保留，会以单独的widget显示
        //ui->control_widget->setParent(NULL);  //断绝父子关系后透明度就可以变了，但键盘事件就没有了
<<<<<<< HEAD
=======
        //        ui->control_widget->setParent(NULL);  //断绝父子关系后透明度就可以变了，但键盘事件就没有了
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        ui->central_layout->removeWidget(ui->control_widget);
        //去掉多出来的标题栏
        ui->control_widget->setWindowFlags(Qt::FramelessWindowHint);
        //调整控制窗口，适合全屏时的显示
<<<<<<< HEAD
        ui->control_widget->move(0,ui->play_widget->height() - ui->control_widget->height());
        ui->control_widget->resize(ui->play_widget->width(),ui->control_widget->height());
        //修改样式
        ui->control_widget->setStyleSheet("background:rgba(0,0,0,1);");
//        //改透明度
//        ui->control_widget->setWindowOpacity(0.9);
//        //键盘焦点代理，好像没什么用
//        ui->control_widget->setFocusProxy(this);
//        this->setFocus();
//        ui->control_widget->releaseKeyboard();
//        //透明度
//        QGraphicsOpacityEffect *opacityEffect=new QGraphicsOpacityEffect;
//        opacityEffect->setOpacity(0.7);
//        ui->control_widget->setGraphicsEffect(opacityEffect);
=======
        ui->control_widget->move(0, ui->play_widget->height() - ui->control_widget->height());
        ui->control_widget->resize(ui->play_widget->width(), ui->control_widget->height());
        //修改样式
        ui->control_widget->setStyleSheet("background:rgba(0,0,0,1);");
        //        //改透明度
        //        ui->control_widget->setWindowOpacity(0.9);
        //        //键盘焦点代理，好像没什么用
        //        ui->control_widget->setFocusProxy(this);
        //        this->setFocus();
        //        ui->control_widget->releaseKeyboard();
        //        //透明度
        //        QGraphicsOpacityEffect *opacityEffect=new QGraphicsOpacityEffect;
        //        opacityEffect->setOpacity(0.7);
        //        ui->control_widget->setGraphicsEffect(opacityEffect);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    }
}

//上一首
void MainWindow::on_last_button_clicked()
{
    qint16 n = ui->play_table->numFile;
    if(n == 0)
    {
        on_play_button_clicked();
        return;
    }
<<<<<<< HEAD
    QString path =ui->play_table->getPrevFile();
    if(checkIfExist(path))
    {
        filePath=path;
=======
    QString path = ui->play_table->getPrevFile();
    if(checkIfExist(path))
    {
        filePath = path;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
        saveFilePath();
        ui->play_table->showHighLight();
    }
}

//下一首
void MainWindow::on_next_button_clicked()
{
    qint16 n = ui->play_table->numFile;
    if(n == 0)
    {
        on_play_button_clicked();
        return;
    }
<<<<<<< HEAD
    QString path=ui->play_table->getNextFile();
    if(checkIfExist(path))
    {
        filePath=path;
=======
    QString path = ui->play_table->getNextFile();
    if(checkIfExist(path))
    {
        filePath = path;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, (void*)ui->play_widget->winId());
        saveFilePath();
        ui->play_table->showHighLight();
    }
}

void MainWindow::on_position_ctrl_sliderPressed()
{
<<<<<<< HEAD
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0) return;
=======
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
    {
        return;
    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //停下定时器
    myTimer->stop();
}


void MainWindow::on_position_ctrl_sliderReleased()
{
<<<<<<< HEAD
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0) return;

    //先加锁
    seek_mutex.tryLock();

=======
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
    {
        return;
    }
    //先加锁
    seek_mutex.tryLock();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //切换进度要在松开鼠标后实现
    double percentage = 1.0 * ui->position_ctrl->value() / 10000.0; //精确到小数点后四位
    int posSec = ASongFFmpeg::getInstance()->getDuration() * percentage;
    ASongFFmpeg::getInstance()->seek(posSec);
    //重开定时器
    myTimer->start();
<<<<<<< HEAD

    //解锁
    seek_mutex.unlock();

=======
    //解锁
    seek_mutex.unlock();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //seek完后一定转入播放状态，所以按钮为”暂停“
    ui->play_button->setStyleSheet("#play_button{\
                                       image: url(:/img/pause.png);\
                                   }\
                                   #play_button::hover{\
                                       image: url(:/img/pause2.png);\
                                   }");
}

void MainWindow::on_play_widget_customContextMenuRequested(const QPoint &/*pos*/)
{
    QMenu *cmenu = new QMenu(ui->title_widget);
<<<<<<< HEAD
    cmenu->resize(100,100);
=======
    cmenu->resize(100, 100);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //定义菜单项
    QAction *openFIle = new QAction(tr("打开文件"), this);
    QMenu *playMode = new QMenu(tr("模式"), this);
    //四种播放模式
    QAction *mode0 = new QAction(tr("单次播放"), this);
    QAction *mode1 = new QAction(tr("顺序播放"), this);
    QAction *mode2 = new QAction(tr("随机播放"), this);
    QAction *mode3 = new QAction(tr("单曲循环"), this);
<<<<<<< HEAD
    QAction *play = new QAction(tr(ASongFFmpeg::getInstance()->getMediaStatus()==1?"暂停":"播放"), this);
//    QAction *play = new QAction(tr(ui->play_button->text().toStdString().c_str()), this);
    QAction *last = new QAction(tr("上一个"), this);
    QAction *next = new QAction(tr("下一个"), this);
    QAction *stop = new QAction(tr("停止"), this);
    QAction *muteOrUnmute = new QAction(tr(ui->volume_ctrl->value()==0?"解除静音":"静音"), this);
    QAction *fullScreen = new QAction(tr(ui->play_widget->size() == this->size()?"退出全屏":"全屏"), this);
=======
    QAction *play = new QAction(tr(ASongFFmpeg::getInstance()->getMediaStatus() == 1 ? "暂停" : "播放"), this);
    //    QAction *play = new QAction(tr(ui->play_button->text().toStdString().c_str()), this);
    QAction *last = new QAction(tr("上一个"), this);
    QAction *next = new QAction(tr("下一个"), this);
    QAction *stop = new QAction(tr("停止"), this);
    QAction *muteOrUnmute = new QAction(tr(ui->volume_ctrl->value() == 0 ? "解除静音" : "静音"), this);
    QAction *fullScreen = new QAction(tr(ui->play_widget->size() == this->size() ? "退出全屏" : "全屏"), this);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
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
<<<<<<< HEAD
//    connect(mode0, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(0);}));
//    connect(mode1, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(1);}));
//    connect(mode2, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(2);}));
//    connect(mode3, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(3);}));
    //lambda表达式yyds
    connect(mode0, &QAction::triggered, this, [=](){setPlayMode(0);});
    connect(mode1, &QAction::triggered, this, [=](){setPlayMode(1);});
    connect(mode2, &QAction::triggered, this, [=](){setPlayMode(2);});
    connect(mode3, &QAction::triggered, this, [=](){setPlayMode(3);});
=======
    //    connect(mode0, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(0);}));
    //    connect(mode1, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(1);}));
    //    connect(mode2, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(2);}));
    //    connect(mode3, SIGNAL(triggered(bool)), this, SLOT([=](){setPlayMode(3);}));
    //lambda表达式yyds
    connect(mode0, &QAction::triggered, this, [ = ]()
    {
        setPlayMode(0);
    });
    connect(mode1, &QAction::triggered, this, [ = ]()
    {
        setPlayMode(1);
    });
    connect(mode2, &QAction::triggered, this, [ = ]()
    {
        setPlayMode(2);
    });
    connect(mode3, &QAction::triggered, this, [ = ]()
    {
        setPlayMode(3);
    });
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    connect(play, SIGNAL(triggered(bool)), this, SLOT(on_play_button_clicked()));
    connect(last, SIGNAL(triggered(bool)), this, SLOT(on_last_button_clicked()));
    connect(next, SIGNAL(triggered(bool)), this, SLOT(on_next_button_clicked()));
    connect(stop, SIGNAL(triggered(bool)), this, SLOT(on_stop_button_clicked()));
    connect(muteOrUnmute, SIGNAL(triggered(bool)), this, SLOT(on_mute_button_clicked()));
    connect(fullScreen, SIGNAL(triggered(bool)), this, SLOT(on_fullScreen_button_clicked()));
    cmenu->exec(QCursor::pos());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //按下Ctrl键
    if(event->modifiers() == Qt::ControlModifier)
    {
        switch (event->key())
        {
<<<<<<< HEAD
        case Qt::Key_Left:
            on_last_button_clicked();
            break;
        case Qt::Key_Right:
            on_next_button_clicked();
            break;
        case Qt::Key_F:
            on_fullScreen_button_clicked();
            break;
=======
            case Qt::Key_Left:
                on_last_button_clicked();
                break;
            case Qt::Key_Right:
                on_next_button_clicked();
                break;
            case Qt::Key_F:
                on_fullScreen_button_clicked();
                break;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
        return;
    }
    //单键
    switch (event->key())
    {
<<<<<<< HEAD
    case Qt::Key_Space:
        on_play_button_clicked();
        break;
    case Qt::Key_Up:
        ui->volume_ctrl->value()+10>100?ui->volume_ctrl->setValue(100):ui->volume_ctrl->setValue(ui->volume_ctrl->value()+10);
        break;
    case Qt::Key_Down:
        ui->volume_ctrl->value()-10<0?ui->volume_ctrl->setValue(0):ui->volume_ctrl->setValue(ui->volume_ctrl->value()-10);
        break;
    case Qt::Key_Left:
    {
        //先加锁
//        seek_mutex.tryLock();

        int duration = ASongFFmpeg::getInstance()->getDuration();
        int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
        nowSec = nowSec - 5 < 0 ? 0 : nowSec - 5;
        int posSlider = 10000.0 * nowSec / duration; //精确到小数点后四位
        ASongFFmpeg::getInstance()->seek(nowSec);
        ui->position_ctrl->setValue(posSlider);

        //解锁
//        seek_mutex.unlock();

        //seek完后一定转入播放状态，所以按钮为”暂停“
        ui->play_button->setStyleSheet("#play_button{\
                                           image: url(:/img/pause.png);\
                                       }\
                                       #play_button::hover{\
                                           image: url(:/img/pause2.png);\
                                       }");
    }
        break;
    case Qt::Key_Right:
    {
        //先加锁
//        seek_mutex.tryLock();

        int duration = ASongFFmpeg::getInstance()->getDuration();
        int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
        nowSec = nowSec + 10 >= duration ? duration - 1 : nowSec + 10;
        int posSlider = 10000.0 * nowSec / duration; //精确到小数点后四位
        ASongFFmpeg::getInstance()->seek(nowSec);
        ui->position_ctrl->setValue(posSlider);

        //解锁
//        seek_mutex.unlock();

        //seek完后一定转入播放状态，所以按钮为”暂停“
        ui->play_button->setStyleSheet("#play_button{\
                                           image: url(:/img/pause.png);\
                                       }\
                                       #play_button::hover{\
                                           image: url(:/img/pause2.png);\
                                       }");
    }
        break;
    case Qt::Key_Escape:
        if(ui->play_widget->size() == this->size())
        {
            on_fullScreen_button_clicked();
        }
        break;
=======
        case Qt::Key_Space:
            on_play_button_clicked();
            break;
        case Qt::Key_Up:
            ui->volume_ctrl->value() + 10 > 100 ? ui->volume_ctrl->setValue(100) : ui->volume_ctrl->setValue(ui->volume_ctrl->value() + 10);
            break;
        case Qt::Key_Down:
            ui->volume_ctrl->value() - 10 < 0 ? ui->volume_ctrl->setValue(0) : ui->volume_ctrl->setValue(ui->volume_ctrl->value() - 10);
            break;
        case Qt::Key_Left:
        {
            //先加锁
            //        seek_mutex.tryLock();
            int duration = ASongFFmpeg::getInstance()->getDuration();
            int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
            nowSec = nowSec - 5 < 0 ? 0 : nowSec - 5;
            int posSlider = 10000.0 * nowSec / duration; //精确到小数点后四位
            ASongFFmpeg::getInstance()->seek(nowSec);
            ui->position_ctrl->setValue(posSlider);
            //解锁
            //        seek_mutex.unlock();
            //seek完后一定转入播放状态，所以按钮为”暂停“
            ui->play_button->setStyleSheet("#play_button{\
                   image: url(:/img/pause.png);\
               }\
               #play_button::hover{\
                   image: url(:/img/pause2.png);\
               }");
        }
        break;
        case Qt::Key_Right:
        {
            //先加锁
            //        seek_mutex.tryLock();
            int duration = ASongFFmpeg::getInstance()->getDuration();
            int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
            nowSec = nowSec + 10 >= duration ? duration - 1 : nowSec + 10;
            int posSlider = 10000.0 * nowSec / duration; //精确到小数点后四位
            ASongFFmpeg::getInstance()->seek(nowSec);
            ui->position_ctrl->setValue(posSlider);
            //解锁
            //        seek_mutex.unlock();
            //seek完后一定转入播放状态，所以按钮为”暂停“
            ui->play_button->setStyleSheet("#play_button{\
                   image: url(:/img/pause.png);\
               }\
               #play_button::hover{\
                   image: url(:/img/pause2.png);\
               }");
        }
        break;
        case Qt::Key_Escape:
            if(ui->play_widget->size() == this->size())
            {
                on_fullScreen_button_clicked();
            }
            break;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
<<<<<<< HEAD

=======
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
<<<<<<< HEAD
    if(ui->play_widget->width()==ui->centralwidget->width())
    {
        //显示操作栏
        if(QCursor().pos().y() >= ui->play_widget->height()*0.8)
=======
    if(ui->play_widget->width() == ui->centralwidget->width())
    {
        //显示操作栏
        if(QCursor().pos().y() >= ui->play_widget->height() * 0.8)
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        {
            ui->control_widget->show();
        }
        else
        {
            ui->control_widget->hide();
        }
    }
}

void MainWindow::playFinishSlot()
{
    ASongFFmpeg::getInstance()->stop();
<<<<<<< HEAD
    if(ui->play_table->playMode==0)
=======
    if(ui->play_table->playMode == 0)
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    {
        //单次播放
        return;
    }
<<<<<<< HEAD
    else if(ui->play_table->playMode==3)
=======
    else if(ui->play_table->playMode == 3)
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    {
        //单曲循环
        on_play_button_clicked();
    }
    else
    {
        on_next_button_clicked();
    }
<<<<<<< HEAD
     ui->play_button->setText("播放");
=======
    ui->play_button->setText("播放");
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

void MainWindow::on_play_table_customContextMenuRequested(const QPoint &pos)
{
    QString tmp;
    QMenu *pMenu = new QMenu(this);
    QAction *mes = new QAction(tr("详细信息"), this);
    QAction *dlt = new QAction(tr("删除文件"), this);
<<<<<<< HEAD

    /* 添加菜单项 */
    pMenu->addAction(mes);
    pMenu->addAction(dlt);

    /* 连接槽函数 */
    connect(mes, SIGNAL(triggered()), ui->play_table, SLOT(showMessage()));
    connect(dlt, SIGNAL(triggered()), ui->play_table, SLOT(deleteFile()));  //直接触发窗口的close函数

    /* 在鼠标右键处显示菜单 */
    pMenu->exec(cursor().pos());

    /* 释放内存 */
    QList<QAction*> list = pMenu->actions();
    foreach (QAction* pAction, list) delete pAction;
=======
    /* 添加菜单项 */
    pMenu->addAction(mes);
    pMenu->addAction(dlt);
    /* 连接槽函数 */
    connect(mes, SIGNAL(triggered()), ui->play_table, SLOT(showMessage()));
    connect(dlt, SIGNAL(triggered()), ui->play_table, SLOT(deleteFile()));  //直接触发窗口的close函数
    /* 在鼠标右键处显示菜单 */
    pMenu->exec(cursor().pos());
    /* 释放内存 */
    QList<QAction*> list = pMenu->actions();
    foreach (QAction * pAction, list) delete pAction;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    delete pMenu;
}

bool MainWindow::checkIfExist(QString path)
{
    QDir dir(path);
    if(!dir.exists(path))
    {
<<<<<<< HEAD
        MyMessageWidget *infoWindow=new MyMessageWidget();
=======
        MyMessageWidget *infoWindow = new MyMessageWidget();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        infoWindow->show();
        setListFromFilePath();
        return false;
    }
    return true;
}

void MainWindow::setPlayMode(qint16 mode_index)
{
<<<<<<< HEAD

//    //即将使用
//    QIcon icon = QIcon(":/img/random.png");
//    QSize *mySize = new QSize(20,20);
//    QPixmap myPic = icon.pixmap(icon.actualSize(*mySize));
//    ui->play_button->setIcon(QIcon(myPic));
//    ui->play_button->setIconSize(*mySize);

    ui->play_table->playMode = mode_index;
    qint16 &randPos=ui->play_table->randomPos;
    qint16 &orderPos=ui->play_table->playPos;
=======
    //    //即将使用
    //    QIcon icon = QIcon(":/img/random.png");
    //    QSize *mySize = new QSize(20,20);
    //    QPixmap myPic = icon.pixmap(icon.actualSize(*mySize));
    //    ui->play_button->setIcon(QIcon(myPic));
    //    ui->play_button->setIconSize(*mySize);
    ui->play_table->playMode = mode_index;
    qint16 &randPos = ui->play_table->randomPos;
    qint16 &orderPos = ui->play_table->playPos;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    ui->playmode_button->setText("");
    switch (mode_index)
    {
        case 0:
<<<<<<< HEAD
            ui->play_table->playMode=0;
            orderPos=ui->play_table->random_order[randPos];
=======
            ui->play_table->playMode = 0;
            orderPos = ui->play_table->random_order[randPos];
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/onlyOnce.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/onlyOnce2.png);\
                                           }");
            ui->playmode_button->setText("1");
<<<<<<< HEAD
//            ui->playmode_button->setText("单次播放");
            break;
        case 1:
            ui->play_table->playMode=1;
            orderPos=ui->play_table->random_order[randPos];
=======
            //            ui->playmode_button->setText("单次播放");
            break;
        case 1:
            ui->play_table->playMode = 1;
            orderPos = ui->play_table->random_order[randPos];
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/loop.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/loop2.png);\
                                           }");
<<<<<<< HEAD
//            ui->playmode_button->setText("顺序播放");
            break;
        case 2:
            ui->play_table->playMode=2;
=======
            //            ui->playmode_button->setText("顺序播放");
            break;
        case 2:
            ui->play_table->playMode = 2;
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/random.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/random2.png);\
                                           }");
<<<<<<< HEAD
//            ui->playmode_button->setText("随机播放");
            randPos=ui->play_table->order_random[orderPos];
            break;
        case 3:
            ui->play_table->playMode=3;
            orderPos=ui->play_table->random_order[randPos];
=======
            //            ui->playmode_button->setText("随机播放");
            randPos = ui->play_table->order_random[orderPos];
            break;
        case 3:
            ui->play_table->playMode = 3;
            orderPos = ui->play_table->random_order[randPos];
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/singleCycle.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/singleCycle2.png);\
                                           }");
<<<<<<< HEAD
//            ui->playmode_button->setText("单曲循环");
            break;
    }

=======
            //            ui->playmode_button->setText("单曲循环");
            break;
    }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
}

void MainWindow::on_multiple_button_clicked()
{
    //倍速选项
<<<<<<< HEAD
    QString vector[6] = {"0.5","1.0","1.5","2.0","4.0","8.0"};
    //关闭或重新打开
    if(multipleWidget != nullptr)
    {
        multipleWidget->isVisible()?multipleWidget->hide():multipleWidget->show();
=======
    QString vector[6] = {"0.5", "1.0", "1.5", "2.0", "4.0", "8.0"};
    //关闭或重新打开
    if(multipleWidget != nullptr)
    {
        multipleWidget->isVisible() ? multipleWidget->hide() : multipleWidget->show();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        //遍历
        QList<QAbstractButton*> list = m_pButtonGroup->buttons();
        foreach (QAbstractButton *pButton, list)
        {
            //设置预选项
            if(pButton->objectName() == ui->maximize_button->text())
<<<<<<< HEAD
                pButton->setChecked(true);
=======
            {
                pButton->setChecked(true);
            }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        }
        return;
    }
    //第一次生成
    multipleWidget = new QWidget(this);//ui->multiple_button->parentWidget()
<<<<<<< HEAD
    multipleWidget->resize(100,200);
    //位置偏差
    QPoint *p = new QPoint(-0.3*multipleWidget->size().width(),-1*multipleWidget->size().height());
    multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
    multipleWidget->setStyleSheet("background:black;border-radius:5px;");
    multipleWidget->show();

=======
    multipleWidget->resize(100, 200);
    //位置偏差
    QPoint *p = new QPoint(-0.3 * multipleWidget->size().width(), -1 * multipleWidget->size().height());
    multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
    multipleWidget->setStyleSheet("background:black;border-radius:5px;");
    multipleWidget->show();
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    //布局
    QVBoxLayout *pLayout = new QVBoxLayout();
    //添加单选按钮组
    m_pButtonGroup  = new QButtonGroup(multipleWidget);
    //互斥
    m_pButtonGroup ->setExclusive(true);
    //添加单选按钮
    for (int i = 0; i < 6; ++i)
    {
        QRadioButton *pButton = new QRadioButton(multipleWidget);
        // 设置文本
        pButton->setText(QString::fromUtf8("%1 倍速").arg(vector[i]));
        //取个名字
        pButton->setObjectName(vector[i]);
        // 设置初选项
        if(i == 1)
<<<<<<< HEAD
            pButton->setChecked(true);

=======
        {
            pButton->setChecked(true);
        }
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
        pLayout->addWidget(pButton);
        m_pButtonGroup->addButton(pButton);
    }
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(20, 0, 0, 0);
<<<<<<< HEAD

    multipleWidget->setLayout(pLayout);

=======
    multipleWidget->setLayout(pLayout);
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    // 连接信号槽
    connect(m_pButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(setMutipleSpeed(QAbstractButton*)));
}

void MainWindow::setMutipleSpeed(QAbstractButton *button)
{
    // 当前点击的按钮
    qDebug() << QString("Clicked Button : %1").arg(button->text());
    qDebug() << button->objectName();
<<<<<<< HEAD

=======
    ASongFFmpeg::getInstance()->setSpeed(atof(button->objectName().toStdString().c_str()));
>>>>>>> 817b993240347ab0a2c666567cd5b09a48d19c4f
    ui->multiple_button->setText(button->objectName());
    //ToBeDone
}
