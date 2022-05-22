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
#include <QToolTip>



class PlayTable : public QTableWidget
{
    Q_OBJECT

public:
    //提升 机制 待研究
    explicit PlayTable(QWidget *parent = nullptr);
    qint16 playPos;
    qint16 randomPos;
    qint16 numFile=0;
    //key为顺序列表的下标，value为随机列表的下标
    QList<qint16> order_random;
    //同理
    QList<qint16> random_order;
    QList<QString> orderInfoList;
    QList<QString> randomList;
    void init();
    void setTable(QList<QString> infoList,QString filePath);
    bool isNeededFile(QFileInfo file);
    QString getPath(qint16 row);
    QString getNextFile();
    QString getPrevFile();
    //播放模式 0 只播当前 1 顺序 2 随机 3 单个循环
    qint16 playMode=1;
    qint16 getNumFiles();
    void showHighLight(qint16 pre,qint16 cur);
    QString getFileNameFromPath(QString path);
    void deleteFile();
    void doMouseTrackTip(QModelIndex index);
    void addFilePath(QString filepath);

private:
    QVector<QString> neededFile={"mp3","mp4","flv","avi","mkv"};
    void generateRandomList();
    QColor backgroundColor=QColor(68, 65, 63);
    QColor selectColor=QColor(158, 158, 252);
    QColor onPlayingColor=QColor(224, 224, 226);

private slots:
    void showMessage();
};

#endif // PLAYTABLE_H

