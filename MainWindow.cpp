#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ASongAudioOutput.h"
#include "ASongFFmpeg.h"
#include "ASongAudio.h"
#include "ASongVideo.h"

#include <QDebug>

#include<QFileDialog>
#include "MyMessageWidget.h"

#define STRETCH_RECT_HEIGHT 4       // 拉伸小矩形的高度;
#define STRETCH_RECT_WIDTH 4        // 拉伸小矩形的宽度;

//定时器
QTimer* myTimer = new QTimer();
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //定时器
    myTimer->setInterval(myTimerTime); //0.5秒
    connect(myTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    myTimer->start();
    //鼠标移动相关
    old_mouse_value = QCursor().pos();
    sustain = 0;
    //倍速窗口
    multipleWidget = nullptr;
    //波形图窗口
    waveWidget = nullptr;
    //倍速按钮组
    m_pButtonGroup = nullptr;
    connect(this->ui->play_table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onPlayTableCellDoubleClicked(int, int)));
    //启用鼠标拖拽放下操做
    setAcceptDrops(true);
    //上一次进程关闭时保存的文件路径
    readFilePath();
    connect(ASongAudioOutput::getInstance(), SIGNAL(playFinish()), this, SLOT(playFinishSlot()));
    //缩放窗口
    m_stretchRectState = NO_SELECT;
    m_windowMinHeight = 50;
    m_windowMinWidth = 100;
    m_isWindowMax = false;
    m_isMousePressed = false;
    //初始化边边角角拉伸矩形
    calculateCurrentStrechRect();
    //默认生成波形图窗口，方便释放内核传来的数据
    on_wave_button_clicked();
    waveWidget->hide();
}

MainWindow::~MainWindow()
{
    // 结束线程
    ASongFFmpeg::getInstance()->stop();
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
                    filePath = "";
                    break;
                }
                ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
                ui->play_button->setStyleSheet("#play_button{\
   image: url(:/img/pause.png);\
   }\
   #play_button::hover{\
   image: url(:/img/pause2.png);\
   }");
                break;
            }
            else
            {
                openFile();
                ui->play_button->setStyleSheet("#play_button{\
   image: url(:/img/pause.png);\
   }\
   #play_button::hover{\
   image: url(:/img/pause2.png);\
   }");
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
   image: url(:/img/pause.png);\
   }\
   #play_button::hover{\
   image: url(:/img/pause2.png);\
   }");
            break;
        }
        case 0:
        {
            if(filePath == "")
            {
                openFile();
                break;
            }
            ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
            ui->play_button->setStyleSheet("#play_button{\
   image: url(:/img/play.png);\
   }\
   #play_button::hover{\
   image: url(:/img/play2.png);\
   }");
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
        ui->mute_button->setStyleSheet("#mute_button{\
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
    if(multipleWidget != nullptr && multipleWidget->isVisible())
    {
        multipleWidget->hide();
    }
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
                       "视频文件(*.mp4 *.flv *.avi *.mkv);;音频文件(*.mp3)"));
    if(!path.isEmpty())
    {
        filePath = path;
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, path, ui->play_widget);
        setListFromFilePath();
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
        QList<QString> neededList;
        qint16 count_row = 0;
        foreach (QFileInfo file, _list)                  			//遍历只加载音视频文件到文件列表
        {
            if(ui->play_table->isNeededFile(file))          //判断进行再次确认是可播放文件
            {
                if(file.absoluteFilePath() == filePath)
                {
                    ui->play_table->playPos = count_row;
                }
                neededList.append(file.absoluteFilePath());
                count_row++;
            }
        }
        ui->play_table->setTable(neededList, filePath);
    }
}

void MainWindow::onPlayTableCellDoubleClicked(int row, int column)
{
    ui->play_table->showHighLight(ui->play_table->playPos, row);
    QString path = this->ui->play_table->getPath(row);
    if(path == "")
    {
        qDebug() << "视频列表双击空对象";
        return;
    }
    if(checkIfExist(path))
    {
        filePath = path;
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
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
        ui->play_table->addFilePath(filePath);
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
    }
    else
    {
        qDebug() << "拖拽文件类型不匹配";
    }
}

void MainWindow::saveFilePath()
{
    //存储播放路径
    QSettings *iniWriter = new QSettings(SavePath, QSettings::IniFormat);
    if(iniWriter)
    {
        iniWriter->setValue(curPathKey, filePath);
        iniWriter->setValue(pathListKey, ui->play_table->orderInfoList);
    }
    delete iniWriter;
}

void MainWindow::readFilePath()
{
    QSettings *iniReader = new QSettings(SavePath, QSettings::IniFormat);
    if(iniReader)
    {
        filePath = iniReader->value(curPathKey).toString();
        QDir dir(filePath);
        if(!dir.exists(filePath))
        {
            filePath = "";
            return;
        }
        QList<QString> filePathList = iniReader->value(pathListKey).value<QList<QString>>();
        if(filePathList.empty())
        {
            //通过filePath设置播放列表
            setListFromFilePath();
            qDebug() << "播放路径读取成功，播放路径读取失败";
        }
        else
        {
            ui->play_table->setTable(filePathList, filePath);
        }
    }
}

void MainWindow::handleTimeout()
{
    int duration = ASongFFmpeg::getInstance()->getDuration();
    int nowSec = ASongFFmpeg::getInstance()->getCurPlaySec();
    //    if(duration == 0)
    //    {
    //        return;    //没有视频就不用动
    //    }
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
    if(QCursor().pos() == old_mouse_value)
    {
        sustain = sustain + 1 <= cursorTime / myTimerTime ? sustain + 1 : cursorTime / myTimerTime;
    }
    else
    {
        sustain = 0;
        old_mouse_value = QCursor().pos();
    }
    //鼠标隐藏实现
    if(ui->play_widget->width() == ui->centralwidget->width())
    {
        if(sustain == cursorTime / myTimerTime)
        {
            this->setCursor(QCursor(Qt::BlankCursor));
            ASongFFmpeg::getInstance()->hideCursor();
            //顺便隐藏这个控制栏
            ui->control_widget->hide();
            //顺便隐藏倍速窗口（如果有）
            if(multipleWidget != nullptr && multipleWidget->isVisible())
            {
                multipleWidget->hide();
            }
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
    //    //倍速窗口调整位置，避免他打开是进行别的操作导致小bug，好像没有用耶
    //    if(multipleWidget != nullptr)
    //    {
    //        //位置偏差
    //        QPoint *p = new QPoint(-0.3*multipleWidget->size().width(),-1*multipleWidget->size().height());
    //        multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
    //        multipleWidget->move(0,0);
    //    }
    //倒放图标高亮
    if(ASongFFmpeg::getInstance()->invertFlag)
    {
            ui->reverse_button->setStyleSheet("#reverse_button{\
                                       image: url(:/img/reverse2.png);\
                                   }");
    }
    //倒放屏蔽恢复
    if(!ASongFFmpeg::getInstance()->invertFlag)
    {
        ui->position_ctrl->setEnabled(true);
        ui->forward_button->setEnabled(true);
        ui->backward_button->setEnabled(true);
        ui->reverse_button->setStyleSheet("#reverse_button{\
                                   image: url(:/img/reverse.png);\
                               }");
    }
}

void MainWindow::on_fullScreen_button_clicked()
{
    //倍速窗口关闭
    if(multipleWidget != nullptr && multipleWidget->isVisible())
    {
        multipleWidget->hide();
    }
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
        //        ui->control_widget->setParent(NULL);  //断绝父子关系后透明度就可以变了，但键盘事件就没有了
        ui->central_layout->removeWidget(ui->control_widget);
        //去掉多出来的标题栏
        ui->control_widget->setWindowFlags(Qt::FramelessWindowHint);
        //调整控制窗口，适合全屏时的显示
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
    }
}

//上一首
void MainWindow::on_last_button_clicked()
{
    qint16 n = ui->play_table->orderInfoList.size();
    if(n == 0)
    {
        on_play_button_clicked();
        return;
    }
    QString path = ui->play_table->getPrevFile();
    if(checkIfExist(path))
    {
        filePath = path;
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
    }
}

//下一首
void MainWindow::on_next_button_clicked()
{
    qint16 n = ui->play_table->orderInfoList.size();
    if(n == 0)
    {
        on_play_button_clicked();
        return;
    }
    QString path = ui->play_table->getNextFile();
    if(checkIfExist(path))
    {
        filePath = path;
        ASongFFmpeg::getInstance()->stop();
        ASongFFmpeg::getInstance()->play(this, filePath, ui->play_widget);
    }
}

void MainWindow::on_position_ctrl_sliderPressed()
{
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
    {
        return;
    }
    //停下定时器
    myTimer->stop();
}


void MainWindow::on_position_ctrl_sliderReleased()
{
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
    {
        return;
    }
    //先加锁
    //    seek_mutex.tryLock();
    //切换进度要在松开鼠标后实现
    double percentage = 1.0 * ui->position_ctrl->value() / 10000.0; //精确到小数点后四位
    int posSec = ASongFFmpeg::getInstance()->getDuration() * percentage;
    ASongFFmpeg::getInstance()->seek(posSec);
    //重开定时器
    myTimer->start();
    //解锁
    //    seek_mutex.unlock();
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
    cmenu->resize(100, 100);
    //定义菜单项
    QAction *openFIle = new QAction(tr("打开文件夹 (Ctrl+I)"), this);
    QMenu *playMode = new QMenu(tr("播放模式 (C)"), this);
    //四种播放模式
    QAction *mode0 = new QAction(tr("单次播放"), this);
    QAction *mode1 = new QAction(tr("顺序播放"), this);
    QAction *mode2 = new QAction(tr("随机播放"), this);
    QAction *mode3 = new QAction(tr("单曲循环"), this);
    QAction *play = new QAction(tr(ASongFFmpeg::getInstance()->getMediaStatus() == 1 ? "暂停" : "播放"), this);
    //    QAction *play = new QAction(tr(ui->play_button->text().toStdString().c_str()), this);
    QAction *last = new QAction(tr("上一个 (Ctrl+←)"), this);
    QAction *next = new QAction(tr("下一个 (Ctrl+→)"), this);
    QAction *backward1 = new QAction(tr("上一帧 (A)"), this);
    QAction *forward1 = new QAction(tr("下一帧(D)"), this);
    QAction *stop = new QAction(tr("停止 (S)"), this);
    QAction *muteOrUnmute = new QAction(tr(ui->volume_ctrl->value() == 0 ? "解除静音 (M)" : "静音 (M)"), this);
    QAction *fullScreen = new QAction(tr(ui->play_widget->size() == this->size() ? "退出全屏 (Ctrl+F)" : "全屏 (Ctrl+F)"), this);
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
    cmenu->addAction(backward1);
    cmenu->addAction(forward1);
    cmenu->addAction(stop);
    cmenu->addAction(muteOrUnmute);
    cmenu->addAction(fullScreen);
    //连接槽
    connect(openFIle, SIGNAL(triggered(bool)), this, SLOT(openFile()));
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
    connect(play, SIGNAL(triggered(bool)), this, SLOT(on_play_button_clicked()));
    connect(last, SIGNAL(triggered(bool)), this, SLOT(on_last_button_clicked()));
    connect(next, SIGNAL(triggered(bool)), this, SLOT(on_next_button_clicked()));
    connect(backward1, SIGNAL(triggered(bool)), this, SLOT(on_backward_button_clicked()));
    connect(forward1, SIGNAL(triggered(bool)), this, SLOT(on_forward_button_clicked()));
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
            case Qt::Key_Left:
                on_last_button_clicked();
                break;
            case Qt::Key_Right:
                on_next_button_clicked();
                break;
            case Qt::Key_F:
                on_fullScreen_button_clicked();
                break;
            case Qt::Key_I:
                openFile();
                break;
        }
        return;
    }
    //单键
    switch (event->key())
    {
        case Qt::Key_S:
            on_stop_button_clicked();
            break;
        case Qt::Key_A:
            if(ASongFFmpeg::getInstance()->invertFlag)
            {
                break;
            }
            on_backward_button_clicked();
            break;
        case Qt::Key_D:
            if(ASongFFmpeg::getInstance()->invertFlag)
            {
                break;
            }
            on_forward_button_clicked();
            break;
        case Qt::Key_C:
            on_playmode_button_clicked();
            break;
        case Qt::Key_M:
            on_mute_button_clicked();
            break;
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
            if(ASongFFmpeg::getInstance()->invertFlag)
            {
                break;
            }
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
            if(ASongFFmpeg::getInstance()->invertFlag)
            {
                break;
            }
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
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
}

//搬家了
//void MainWindow::mouseMoveEvent(QMouseEvent *event)
//{
//    if(ui->play_widget->width() == ui->centralwidget->width())
//    {
//        //显示操作栏
//        if(QCursor().pos().y() >= ui->play_widget->height() * 0.8)
//        {
//            ui->control_widget->show();
//        }
//        else
//        {
//            ui->control_widget->hide();
//            //倍速窗口
//            if(multipleWidget != nullptr && multipleWidget->isVisible())
//            {
//                multipleWidget->hide();
//            }
//        }
//    }
//}

void MainWindow::playFinishSlot()
{
    ASongFFmpeg::getInstance()->stop();
    if(ui->play_table->playMode == 0)
    {
        //单次播放
        return;
    }
    else if(ui->play_table->playMode == 3)
    {
        //单曲循环
        on_play_button_clicked();
    }
    else
    {
        on_next_button_clicked();
    }
}

void MainWindow::on_play_table_customContextMenuRequested(const QPoint &pos)
{
    QString tmp;
    QMenu *pMenu = new QMenu(this);
    QAction *mes = new QAction(tr("详细信息"), this);
    QAction *dlt = new QAction(tr("删除文件"), this);
    QAction *clear = new QAction(tr("清空列表"), this);
    QAction *resetRadomList = new QAction(tr("重设随机顺序"), this);
    /* 添加菜单项 */
    pMenu->addAction(mes);
    pMenu->addAction(dlt);
    pMenu->addAction(clear);
    pMenu->addAction(resetRadomList);
    /* 连接槽函数 */
    connect(mes, SIGNAL(triggered()), ui->play_table, SLOT(showMessage()));
    connect(dlt, SIGNAL(triggered()), this, SLOT(deleteFile()));  //直接触发窗口的close函数
    connect(clear, SIGNAL(triggered()), this, SLOT(clearPlayList()));  //直接触发窗口的close函数
    connect(resetRadomList, SIGNAL(triggered()), this->ui->play_table, SLOT(generateRandomList()));  //直接触发窗口的close函数
    /* 在鼠标右键处显示菜单 */
    pMenu->exec(cursor().pos());
    /* 释放内存 */
    QList<QAction*> list = pMenu->actions();
    foreach (QAction * pAction, list) delete pAction;
    delete pMenu;
}

bool MainWindow::checkIfExist(QString path)
{
    QDir dir(path);
    if(!dir.exists(path))
    {
        MyMessageWidget *infoWindow = new MyMessageWidget();
        infoWindow->show();
        ui->play_table->allCloseWidget.append(infoWindow);
        setListFromFilePath();
        return false;
    }
    return true;
}

void MainWindow::setPlayMode(qint16 mode_index)
{
    //    //即将使用
    //    QIcon icon = QIcon(":/img/random.png");
    //    QSize *mySize = new QSize(20,20);
    //    QPixmap myPic = icon.pixmap(icon.actualSize(*mySize));
    //    ui->play_button->setIcon(QIcon(myPic));
    //    ui->play_button->setIconSize(*mySize);
    ui->play_table->playMode = mode_index;
    qint16 &randPos = ui->play_table->randomPos;
    qint16 &orderPos = ui->play_table->playPos;
    ui->playmode_button->setText("");
    switch (mode_index)
    {
        case 0:
            ui->play_table->playMode = 0;
            orderPos = ui->play_table->random_order[randPos];
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/onlyOnce.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/onlyOnce2.png);\
                                           }");
            ui->playmode_button->setText("1");
            //            ui->playmode_button->setText("单次播放");
            break;
        case 1:
            ui->play_table->playMode = 1;
            orderPos = ui->play_table->random_order[randPos];
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/loop.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/loop2.png);\
                                           }");
            //            ui->playmode_button->setText("顺序播放");
            break;
        case 2:
            ui->play_table->playMode = 2;
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/random.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/random2.png);\
                                           }");
            //            ui->playmode_button->setText("随机播放");
            randPos = ui->play_table->order_random[orderPos];
            break;
        case 3:
            ui->play_table->playMode = 3;
            orderPos = ui->play_table->random_order[randPos];
            ui->playmode_button->setStyleSheet("#playmode_button{\
                                               image: url(:/img/singleCycle.png);\
                                           }\
                                           #playmode_button::hover{\
                                               image: url(:/img/singleCycle2.png);\
                                           }");
            //            ui->playmode_button->setText("单曲循环");
            break;
    }
}

void MainWindow::on_multiple_button_clicked()
{
    //倍速选项
    QString vector[6] = {"0.5", "1.0", "1.5", "2.0", "4.0", "8.0"};
    //关闭或重新打开，记得换位置
    if(multipleWidget != nullptr)
    {
        QPoint *p = new QPoint(-0.3 * multipleWidget->size().width(), -1 * multipleWidget->size().height());
        multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
        multipleWidget->isVisible() ? multipleWidget->hide() : multipleWidget->show();
        //遍历
        QList<QAbstractButton*> list = m_pButtonGroup->buttons();
        foreach (QAbstractButton *pButton, list)
        {
            //设置预选项
            if(pButton->objectName() == ui->maximize_button->text())
            {
                pButton->setChecked(true);
            }
        }
        return;
    }
    //第一次生成
    multipleWidget = new QWidget(this);//ui->multiple_button->parentWidget()
    multipleWidget->resize(100, 200);
    //位置偏差
    QPoint *p = new QPoint(-0.3 * multipleWidget->size().width(), -1 * multipleWidget->size().height());
    multipleWidget->move(ui->control_widget->pos() + ui->control_sub_widget->pos() + ui->multiple_button->pos() + *p);
    multipleWidget->setStyleSheet("background:black;border-radius:5px;");
    multipleWidget->show();
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
        //焦点取消
        pButton->setFocusPolicy(Qt::NoFocus);
        // 设置文本
        pButton->setText(QString::fromUtf8("%1 倍速").arg(vector[i]));
        //取个名字
        pButton->setObjectName(vector[i]);
        // 设置初选项
        if(i == 1)
        {
            pButton->setChecked(true);
        }
        pLayout->addWidget(pButton);
        m_pButtonGroup->addButton(pButton);
    }
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(20, 0, 0, 0);
    multipleWidget->setLayout(pLayout);
    // 连接信号槽
    connect(m_pButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(setMutipleSpeed(QAbstractButton*)));
}

void MainWindow::setMutipleSpeed(QAbstractButton *button)
{
    // 当前点击的按钮
    ASongFFmpeg::getInstance()->setSpeed(atof(button->objectName().toStdString().c_str()));
    ui->multiple_button->setText(button->objectName());
    //ToBeDone
}

void MainWindow::deleteFile()
{
    qint16 n = ui->play_table->orderInfoList.size();
    if(n == 0)
    {
        return;
    }
    if( n == 1 )
    {
        clearPlayList();
        return;
    }
    //如果是当前播放，切换filename,保证filename 正确
    if(ui->play_table->currentRow() == ui->play_table->playPos)
    {
        QString path = ui->play_table->getNextFile();
        if(checkIfExist(path))
        {
            filePath = path;
        }
        on_stop_button_clicked();
    }
    ui->play_table->deleteFile();
}

void MainWindow::clearPlayList()
{
    ui->play_table->clear();
    ui->play_table->setRowCount(0);
    on_stop_button_clicked();
    filePath = "";
}

void MainWindow::on_forward_button_clicked()
{
    ASongFFmpeg::getInstance()->step_to_dst_frame(1);
}

void MainWindow::on_backward_button_clicked()
{
    ASongFFmpeg::getInstance()->step_to_dst_frame(-1);
}


void MainWindow::on_wave_button_clicked()
{
    //关闭或重新打开
    if(waveWidget != nullptr)
    {
        waveWidget->isVisible() ? waveWidget->hide() : waveWidget->show();
        return;
    }
    //第一次生成
    waveWidget = new MyPlayWidget();
    waveWidget->resize(300, 600);
    waveWidget->move(this->pos().x(), this->pos().y());
    waveWidget->setStyleSheet("background-color:#44413f");
    waveWidget->setFocusPolicy(Qt::NoFocus);
    waveWidget->show();
    //给个按钮
    QPushButton *change_button = new QPushButton(waveWidget);
    change_button->resize(50, 30);
    change_button->move(0, waveWidget->size().height() - 30 - 5);
    change_button->setText("模式");
    change_button->show();
    change_button->setStyleSheet("background:black");
    connect(change_button, &QPushButton::clicked, waveWidget, &MyPlayWidget::changeWaveMode);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    saveFilePath();
    if(waveWidget != nullptr)
    {
        waveWidget->close();
    }
    for(QWidget * widget : ui->play_table->allCloseWidget)
    {
        widget->close();
    }
    ui->play_table->allCloseWidget.clear();
    event->accept();
}

//缩放窗口
// 计算拉伸区域Rect位置;
// 以下8个Rect对应上图中8个区域;

void MainWindow::calculateCurrentStrechRect()
{
    // 四个角Rect;
    m_leftTopRect = QRect(0, 0, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    m_leftBottomRect = QRect(0, this->height() - STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, STRETCH_RECT_WIDTH);
    m_rightTopRect = QRect(this->width() - STRETCH_RECT_WIDTH, 0, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    m_rightBottomRect = QRect(this->width() - STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    // 四条边Rect;
    m_topBorderRect = QRect(STRETCH_RECT_WIDTH, 0, this->width() - STRETCH_RECT_WIDTH * 2, STRETCH_RECT_HEIGHT);
    m_rightBorderRect = QRect(this->width() - STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT * 2);
    m_bottomBorderRect = QRect(STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT, this->width() - STRETCH_RECT_WIDTH * 2, STRETCH_RECT_HEIGHT);
    m_leftBorderRect = QRect(0, STRETCH_RECT_HEIGHT, STRETCH_RECT_WIDTH, this->height() - STRETCH_RECT_HEIGHT * 2);
}

// 根据当前鼠标位置，定位鼠标在具体哪一块拉伸区域;
WindowStretchRectState MainWindow::getCurrentStretchState(QPoint cursorPos)
{
    WindowStretchRectState stretchState;
    if (m_leftTopRect.contains(cursorPos))
    {
        stretchState = LEFT_TOP_RECT;
    }
    else if (m_rightTopRect.contains(cursorPos))
    {
        stretchState = RIGHT_TOP_RECT;
    }
    else if (m_rightBottomRect.contains(cursorPos))
    {
        stretchState = RIGHT_BOTTOM_RECT;
    }
    else if (m_leftBottomRect.contains(cursorPos))
    {
        stretchState = LEFT_BOTTOM_RECT;
    }
    else if (m_topBorderRect.contains(cursorPos))
    {
        stretchState = TOP_BORDER;
    }
    else if (m_rightBorderRect.contains(cursorPos))
    {
        stretchState = RIGHT_BORDER;
    }
    else if (m_bottomBorderRect.contains(cursorPos))
    {
        stretchState = BOTTOM_BORDER;
    }
    else if (m_leftBorderRect.contains(cursorPos))
    {
        stretchState = LEFT_BORDER;
    }
    else
    {
        stretchState = NO_SELECT;
    }
    return stretchState;
}

// 根据getCurrentStretchState返回状态进行更新鼠标样式;
void MainWindow::updateMouseStyle(WindowStretchRectState stretchState)
{
    switch (stretchState)
    {
        case NO_SELECT:
            setCursor(Qt::ArrowCursor);
            break;
        case LEFT_TOP_RECT:
        case RIGHT_BOTTOM_RECT:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TOP_BORDER:
        case BOTTOM_BORDER:
            setCursor(Qt::SizeVerCursor);
            break;
        case RIGHT_TOP_RECT:
        case LEFT_BOTTOM_RECT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case LEFT_BORDER:
        case RIGHT_BORDER:
            setCursor(Qt::SizeHorCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //搬家了
    if(ui->play_widget->width() == ui->centralwidget->width())
    {
        //显示操作栏
        if(QCursor().pos().y() >= ui->play_widget->height() * 0.8)
        {
            ui->control_widget->show();
        }
        else
        {
            ui->control_widget->hide();
            //倍速窗口
            if(multipleWidget != nullptr && multipleWidget->isVisible())
            {
                multipleWidget->hide();
            }
        }
    }
    // 如果窗口最大化是不能拉伸的;
    // 也不用更新鼠标样式;
    if (m_isWindowMax)
    {
        return __super::mouseMoveEvent(event);
    }
    // 如果当前鼠标未按下，则根据当前鼠标的位置更新鼠标的状态及样式;
    if (!m_isMousePressed)
    {
        QPoint cursorPos = event->pos();
        // 根据当前鼠标的位置显示不同的样式;
        m_stretchRectState = getCurrentStretchState(cursorPos);
        updateMouseStyle(m_stretchRectState);
    }
    // 如果当前鼠标左键已经按下，则记录下第二个点的位置，并更新窗口的大小;
    else
    {
        m_endPoint = this->mapToGlobal(event->pos());
        updateWindowSize();
    }
    return __super::mouseMoveEvent(event);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 当前鼠标进入了以上指定的8个区域，并且是左键按下时才开始进行窗口拉伸;
    if (m_stretchRectState != NO_SELECT && event->button() == Qt::LeftButton)
    {
        m_isMousePressed = true;
        // 记录下当前鼠标位置，为后面计算拉伸位置;
        m_startPoint = this->mapToGlobal(event->pos());
        // 保存下拉伸前的窗口位置及大小;
        m_windowRectBeforeStretch = this->geometry();
    }
    return __super::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标松开后意味之窗口拉伸结束，置标志位，并且重新计算用于拉伸的8个区域Rect;
    m_isMousePressed = false;
    calculateCurrentStrechRect();
    return __super::mouseReleaseEvent(event);
}

// 拉伸窗口过程中，根据记录的坐标更新窗口大小;
void MainWindow::updateWindowSize()
{
    // 拉伸时要注意设置窗口最小值;
    QRect windowRect = m_windowRectBeforeStretch;
    int delValue_X = m_startPoint.x() - m_endPoint.x();
    int delValue_Y = m_startPoint.y() - m_endPoint.y();
//    qDebug()<<m_startPoint<<m_endPoint;
    if (m_stretchRectState == LEFT_BORDER)
    {
        QPoint topLeftPoint = windowRect.topLeft();
        topLeftPoint.setX(topLeftPoint.x() - delValue_X);
        windowRect.setTopLeft(topLeftPoint);
        qDebug()<<windowRect.bottomRight();
    }
    else if (m_stretchRectState == RIGHT_BORDER)
    {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setX(bottomRightPoint.x() - delValue_X);
        windowRect.setBottomRight(bottomRightPoint);
        qDebug()<<windowRect.topLeft();
    }
    else if (m_stretchRectState == TOP_BORDER)
    {
        QPoint topLeftPoint = windowRect.topLeft();
        topLeftPoint.setY(topLeftPoint.y() - delValue_Y);
        windowRect.setTopLeft(topLeftPoint);
    }
    else if (m_stretchRectState == BOTTOM_BORDER)
    {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setY(bottomRightPoint.y() - delValue_Y);
        windowRect.setBottomRight(bottomRightPoint);
    }
    else if (m_stretchRectState == LEFT_TOP_RECT)
    {
        QPoint topLeftPoint = windowRect.topLeft();
        topLeftPoint.setX(topLeftPoint.x() - delValue_X);
        topLeftPoint.setY(topLeftPoint.y() - delValue_Y);
        windowRect.setTopLeft(topLeftPoint);
    }
    else if (m_stretchRectState == RIGHT_TOP_RECT)
    {
        QPoint topRightPoint = windowRect.topRight();
        topRightPoint.setX(topRightPoint.x() - delValue_X);
        topRightPoint.setY(topRightPoint.y() - delValue_Y);
        windowRect.setTopRight(topRightPoint);
    }
    else if (m_stretchRectState == RIGHT_BOTTOM_RECT)
    {
        QPoint bottomRightPoint = windowRect.bottomRight();
        bottomRightPoint.setX(bottomRightPoint.x() - delValue_X);
        bottomRightPoint.setY(bottomRightPoint.y() - delValue_Y);
        windowRect.setBottomRight(bottomRightPoint);
    }
    else if (m_stretchRectState == LEFT_BOTTOM_RECT)
    {
        QPoint bottomLeftPoint = windowRect.bottomLeft();
        bottomLeftPoint.setX(bottomLeftPoint.x() - delValue_X);
        bottomLeftPoint.setY(bottomLeftPoint.y() - delValue_Y);
        windowRect.setBottomLeft(bottomLeftPoint);
    }
    // 避免宽或高为零窗口显示有误，这里给窗口设置最小拉伸高度、宽度;
    if (windowRect.width() < m_windowMinWidth)
    {
        windowRect.setLeft(this->geometry().left());
        windowRect.setWidth(m_windowMinWidth);
    }
    if (windowRect.height() < m_windowMinHeight)
    {
        windowRect.setTop(this->geometry().top());
        windowRect.setHeight(m_windowMinHeight);
    }
    this->setGeometry(windowRect);
}

void MainWindow::on_reverse_button_clicked()
{
    ASongFFmpeg::getInstance()->invertPlay();
    ui->position_ctrl->isEnabled() ? ui->position_ctrl->setEnabled(false) : ui->position_ctrl->setEnabled(true);
    ui->forward_button->isEnabled() ? ui->forward_button->setEnabled(false) : ui->forward_button->setEnabled(true);
    ui->backward_button->isEnabled() ? ui->backward_button->setEnabled(false) : ui->backward_button->setEnabled(true);
}

