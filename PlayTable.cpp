#include "PlayTable.h"
#include "MyMessageWidget.h"

extern QTimer* myTimer;

PlayTable::PlayTable(QWidget *parent)
    : QTableWidget(parent)
{
    init();
}

void PlayTable::init()
{
    this->setRowCount(numFile);

    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    this->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    this->setSelectionMode(QAbstractItemView::SingleSelection);  //设置为只能选中单个目标
    this->resize(390,480);
    this->setColumnCount(1);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->setShowGrid(false);
    this->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
    this->setColumnWidth(0,this->size().width()-190);
    this->setMouseTracking(true);
    connect(this, &PlayTable::entered, this, &PlayTable::doMouseTrackTip);
}

void PlayTable::setTable(QList<QString> infoList,QString filePath)
{
    order_random.resize(infoList.size());
    random_order.resize(infoList.size());
    //顺序播放列表
    orderInfoList=infoList;
    this->clearContents();
    numFile=infoList.size();
    this->setRowCount(numFile);
    for(qint16 i=0;i<numFile;i++)
    {
        this->setItem(i,0,new QTableWidgetItem(getFileNameFromPath(infoList[i])));
//        this->setItem(i,1,new QTableWidgetItem(infoList[i]));
    }

    for(int i=0;i<orderInfoList.size();i++)
    {
        if(orderInfoList[i]==filePath)
        {
            playPos=i;
            showHighLight(0,playPos);
        }
    }
    //生成随机列表
    generateRandomList();
    randomPos=order_random[playPos];
}

void PlayTable::generateRandomList()
{
    //根据orderInfoLst生成RandomList
    numFile=orderInfoList.size();
//    order_random.resize(numFile);
//    random_order.resize(numFile);
    //记录原来的位置
    for(qint16 i=0;i<numFile;i++)
    {
        random_order[i]=i;
    }
    quint32 seed = quint32(QDateTime::currentDateTime().toSecsSinceEpoch());
    QRandomGenerator generator(seed);
    for(qint16 i=0;i<numFile;i++)
    {
        int rand = generator.bounded(i, numFile);
        qSwap(random_order[i],random_order[rand]);
    }
    for(qint16 i=0;i<numFile;i++)
    {
        order_random[random_order[i]]=i;
    }
    randomPos=order_random[playPos];
}

bool PlayTable::isNeededFile(QFileInfo file)
{
    QString suffix=file.suffix();
    for(QString str:neededFile)
    {
        if(suffix==str)
            return true;
    }
    return false;
}

QString PlayTable::getPath(qint16 row)
{
    if(row>=orderInfoList.size())
        return "";
    return this->orderInfoList[row];
}

QString PlayTable::getPrevFile()
{
    qint16 n=orderInfoList.size();
    qint16 pre=-1;
    if(playMode==2)
    {
        pre=random_order[randomPos];
        randomPos=(randomPos-1+n)%n;
        playPos=random_order[randomPos];
    }
    else
    {
        pre=playPos;
        playPos=(playPos-1+n)%n;
        randomPos=order_random[playPos];
    }
    this->showHighLight(pre,playPos);
    return orderInfoList[playPos];
}

QString PlayTable::getNextFile()
{
    qint16 n=orderInfoList.size();
    qint16 pre=-1;
    if(playMode==2)
    {
        pre=random_order[randomPos];
        randomPos=(randomPos+1)%n;
        playPos=random_order[randomPos];
    }
    else
    {
        pre=playPos;
        playPos=(playPos+1)%n;
        randomPos=order_random[playPos];

    }
    this->showHighLight(pre,playPos);
    return orderInfoList[playPos];
}

qint16 PlayTable::getNumFiles()
{
    return orderInfoList.size();
}

void PlayTable::showHighLight(qint16 pre,qint16 cur)
{
    for(int i=0;i<1;i++)
    {
        this->item(pre,i)->setBackground(QBrush(this->backgroundColor));
        this->item(cur,i)->setBackground(QBrush(this->onPlayingColor));
    }

}

void PlayTable::showMessage()
{
    int rowIndex = this->currentRow();
    if (rowIndex != -1)
    {
        QString path=orderInfoList[rowIndex];
        MediaMetaData * mediaMetaData=ASongFFmpeg::getInstance()->openMediaInfo(path);
        MyMessageWidget * myMessageWidget = new MyMessageWidget(mediaMetaData);
        myMessageWidget->show();

    }
}
void PlayTable::deleteFile()
{
    //保证playPos和randomPos，还有两种映射的正确
    int orderDltRowIndex = this->currentRow();
    int randomDltRowIndex = order_random[orderDltRowIndex];
    if (orderDltRowIndex != -1)
    {   //删除顺序列表，随机列表，两个哈希的相应映射对
        this->removeRow(orderDltRowIndex);
        orderInfoList.removeAt(orderDltRowIndex);
        order_random.removeAt(orderDltRowIndex);
        random_order.removeAt(randomDltRowIndex);
        //修正哈希映射
        for(int i=0;i<orderInfoList.size();i++)
        {
            if(random_order[i]>=orderDltRowIndex)
                random_order[i]--;
        }
        for(int i=0;i<orderInfoList.size();i++)
        {
            if(order_random[i]>=randomDltRowIndex)
                order_random[i]--;
        }
    }

    if(playPos>orderDltRowIndex)
    {
        playPos--;
    }
    randomPos=order_random[playPos];
}

QString PlayTable::getFileNameFromPath(QString path)
{
    return path.section('/', -1, -1);
}

void PlayTable::doMouseTrackTip(QModelIndex index)
{
    QToolTip::showText(QCursor::pos(), orderInfoList[index.row()]);
}

void PlayTable::addFilePath(QString filepath)
{
    for(QString str:orderInfoList)
    {
        if(str==filepath)
            return;
    }
    orderInfoList.append(filepath);
    setTable(orderInfoList,filepath);
}
