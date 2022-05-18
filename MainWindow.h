#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QMutex>
#include <QMenu>
#include "PlayTable.h"
#include <QFileInfo>
#include <QUrl>
#include <QMimeData>
#include <QEvent>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSettings>



QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE




class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    QString getTimeString(int position);

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

    void on_more_button_clicked();


    //打开文件选择窗口,并播放文件(问题，如果打开错误类型的文件如何处理)
    void openFile();

    void onPlayTableCellDoubleClicked(int row, int column);

    void dropEvent(QDropEvent *e);

    //定时事件
    void handleTimeout();

    void on_fullScreen_button_clicked();

    void on_last_button_clicked();

    void on_next_button_clicked();


    void on_position_ctrl_sliderPressed();

    void on_position_ctrl_sliderReleased();

    void on_play_widget_customContextMenuRequested(const QPoint &pos);

    void playFinishSlot();

    void on_play_table_customContextMenuRequested(const QPoint &pos);

    //检查路径是否还存在，如果已删除，则重新设置播放列表
    bool checkIfExist(QString path);

    void on_multiple_button_clicked();

    //设置播放模式
    void setPlayMode(qint16 mode_index);
    //设置倍速
    void setMutipleSpeed(QAbstractButton *button);
private:
    //锁
    QMutex seek_mutex;
    QMutex play_mutex;
    //鼠标位置旧值，判断鼠标隐藏
    QPoint old_mouse_value;
    //多久没动了
    int sustain;
    //倍速窗口
    QWidget *multipleWidget;
    QButtonGroup *m_pButtonGroup;

    Ui::MainWindow *ui;
    QString filePath;
    //存储播放路径的文件的路径
    QString SavePath = "filePath.ini";
    QString curPathKey="curFilePath";
    QString pathListKey="filePathList";

    void setListFromFilePath();
    void dragEnterEvent(QDragEnterEvent *e);
    //读取/存储历史播放
    void saveFilePath();
    void readFilePath();
    //全屏下的鼠标事件
    void mouseMoveEvent (QMouseEvent *event);
    //平时的键盘事件
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);


};
#endif // MAINWINDOW_H
