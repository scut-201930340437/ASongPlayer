#include "PlayTable.h"
#include "MyMessageWidget.h"

extern QTimer* myTimer;

QVector<QString> PlayTable::neededFile={"mp3","mp4","flv","avi","mkv"};
QColor PlayTable::backgroundColor=QColor(68, 65, 63);
QColor PlayTable::selectColor=QColor(158, 158, 252);
QColor PlayTable::onPlayingColor=QColor(224, 224, 226);

PlayTable::PlayTable(QWidget *parent)
    : QTableWidget(parent)
{
    init();
}

void PlayTable::init()
{
    this->setRowCount(0);

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

void PlayTable::setTable(QList<QString> infoList,QString &filePath)
{
    order_random.resize(infoList.size());
    random_order.resize(infoList.size());
    //顺序播放列表
    orderList=infoList;
    this->clearContents();
    qint16 numFile=infoList.size();
    this->setRowCount(numFile);
    for(qint16 i=0;i<numFile;i++)
    {
        this->setItem(i,0,new QTableWidgetItem(getFileNameFromPath(infoList[i])));
//        this->setItem(i,1,new QTableWidgetItem(infoList[i]));
    }
    if(filePath=="")
    {
        filePath=orderList[0];
    }
    for(int i=0;i<orderList.size();i++)
    {
        if(orderList[i]==filePath)
        {
            orderPos=i;
            showHighLight(0,orderPos);
        }
    }
    //生成随机列表
    generateRandomList();
    randomPos=order_random[orderPos];
}

void PlayTable::generateRandomList()
{
    //根据orderInfoLst生成随机序列
    qint16 numFile=orderList.size();

    if(numFile==0)
        return;

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
    randomPos=order_random[orderPos];
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
    if(row>=orderList.size())
        return "";
    return this->orderList[row];
}

QString PlayTable::getPrevFile()
{
    qint16 n=orderList.size();
    qint16 pre=-1;
    if(playMode==2)
    {
        pre=random_order[randomPos];
        randomPos=(randomPos-1+n)%n;
        orderPos=random_order[randomPos];
    }
    else
    {
        pre=orderPos;
        orderPos=(orderPos-1+n)%n;
        randomPos=order_random[orderPos];
    }
    this->showHighLight(pre,orderPos);
    return orderList[orderPos];
}

QString PlayTable::getNextFile()
{
    qint16 n=orderList.size();
    qint16 pre=-1;
    if(playMode==2)
    {
        pre=random_order[randomPos];
        randomPos=(randomPos+1)%n;
        orderPos=random_order[randomPos];
    }
    else
    {
        pre=orderPos;
        orderPos=(orderPos+1)%n;
        randomPos=order_random[orderPos];

    }
    this->showHighLight(pre,orderPos);
    return orderList[orderPos];
}

qint16 PlayTable::getNumFiles()
{
    return orderList.size();
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
        QString path=orderList[rowIndex];
        MediaMetaData * mediaMetaData=ASongFFmpeg::getInstance()->openMediaInfo(path);
        MyMessageWidget * myMessageWidget = new MyMessageWidget(mediaMetaData);
        myMessageWidget->show();
        allCloseWidget.append(myMessageWidget);
    }
}
void PlayTable::deleteFile()
{
    //保证orderPos和randomPos，还有两种映射的正确
    int orderDltRowIndex = this->currentRow();
    if (orderDltRowIndex != -1)
    {
        int randomDltRowIndex = order_random[orderDltRowIndex];
        //删除顺序列表，随机列表，两个哈希的相应映射对
        this->removeRow(orderDltRowIndex);
        orderList.removeAt(orderDltRowIndex);
        order_random.removeAt(orderDltRowIndex);
        random_order.removeAt(randomDltRowIndex);
        //修正哈希映射
        for(int i=0;i<orderList.size();i++)
        {
            if(random_order[i]>=orderDltRowIndex)
                random_order[i]--;
        }
        for(int i=0;i<orderList.size();i++)
        {
            if(order_random[i]>=randomDltRowIndex)
                order_random[i]--;
        }
        if(orderPos>orderDltRowIndex)
        {
            orderPos--;
        }
        randomPos=order_random[orderPos];
    }
}

QString PlayTable::getFileNameFromPath(QString path)
{
    return path.section('/', -1, -1);
}

void PlayTable::doMouseTrackTip(QModelIndex index)
{
    QToolTip::showText(QCursor::pos(), orderList[index.row()]);
}

void PlayTable::addFilePath(QString filepath)
{
    for(QString str:orderList)
    {
        if(str==filepath)
            return;
    }
    orderList.append(filepath);
    setTable(orderList,filepath);
}
void PlayTable:: myClear()
{
    this->clear();
    orderList.clear();
    random_order.clear();
    order_random.clear();
}
