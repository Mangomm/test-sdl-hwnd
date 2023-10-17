#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QFrame;
class test_player;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_clicked();

private:
    Ui::MainWindow *ui;

    /* do not override window events of QFrame for now */
    QFrame      *_frame;
    test_player *_player;
    /* false-stop; true-play */
    bool        _is_play;
};
#endif // MAINWINDOW_H
