#include "PlayTable.h"

PlayTable::PlayTable()
{
    init();

}

void PlayTable::init()
{
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    this->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    this->setSelectionMode(QAbstractItemView::SingleSelection);  //设置为只能选中单个目标
    this->resize(390,480);
    this->setRowCount(10);
    this->setColumnCount(2);
    QStringList header;
    header<<"name"<<"duration";
    this->setHorizontalHeaderLabels(header);
}

void PlayTable::setTable(QFileInfoList infoList){
    this->clearContents();
    for(qint16 i=0;i<infoList.size();i++){
        this->setItem(i,0,new QTableWidgetItem(infoList[i].fileName()));
        this->setItem(i,1,new QTableWidgetItem(infoList[i].absoluteFilePath()));
    }
}

bool PlayTable::isNeededFile(QFileInfo file){
    QString suffix=file.suffix();
    for(QString str:neededFile){
        if(suffix==str)
            return true;
    }
    return false;
}
