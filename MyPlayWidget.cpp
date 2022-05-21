#include "MyPlayWidget.h"
#include "ASongFFmpeg.h"
#include "ASongAudioOutput.h"

extern QTimer* myTimer;

MyPlayWidget::MyPlayWidget(QWidget *parent)
    : QWidget{parent}
{
    batten_num = 115;
    max_height = this->size().height() / 2;
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    batten_height = new float[batten_num];
    batten_height2 = new float[batten_num];
    for(int i = 0; i < batten_num; ++i)
    {
        batten_height[i] = max_height;
        batten_height2[i] = max_height / 2;
    }
    batten_contain = 1;
    mul = 1;
    max_num = 0;
//    connect(myTimer, &QTimer::timeout, this, &MyPlayWidget::test);

    connect(ASongAudioOutput::getInstance(), SIGNAL(playAudio(const char*,int)),
            this, SLOT(waveDraw(const char*,int)));
}

void MyPlayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    //柱状图
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
//                         batten_width, batten_height2[i]);
//    }
    //线性图
    //蓝色
    painter.setPen(Qt::blue);
    for(int i = 0; i < batten_num - 1; ++i)
    {
        painter.drawLine(i * this->size().width() / batten_num, this->size().height() / 2 - batten_height[i],
                         (i + 1) * this->size().width() / batten_num, this->size().height() / 2 - batten_height[i + 1]);
    }
    //红色
    painter.setPen(Qt::red);
    for(int i = 0; i < batten_num; ++i)
    {
        painter.drawLine(i * this->size().width() / batten_num, this->size().height() / 2 + batten_height2[i],
                         (i + 1) * this->size().width() / batten_num, this->size().height() / 2 + batten_height2[i + 1]);
    }
    //中线
    painter.setPen(Qt::black);
    painter.drawLine(0,this->size().height() / 2,this->size().width(),this->size().height() / 2);
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

    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
        return;
    //数据样式更新
    max_height = this->size().height() / 2;
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    //数据大小更新
    int i = 0,j = 0;
    while(i < batten_num && j < batten_num)
    {
        if(((short*)outBuffer)[i+j] > 0)
        {
            max_num = max_num > ((short*)outBuffer)[i+j] ? max_num : ((short*)outBuffer)[i+j];
            mul = 1.0 * max_height / max_num;
            batten_height[i] = ((short*)outBuffer)[i+j] * mul;
            ++i;
        }
        else
        {
            max_num = max_num > -((short*)outBuffer)[i+j] ? max_num : -((short*)outBuffer)[i+j];
            mul = 1.0 * max_height / max_num;
            batten_height2[j] = -((short*)outBuffer)[i+j] * mul;
            ++j;
        }
    }
//    qDebug()<<i<<j<<this->size().height();
    //绘图更新
    update();
    //释放内存
    free((void*)outBuffer);
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

void MyPlayWidget::test()
{
        if(ASongFFmpeg::getInstance()->getMediaStatus() > 0) return;
        //数据更新
        max_height = this->size().height() / 2;
        interval = 1.0 * this->size().width() / (3 * batten_num + 1);
        batten_width = interval * 2;
        randtime=QTime::currentTime();
        srand(randtime.msec()+randtime.second()*1000); //以当前时间ms为随机种子
        for(int i = 0; i < batten_num; ++i)
        {
            int n = rand() % max_height;    //产生100以内的随机整数
            if(n % 2 == 0) n*=-1;  //偶数反转
            batten_height[i] = n;
        }
        //绘图更新
        update();
}
