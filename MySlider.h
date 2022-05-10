#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>

class MySlider : public QSlider
{
public:
    //点击静音按钮之前的值
    int oldValue;

    MySlider(QWidget *parent = nullptr);
    ~MySlider();
    //任意移动进度条
    void mousePressEvent(QMouseEvent *ev);
    //音量条设静音
    void setZeroValue();
    //再点一次回到原音量
    void setOldValue();
};

#endif // MYSLIDER_H
