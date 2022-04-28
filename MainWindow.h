#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "asongplayer.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>
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
    ~MainWindow();

private slots:
    void on_action_openFile_triggered();

    void on_play_button_clicked();

    void on_stop_button_clicked();

    void on_mute_button_clicked();


    void on_volume_ctrl_valueChanged(int value);

    void on_position_ctrl_valueChanged(int value);

    void onDurationChanged(qint64 _duration);

    void on_position_ctrl_sliderPressed();

    void on_fullScreen_button_clicked();

    void keyPressEvent(QKeyEvent *event);

    //    void onerrorOccurred(QMediaPlayer::Error error, const QString & errorString);

private:
    Ui::MainWindow *ui;
    QString filename = "";
    ASongPlayer *asongPlayer = nullptr;
    ASongAudio *asongAudio = nullptr;
    ASongVideo *asongVideo = nullptr;

    QTimer *position_timer = NULL;

    bool positionCtrlPressed = false;

    QString duration_str = "00:00:00";
};
#endif // MAINWINDOW_H
