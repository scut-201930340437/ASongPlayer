#include "PlayTable.h"

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
    //设置标题
    QStringList header;
    header<<"name"<<"duration";
    this->setHorizontalHeaderLabels(header);
}

void PlayTable::setTable(QFileInfoList infoList){
    this->clearContents();
    numFile=infoList.size();
    this->setRowCount(numFile);
    for(qint16 i=0;i<numFile;i++){
        this->setItem(i,0,new QTableWidgetItem(infoList[i].fileName()));
        this->setItem(i,1,new QTableWidgetItem(infoList[i].absoluteFilePath()));
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
    return this->item(row,1)->text();
}
