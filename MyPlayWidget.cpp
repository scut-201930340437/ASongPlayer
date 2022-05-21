#include "MyPlayWidget.h"
#include "ASongFFmpeg.h"
#include "ASongAudioOutput.h"

extern QTimer* myTimer;

MyPlayWidget::MyPlayWidget(QWidget *parent)
    : QWidget{parent}
{
//    batten_num = 10;
//    max_height = this->size().height() / 2;
//    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
//    batten_width = interval * 2;
//    batten_height = new float[batten_num];
//    for(int i = 0; i < batten_num; ++i)
//    {
//        batten_height[i] = max_height;
//    }
//    connect(myTimer, &QTimer::timeout, this, &MyPlayWidget::test);
    connect(ASongAudioOutput::getInstance(), SIGNAL(playAudio(const char*,int)),
            this, SLOT(waveDraw(const char*,int)));
}

void MyPlayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    //蓝色
//    painter.setBrush(Qt::blue);
//    for(int i = 0; i < batten_num; ++i)
//    {
//        painter.drawRect((i + 1) * interval + i * batten_width, this->size().height() / 2 - batten_height[i],
//                         batten_width, batten_height[i]);
//    }
//    //红色
//    painter.setBrush(Qt::red);
//    for(int i = 0; i < batten_num; ++i)
//    {
//        painter.drawRect((i + 1) * interval + i * batten_width, this->size().height() / 2,
//                         batten_width, batten_height[i]);
//    }
}
void MyPlayWidget::waveDraw(const char *outBuffer,int sample_nb)
{
//    if(ASongFFmpeg::getInstance()->getMediaStatus() > 0) return;
//    //数据更新
//    max_height = this->size().height() / 2;
//    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
//    batten_width = interval * 2;
//    randtime=QTime::currentTime();
//    srand(randtime.msec()+randtime.second()*1000); //以当前时间ms为随机种子
//    for(int i = 0; i < batten_num; ++i)
//    {
//        int n = rand() % max_height;    //产生100以内的随机整数
//        if(n % 2 == 0) n*=-1;  //偶数反转
//        batten_height[i] = n;
//    }
    //绘图更新
    qDebug()<<sample_nb;
    update();
}
void MyPlayWidget::mouseMoveEvent(QMouseEvent *event)
{
//    if(a>=this->size().width()||a<=0)
//        x=-x;
//    a+=x;
//    update();
//    qDebug()<<event->pos();
//    emit signalsTest(1);
}
