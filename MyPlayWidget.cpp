#include "MyPlayWidget.h"
#include "ASongFFmpeg.h"

extern QTimer* myTimer;

MyPlayWidget::MyPlayWidget(QWidget *parent)
    : QWidget{parent}
{
    batten_num = 10;
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    batten_height = new float[batten_num];
    for(int i = 0; i < batten_num; ++i)
    {
        batten_height[i] = 100;
    }
    connect(myTimer, &QTimer::timeout, this, &MyPlayWidget::test);
}

void MyPlayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::blue);
    for(int i = 0; i < batten_num; ++i)
    {
        painter.drawRect((i + 1) * interval + i * batten_width, this->size().height() / 2 - batten_height[i],
                         batten_width, batten_height[i]);
    }
}
void MyPlayWidget::test()
{
    if(ASongFFmpeg::getInstance()->getMediaStatus() > 0) return;
    //数据更新
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    qDebug()<<interval<<batten_width<<this->size();
    //绘图更新
    update();
}
void MyPlayWidget::mouseMoveEvent(QMouseEvent *event)
{
//    if(a>=this->size().width()||a<=0)
//        x=-x;
//    a+=x;
//    update();
//    qDebug()<<event->pos();
}
