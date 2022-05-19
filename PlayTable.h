#ifndef PLAYTABLE_H
#define PLAYTABLE_H

#include <QTableWidget>
#include <QVector>
#include <QFileInfo>
#include <QDir>
#include <QRandomGenerator>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QHeaderView>
class PlayTable : public QTableWidget
{
    Q_OBJECT

public:
    //提升 机制 待研究
    explicit PlayTable(QWidget *parent = nullptr);
    qint16 playPos;
    qint16 randomPos;
    qint16 numFile=0;
    QMap<int,int> order_random;
    QMap<int,int> random_order;
    void init();
    void setTable(QFileInfoList infoList);
    bool isNeededFile(QFileInfo file);
    QString getPath(qint16 row);
    QString getNextFile();
    QString getPrevFile();
    //播放模式 0 只播当前 1 顺序 2 随机 3 单个循环
    qint16 playMode=1;
    qint16 getNumFiles();
    void showHighLight();

private:
    QFileInfoList orderInfoList;
    QList<QString> randomList;
    QVector<QString> neededFile={"mp3","mp4","flv","avi","mkv"};
    void generateRandomList();

private slots:
    void showMessage();
    void deleteFile();
};

#endif // PLAYTABLE_H
