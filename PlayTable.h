#ifndef PLAYTABLE_H
#define PLAYTABLE_H

#include <QTableWidget>
#include <QVector>
#include <QFileInfo>
#include <QDir>

class PlayTable : public QTableWidget
{
    Q_OBJECT

public:
    //提升 机制 待研究
    explicit PlayTable(QWidget *parent = nullptr);
    qint16 playPos;
    qint16 numFile=10;
    void init();
    void setTable(QFileInfoList infoList);
    bool isNeededFile(QFileInfo file);
    QString getPath(qint16 row);


private:
    QFileInfoList *playTable;
    QVector<QString> neededFile={"mp3","mp4","flv","avi","mkv"};
};

#endif // PLAYTABLE_H
