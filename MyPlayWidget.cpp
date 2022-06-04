#include "MyPlayWidget.h"
#include "ASongFFmpeg.h"
#include "ASongAudioOutput.h"
#include "MessageTitleBar.h"

extern QTimer* myTimer;

MyPlayWidget::MyPlayWidget(QWidget *parent)
    : QWidget{parent}
{
    setTitleBar(300, 600);
    wave_mode = 2;
    batten_num = 500;  //默认线性图
    max_height = this->size().height() / 2;
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    batten_height = new float[batten_num];
    batten_height2 = new float[batten_num];
    for(int i = 0; i < batten_num + 2; ++i)
    {
        batten_height[i] = max_height;
        batten_height2[i] = max_height;
    }
    batten_contain = 1;
    mul = 1;
    max_num = 0;
    //    connect(myTimer, &QTimer::timeout, this, &MyPlayWidget::test);
    connect(ASongAudioOutput::getInstance(), SIGNAL(playAudio(const char*, int)),
            this, SLOT(waveDraw(const char*, int)));
}

void MyPlayWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(wave_mode == 1)
    {
        //柱状图
        batten_num = 20;
        //蓝色
        painter.setBrush(Qt::red);
        for(int i = 0; i < batten_num; ++i)
        {
            painter.drawRect((i + 1) * interval + i * batten_width, this->size().height() / 2 - batten_height[i],
                             batten_width, batten_height[i]);
        }
        //红色
        painter.setBrush(Qt::blue);
        for(int i = 0; i < batten_num; ++i)
        {
            painter.drawRect((i + 1) * interval + i * batten_width, this->size().height() / 2,
                             batten_width, batten_height2[i]);
        }
    }
    if(wave_mode == 2)
    {
        //线性图
        batten_num = 500;
        //绿色
        painter.setPen(Qt::green);
        for(int i = 0; i < batten_num - 1; ++i)
        {
            painter.drawLine(i * this->size().width() / batten_num, this->size().height() / 2 - batten_height[i],
                             (i + 1) * this->size().width() / batten_num, this->size().height() / 2 - batten_height[i + 1]);
        }
        //黄色
        painter.setPen(Qt::yellow);
        for(int i = 0; i < batten_num - 1; ++i)
        {
            painter.drawLine(i * this->size().width() / batten_num, this->size().height() / 2 + batten_height2[i],
                             (i + 1) * this->size().width() / batten_num, this->size().height() / 2 + batten_height2[i + 1]);
        }
    }
    //中线
    painter.setPen(Qt::white);
    painter.drawLine(0, this->size().height() / 2, this->size().width(), this->size().height() / 2);
}
void MyPlayWidget::waveDraw(const char *outBuffer, int sample_nb)
{
    if(ASongFFmpeg::getInstance()->getMediaStatus() <= 0)
    {
        return;
    }
    //数据样式更新
    max_height = this->size().height() / 2;
    interval = 1.0 * this->size().width() / (3 * batten_num + 1);
    batten_width = interval * 2;
    //数据大小更新
    if(sample_nb < batten_num)
    {
        return;    //太少不要
    }
    batten_contain = sample_nb / batten_num;
    int left = 0, right = 0, sum = 0;
    //    qDebug()<<batten_contain<<sample_nb<<batten_num;
    while(sum < batten_num)
    {
        if(((short*)outBuffer)[left + right] > 0)
        {
            max_num = max_num > ((short*)outBuffer)[left + right] ? max_num : ((short*)outBuffer)[left + right];
            mul = 1.0 * max_height / max_num;
            batten_height[sum] = ((short*)outBuffer)[left + right] * mul;
            left += batten_contain;
        }
        else
        {
            max_num = max_num > -((short*)outBuffer)[left + right] ? max_num : -((short*)outBuffer)[left + right];
            mul = 1.0 * max_height / max_num;
            batten_height2[sum] = -((short*)outBuffer)[left + right] * mul;
            right += batten_contain;
        }
        sum++;
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

void MyPlayWidget::setTitleBar(int width, int height)
{
    QWidget *father = new QWidget(this);  //历史遗留问题，顺便解决无法搞样式的问题
    father->setStyleSheet("background:black");  //标题黑色
    //    this->setStyleSheet("background:grey");
    this->setWindowFlags(Qt::FramelessWindowHint);  // 去掉标题栏
    //    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    MessageTitleBar *title = new MessageTitleBar(father, width, height);
    title->resize(width, 40);
    title->move(0, 0);
}

void MyPlayWidget::changeWaveMode()
{
    wave_mode = wave_mode == 1 ? 2 : 1;
}
