#include "MyTitleBar.h"

MyTitleBar::MyTitleBar(QWidget *parent)
    : QWidget{parent}
{

}

void MyTitleBar::mousePressEvent(QMouseEvent *event)
{
    // 鼠标左键按下事件
    if (event->button() == Qt::LeftButton)
    {
        // 记录鼠标左键状态
        m_leftButtonPressed = true;
        //记录鼠标在屏幕中的位置
        m_start = event->globalPos();
    }
}

void MyTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    //最大化时就不要动了
    if(parentWidget()->parentWidget()->isMaximized()) return;
    // 持续按住才做对应事件
    if(m_leftButtonPressed)
    {
        //将父窗体(centralWidget)的父窗体(mainWindow)移动到父窗体原来的位置加上鼠标移动的位置：event->globalPos()-m_start
        parentWidget()->parentWidget()->move(parentWidget()->parentWidget()->geometry().topLeft() +
                             event->globalPos() - m_start);

        //将鼠标在屏幕中的位置替换为新的位置
        m_start = event->globalPos();
    }
}

void MyTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if (event->button() == Qt::LeftButton)
    {
        // 记录鼠标状态
        m_leftButtonPressed = false;
    }
}

void MyTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(parentWidget()->parentWidget()->isMaximized())
    {
        parentWidget()->parentWidget()->showNormal();
    }
    else
    {
        parentWidget()->parentWidget()->showMaximized();
    }
}
