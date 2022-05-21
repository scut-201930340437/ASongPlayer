#include "MySlider.h"
#include<QDebug>
#include<QMouseEvent>
#include <VideoPreview.h>
#include <ASongFFmpeg.h>

extern QTimer* myTimer;

MySlider::MySlider(QWidget *parent):QSlider (parent)
{
    sustain = 0;
     connect(myTimer, &QTimer::timeout, this, [&](){
         sustain = sustain+1<stay?sustain+1:stay;
     });
     preview = new QWidget(this->parentWidget()->parentWidget()->parentWidget());//排到曾祖父那边了
     preview->resize(165,120);
     label=new QLabel(preview);
     label->resize(preview->size());
     preview->hide();
}

MySlider::~MySlider()
{

}

void MySlider::mousePressEvent(QMouseEvent *ev)
{
    //获取当前点击位置,得到的这个鼠标坐标是相对于当前QSlider的坐标
    int currentX = ev->pos().x();
//    qDebug()<<ev->pos()<<this->size();

    //获取当前点击的位置占整个Slider的百分比
    double per = currentX *1.0 /this->width();

    //利用算得的百分比得到具体数字
    int value = per*(this->maximum() - this->minimum()) + this->minimum();

//    qDebug() << value;

    //设定滑动条位置
    this->setValue(value);

    //滑动条移动事件等事件也用到了mousePressEvent,加这句话是为了不对其产生影响，是的Slider能正常相应其他鼠标事件
    QSlider::mousePressEvent(ev);
}

void MySlider::mouseMoveEvent(QMouseEvent *event)
{
    if(sustain >= stay)
    {
        //获取当前点击位置,得到的这个鼠标坐标是相对于当前QSlider的坐标
        int currentX = event->pos().x();
        //获取当前点击的位置占整个Slider的百分比
        double per = currentX *1.0 /this->width();
        int posSec = ASongFFmpeg::getInstance()->getDuration() * per;

        //获得预览图
        VideoPreview::getInstance()->setPosSec(posSec);
        QImage img = VideoPreview::getInstance()->getPreviewImg();
        if(!img.isNull())
        {
            QPixmap pixmap = QPixmap::fromImage(img);
            pixmap = pixmap.scaled(preview->width(), preview->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            label->setPixmap(pixmap);//在label显示图片
            //位置偏差
            QPoint *p = new QPoint(event->pos().x() - 0.5*preview->size().width(),-1*preview->size().height());
            if(this->parentWidget()->parentWidget()->pos().x() + this->parentWidget()->pos().x() + this->pos().x()+ event->pos().x() - 0.5*preview->size().width()>0)
            {
                preview->move(this->parentWidget()->parentWidget()->pos() + this->parentWidget()->pos() + this->pos() + *p);
            }
            else
            {
                preview->move(0,(this->parentWidget()->parentWidget()->pos() + this->parentWidget()->pos() + this->pos() + *p).y());
            }
            preview->setStyleSheet("background:black");
            preview->show();
        }

    }
    //回调
    QSlider::mouseMoveEvent(event);
}

void MySlider::enterEvent(QEnterEvent *){
    sustain = 0;
}

void MySlider::leaveEvent(QEvent *)
{
    if(preview!=nullptr)
        preview->hide();
}

void MySlider::setZeroValue(){
    this->oldValue = this->value();
    this->setValue(0);
}

void MySlider::setOldValue(){
    this->setValue(this->oldValue);
    this->oldValue = 0;
}
