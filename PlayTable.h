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
    explicit PlayTable(QWidget *parent = nullptr);
    qint16 orderPos;//顺序播放的位置
    qint16 randomPos;
    //key为顺序列表的下标，value为随机列表的下标
    QList<qint16> order_random;
    //同理
    QList<qint16> random_order;
    //所有的顺序播放路径
    QList<QString> orderList;
    //播放模式 0 只播当前 1 顺序 2 随机 3 单个循环
    qint16 playMode=1;
    QList<QWidget *> allCloseWidget;//存储所有打开的窗口，用来级联关闭
    void init();//初始化
    void setTable(QList<QString> infoList,QString &filePath);//设置展示播放列表
    bool isNeededFile(QFileInfo file);//判断文件是否合法
    QString getPath(qint16 row);//得到第row行的数据
    QString getNextFile();//返回下一首的路径，并进行顺序/随机播放内部数据的运算处理
    QString getPrevFile();//同上
    qint16 getNumFiles();//获得当前列表的行数
    void showHighLight(qint16 pre,qint16 cur);//消除高亮，并显示新高亮
    QString getFileNameFromPath(QString path);//从绝对路径中获得文件名
    void deleteFile();//响应MainWindow，删除选中的文件对应的数据结构
    void doMouseTrackTip(QModelIndex index);//鼠标悬停显示文字
    void addFilePath(QString filepath);//添加一个播放路径
    void myClear();//清除所有播放列表相关的数据结构


private:
    static QVector<QString> neededFile;
    static QColor backgroundColor;
    static QColor selectColor;
    static QColor onPlayingColor;

private slots:
    void showMessage();//展示详细信息
    void generateRandomList();//重新生成随机播放列表
};

#endif // PLAYTABLE_H

