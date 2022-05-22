#ifndef MYPLAYWIDGET_H
#define MYPLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMouseEvent>
#include <QPushButton>


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
    // 波形图模式 1，线性图 2，柱状图
    int wave_mode;
    explicit MyPlayWidget(QWidget *parent = nullptr);

    //改变波形图
    void changeWaveMode();


protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);

signals:

private slots:
    void waveDraw(const char *outBuffer,int sample_nb);
    void setTitleBar(int width, int height);

};

#endif // MYPLAYWIDGET_H
