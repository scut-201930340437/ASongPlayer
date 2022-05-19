#ifndef MESSAGETITLEBAR_H
#define MESSAGETITLEBAR_H

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

class MessageTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit MessageTitleBar(QWidget *parent = nullptr, int width = 0, int height = 0);
    //实现常用鼠标事件
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    //实现鼠标事件需要的私有变量
    bool m_leftButtonPressed;
    QPoint m_start;

private slots:
    void maxSlot(QWidget *parent);

signals:

};

#endif // MESSAGETITLEBAR_H
