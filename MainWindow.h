#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMenu>
#include "PlayTable.h"
#include <QFileInfo>
#include <QUrl>
#include <QMimeData>
#include <QEvent>



QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public: MainWindow(QWidget *parent = nullptr);

public:

    //播放模式 0 只播当前 1 顺序 2 随机 3 单个循环
    int playMode;
    //由播放列表告知是否可切换，怕卡了
    bool playModeChangable;
    //可切换音量
    bool volumeValueChangable;

    ~MainWindow();

private slots:
    void on_play_button_clicked();


    void on_stop_button_clicked();

    void on_playmode_button_clicked();

    void on_mute_button_clicked();
    //当音量值不为零应该将静音按钮的文字设置为“静音”
    void on_volume_ctrl_valueChanged(int value);

    void on_close_button_clicked();

    void on_minimize_button_clicked();

    void on_maximize_button_clicked();

    void on_title_widget_customContextMenuRequested(const QPoint &pos);

    void on_MainWindow_customContextMenuRequested(const QPoint &pos);

    void on_more_button_clicked();

    //打开文件选择窗口,并播放文件(问题，如果打开错误类型的文件如何处理)
    void openFile();

    void onPlayTableCellDoubleClicked(int row, int column);

    void dropEvent(QDropEvent *e);

private:

    //    QTimer *sdlTimer = nullptr;
    Ui::MainWindow *ui;
    QString filePath;
    //存储播放路径的文件的路径
    QString SavePath="filename.txt";

    void setListFromFilePath();
    void dragEnterEvent(QDragEnterEvent *e);
    //读取/存储历史播放
    void saveFilePath();
    void readFilePath();


};
#endif // MAINWINDOW_H
