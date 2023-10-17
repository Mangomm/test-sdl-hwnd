#include "mainwindow.h"
#include <QFrame>
#include <QApplication>
#include <QDesktopWidget>
#include "ui_mainwindow.h"
#include "testffmpeg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->btn->setText("play");

    _frame = new QFrame();
    _frame->hide();
    QRect rect = QApplication::desktop()->screenGeometry();
    _frame->resize(rect.width(), rect.height());
    _player = ff_create_player();
    _is_play = false;
}

MainWindow::~MainWindow()
{
    if(_player){
        // stop if still play
        if(_is_play){
            ff_stop_player(_player);
        }
        ff_free_player(_player);
    }

    delete ui;
}

void MainWindow::on_btn_clicked()
{
    if(!_is_play){
        /* file name is fixed at lib-testffmpeg-wrapper */
        ff_play_player(_player, NULL, (void*)_frame->winId());

        _is_play = true;
        ui->btn->setText("stop");
    }else{
        ff_stop_player(_player);
        _is_play = false;
        ui->btn->setText("play");
    }
}
