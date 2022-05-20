#ifndef MYPLAYWIDGET_H
#define MYPLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMouseEvent>

class MyPlayWidget : public QWidget
{
    Q_OBJECT
public:
    int batten_num;
    float batten_width;
    float interval;
    float *batten_height;
    explicit MyPlayWidget(QWidget *parent = nullptr);

    void test();

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);

signals:

};

#endif // MYPLAYWIDGET_H
