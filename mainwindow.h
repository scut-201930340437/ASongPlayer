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




private slots:
    void on_action_openFile_triggered();

    void on_play_button_clicked();

    void on_stop_button_clicked();

private:
    Ui::MainWindow *ui;
    QString filename="";
    QMediaPlayer *player=NULL;
    QAudioOutput *audioOutput=NULL;
    QVideoWidget *videoWidget=NULL;
    bool isPlaying=false;
};
#endif // MAINWINDOW_H
