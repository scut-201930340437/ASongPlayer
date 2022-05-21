#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>
#include <QTimer>
#include <QLabel>
class MySlider : public QSlider
{
public:
    //点击静音按钮之前的值
    int oldValue;
    //进度条的旧的位置，判断是否更新预览图
    int oldPosition = 0;

    MySlider(QWidget *parent = nullptr);
    ~MySlider();
    //任意移动进度条
    void mousePressEvent(QMouseEvent *ev);
    //预览窗口鼠标事件
    void mouseMoveEvent (QMouseEvent *event);
    //鼠标进入
    void enterEvent(QEnterEvent *);
    //鼠标离开
    void leaveEvent(QEvent *);
    //音量条设静音
    void setZeroValue();
    //再点一次回到原音量
    void setOldValue();
private:
    //预览窗口
    QWidget *preview = nullptr;
    QLabel *label;
    //停留2秒就可以显示
    int sustain;
    int stay = 4;
};

#endif // MYSLIDER_H
