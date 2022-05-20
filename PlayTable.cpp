#include "PlayTable.h"
#include "MyMessageWidget.h"

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
    this->setColumnCount(2);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->setShowGrid(false);
    this->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
}

void PlayTable::setTable(QList<QString> infoList,QString filePath)
{
    //顺序播放列表
    orderInfoList=infoList;
    this->clearContents();
    numFile=infoList.size();
    this->setRowCount(numFile);
    for(qint16 i=0;i<numFile;i++)
    {
        this->setItem(i,0,new QTableWidgetItem(getFileNameFromPath(infoList[i])));
        this->setItem(i,1,new QTableWidgetItem(infoList[i]));
    }

    //生成随机列表
    generateRandomList();
    for(int i=0;i<orderInfoList.size();i++)
    {
        if(orderInfoList[i]==filePath)
        {
            playPos=i;
            showHighLight(0,playPos);
        }
    }
    randomPos=order_random[playPos];
}

void PlayTable::generateRandomList()
{
    randomList.clear();
    random_order.clear();
    order_random.clear();

    //根据orderInfoLst生成RandomList
    numFile=orderInfoList.size();
    QVector<int> origin_pos(numFile);
    for(qint16 i=0;i<numFile;i++)
    {
        randomList.append(orderInfoList[i]);
        origin_pos[i]=i;
    }
    quint32 seed = quint32(QDateTime::currentDateTime().toSecsSinceEpoch());
    QRandomGenerator generator(seed);
    for(qint16 i=0;i<numFile;i++)
    {
        int rand = generator.bounded(i, numFile);

        qSwap(randomList[i],randomList[rand]);
        qSwap(origin_pos[i],origin_pos[rand]);
    }
    for(qint16 i=0;i<numFile;i++)
    {
        random_order[i]=origin_pos[i];
        order_random[origin_pos[i]]=i;
    }
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
    if(!this->item(row,1))
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
    for(int i=0;i<2;i++)
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
    int rowIndex = this->currentRow();
    if (rowIndex != -1)
    {
        this->removeRow(rowIndex);
        orderInfoList.removeAt(rowIndex);
        randomList.removeAt(order_random[rowIndex]);
        random_order.remove(order_random[rowIndex]);
        order_random.remove(rowIndex);

    }
}

QString PlayTable::getFileNameFromPath(QString path)
{
    return path.section('/', -1, -1);
}


