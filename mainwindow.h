#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioOutput>
#include <QVideoWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void getDuration();


private slots:
    void on_action_openFile_triggered();

    void on_play_button_clicked();

    void on_stop_button_clicked();

    void on_mute_button_clicked();


    void on_volume_ctrl_valueChanged(int value);

    void on_position_ctrl_valueChanged(int value);

    void onDurationChanged(qint64 _duration);

    void on_position_ctrl_sliderPressed();



private:
    Ui::MainWindow *ui;
    QString filename="";
    QMediaPlayer *player=NULL;
    QAudioOutput *audioOutput=NULL;
    QVideoWidget *videoWidget=NULL;
    QTimer *position_timer=NULL;
    bool isPlaying=false;
    bool existFile=false;
    bool isMute=false;

    bool positionCtrlPressed=false;

    int preVolume=20;
    qint64 duration=0;
};
#endif // MAINWINDOW_H
