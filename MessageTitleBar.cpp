#include "MessageTitleBar.h"
#include "MyMessageWidget.h"

MessageTitleBar::MessageTitleBar(QWidget *parent, int width, int height)
    : QWidget{parent}
{
    QPushButton *close = new QPushButton(this);
//    QPushButton *max = new QPushButton(this);
    QPushButton *min = new QPushButton(this);
    close->setObjectName(QString::fromUtf8("close"));
//    max->setObjectName(QString::fromUtf8("max"));
    min->setObjectName(QString::fromUtf8("min"));
    close->resize(25,25);
//    max->resize(25,25);
    min->resize(25,25);
    close->move(width-25,5);
//    max->move(width-50,5);
    min->move(width-50,5);
    close->setStyleSheet("#close{\
                               image: url(:/img/close.png);\
                           }\
                           #close::hover{\
                               image: url(:/img/close2.png);\
                           }");

//    max->setStyleSheet("#max{\
//                               image: url(:/img/maximize.png);\
//                           }\
//                           #max::hover{\
//                               image: url(:/img/maximize2.png);\
//                           }");
    min->setStyleSheet("#min{\
                               image: url(:/img/minimize.png);\
                           }\
                           #min::hover{\
                               image: url(:/img/minimize2.png);\
                           }");
    //爸爸的爸爸叫爷爷
    connect(close,&QPushButton::clicked,parent->parentWidget(),&MyMessageWidget::close);
//    connect(max,&QPushButton::clicked,parent->parentWidget(),&MyMessageWidget::maxSlot);
    connect(min,&QPushButton::clicked,parent->parentWidget(),&MyMessageWidget::showMinimized);
}

void MessageTitleBar::mousePressEvent(QMouseEvent *event)
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

void MessageTitleBar::mouseMoveEvent(QMouseEvent *event)
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

void MessageTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if (event->button() == Qt::LeftButton)
    {
        // 记录鼠标状态
        m_leftButtonPressed = false;
    }
}

void MessageTitleBar::maxSlot(QWidget *parent)
{
    parent->isMaximized() ? parent->showNormal() : parent->showMaximized();
}
