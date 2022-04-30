#ifndef PLAYTABLE_H
#define PLAYTABLE_H

#include <QTableWidget>
#include <QVector>
#include <QFileInfo>

//播放列表类
class PlayTable : public QTableWidget
{

public:
    PlayTable();
    //现存bug，播放列表固定为10*2，可能超出范围，没有实现分页
    void init();
    void setTable(QFileInfoList infoList);
    bool isNeededFile(QFileInfo file);
private:
    QVector<QString> neededFile={"mp3","mp4","flv","avi"};
};

#endif // PLAYTABLE_H
