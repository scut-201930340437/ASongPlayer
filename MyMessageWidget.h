#ifndef MYMESSAGEWIDGET_H
#define MYMESSAGEWIDGET_H

#include <QAbstractItemModel>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QLineF>
#include <QTreeWidget>
#include <QDir>
//#include <QGraphicsLineItem>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#include "ASongFFmpeg.h"
class MyMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyMessageWidget(QWidget *parent = nullptr);
    //是否存在内存泄漏？TreeWidget？mediaMetaData？
    explicit MyMessageWidget(MediaMetaData *mediaMetaData, QWidget *parent = nullptr);

    void setTitleBar(int width, int height);

private:
    static int TitleHeight;
    static int ImageSize;
    static int WindowWidth;
    static int WindowHeight;
    static int Indentation;
    MediaMetaData *_mediaMetaData;
};


#endif // MYMESSAGEWIDGET_H
