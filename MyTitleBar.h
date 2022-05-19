#ifndef MYTITLEBAR_H
#define MYTITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QPoint>
#include <QPixmap>
#include <QString>

//很重要！TitleBar的爷爷才是MainWindow！！！！！！！！！
class MyTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit MyTitleBar(QWidget *parent = nullptr);
    //实现常用鼠标事件
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *ev);

private:
    //实现鼠标事件需要的私有变量
    bool m_leftButtonPressed;
    QPoint m_start;
signals:

};

#endif // MYTITLEBAR_H
