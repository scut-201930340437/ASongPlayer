#include "MyMessageWidget.h"
#include "MessageTitleBar.h"
#include <QtGlobal>

int MyMessageWidget::TitleHeight=40;
int MyMessageWidget::ImageSize=150;
int MyMessageWidget::WindowWidth=480;
int MyMessageWidget::WindowHeight=700;
int MyMessageWidget::Indentation=ImageSize/4;

MyMessageWidget::MyMessageWidget(QWidget *parent)
    : QWidget(parent)
{
    setTitleBar(200,120);
    this->resize(200,120);
    this->setWindowTitle("information");
    QLabel *label = new QLabel(this);
    label->resize(180,80);
    label->move(10,this->height()/3);
    label->setText("音视频文件不存在");
}

MyMessageWidget::MyMessageWidget(MediaMetaData *mediaMetaData,QWidget *parent)
    : QWidget(parent)
{
    setTitleBar(WindowWidth,WindowHeight);
    this->setFixedSize(WindowWidth,WindowHeight);
    _mediaMetaData=mediaMetaData;
    //封面
    QLabel *label = new QLabel(this);
    label->setScaledContents(true);
    label->resize(ImageSize,ImageSize);
    label->move(ImageSize/4,TitleHeight*1.2);

    if(mediaMetaData->vMetaDatas!=nullptr && mediaMetaData->vMetaDatas->cover!=nullptr)
    {
        label->setPixmap(QPixmap::fromImage(*mediaMetaData->vMetaDatas->cover));//在label显示图片
    }
    else
    {
        QString filename = ":/img/defaultCover.jpg";
        QImage image(filename);
        qDebug()<<image;
        label->setPixmap(QPixmap::fromImage(image));
    }

    //文件名
    QLabel *nameLabel = new QLabel(this);
    nameLabel->move(ImageSize*1.4,TitleHeight*1.5);
    nameLabel->setText(mediaMetaData->filename);
    nameLabel->resize(WindowWidth-nameLabel->x(),100);
    nameLabel->setWordWrap(true);

    //信息树
    QTreeWidget *treeWidget=new QTreeWidget(this);
    treeWidget->resize(WindowWidth-2*Indentation,WindowHeight-TitleHeight*3-ImageSize);
    treeWidget->move(Indentation,TitleHeight*2.2+ImageSize);
    treeWidget->setHeaderLabels(QStringList()<<"1"<<"2");
    //隐藏表头
    treeWidget->setHeaderHidden(true);
    treeWidget->setColumnWidth(0,130);

    //基本信息
    QTreeWidgetItem *basicItem = new QTreeWidgetItem(QStringList()<<"基本信息");
    treeWidget->addTopLevelItem(basicItem);


        QFileInfo fileInfo(mediaMetaData->path);
        QTreeWidgetItem *item11 = new QTreeWidgetItem(QStringList()<<"文件类型"<<fileInfo.suffix());
        QTreeWidgetItem *item12 = new QTreeWidgetItem(QStringList()<<"专辑信息"<<mediaMetaData->album);
        QTreeWidgetItem *item13 = new QTreeWidgetItem(QStringList()<<"艺术家"<<mediaMetaData->artist);
        QTreeWidgetItem *item14 = new QTreeWidgetItem(QStringList()<<"文件时长"<<QString::number(mediaMetaData->durationSec)+" s");
        QTreeWidgetItem *item15 = new QTreeWidgetItem(QStringList()<<"文件路径"<<mediaMetaData->path);
        basicItem->addChildren({item11,item12,item13,item14,item15});

    for(int i=0;i<mediaMetaData->nb_vstreams;i++)
    {
        //视频流信息
        QTreeWidgetItem *viedoItem = new QTreeWidgetItem(QStringList()<<"视频流信息");
        treeWidget->addTopLevelItem(viedoItem);
            QTreeWidgetItem *item1 = new QTreeWidgetItem(QStringList()<<"视频编码"<<avcodec_get_name((mediaMetaData->vMetaDatas+i)->codec_id));
            qint64 _kbps=(mediaMetaData->vMetaDatas+i)->bit_rate/1000>0?(mediaMetaData->vMetaDatas+i)->bit_rate/1000:0;
            QTreeWidgetItem *item2 = new QTreeWidgetItem(QStringList()<<"视频码率"<<QString::number(_kbps)+" kbps");
            double _fps=(mediaMetaData->vMetaDatas+i)->frame_rate>0?(mediaMetaData->vMetaDatas+i)->frame_rate:0;
            QTreeWidgetItem *item3 = new QTreeWidgetItem(QStringList()<<"视频帧率"<<QString::number(_fps)+" fps");
            QTreeWidgetItem *item4 = new QTreeWidgetItem(QStringList()<<"分辨率"<<QString::number((mediaMetaData->vMetaDatas+i)->width)+"*"+QString::number(mediaMetaData->vMetaDatas->height));
            viedoItem->addChildren({item1,item2,item3,item4});
    }
    for(int i=0;i<mediaMetaData->nb_astreams;i++)
    {
        //音频流信息
        QTreeWidgetItem *audioItem = new QTreeWidgetItem(QStringList()<<"音频流信息");
        treeWidget->addTopLevelItem(audioItem);
            QTreeWidgetItem *item1 = new QTreeWidgetItem(QStringList()<<"音频编码"<<avcodec_get_name((mediaMetaData->aMetaDatas+i)->codec_id));
//            qint64 _kbps=(mediaMetaData->aMetaDatas+i)->bit_rate/1000>0?(mediaMetaData->aMetaDatas+i)->bit_rate/1000:0;
            QTreeWidgetItem *item2 = new QTreeWidgetItem(QStringList()<<"音频码率"<<QString::number((mediaMetaData->aMetaDatas+i)->bit_rate/1000)+" kbps");
            QTreeWidgetItem *item3 = new QTreeWidgetItem(QStringList()<<"声道数"<<QString::number((mediaMetaData->aMetaDatas+i)->nb_channels)+" channels");
            QTreeWidgetItem *item4 = new QTreeWidgetItem(QStringList()<<"采样数"<<QString::number((mediaMetaData->aMetaDatas+i)->sample_rate)+" HZ");
            audioItem->addChildren({item1,item2,item3,item4});
    }

    //设置展开
    treeWidget->expandAll();
    ASongFFmpeg::getInstance()->closeMediaInfo(_mediaMetaData);
}

void MyMessageWidget::setTitleBar(int width, int height)
{
    QWidget *father = new QWidget(this);  //历史遗留问题，顺便解决无法搞样式的问题
    father->setStyleSheet("background:black");  //标题黑色
//    this->setStyleSheet("background:grey");
    this->setWindowFlags(Qt::FramelessWindowHint);  // 去掉标题栏
//    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    MessageTitleBar *title = new MessageTitleBar(father, width, height);
    title->resize(width, TitleHeight);
    title->move(0,0);
}



