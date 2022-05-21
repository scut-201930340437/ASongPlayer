#ifndef MYPLAYWIDGET_H
#define MYPLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMouseEvent>


class MyPlayWidget : public QWidget
{
    Q_OBJECT
public:
    //固定属性
    int batten_num;
    float batten_width;
    float interval;
    float *batten_height;
    float *batten_height2;
    int max_height;
    //可变属性
    int batten_contain;  //一根柱子代表多少采样点
    int max_num;  //读出来的最大的采样点的值
    float mul;  //缩放倍数
    //时间
    QTime randtime;
    explicit MyPlayWidget(QWidget *parent = nullptr);


protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);

signals:

private slots:
    void waveDraw(const char *outBuffer,int sample_nb);
    void test();

};

#endif // MYPLAYWIDGET_H
