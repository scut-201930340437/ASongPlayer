#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>


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
    void on_play_button_clicked();

    void on_stop_button_clicked();

private:

    //    QTimer *sdlTimer = nullptr;
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
