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
    int batten_num;
    float batten_width;
    float interval;
    float *batten_height;
    int max_height;
    //时间
    QTime randtime;
    explicit MyPlayWidget(QWidget *parent = nullptr);

    void test();

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);

signals:

};

#endif // MYPLAYWIDGET_H
