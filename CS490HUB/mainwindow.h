#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "objective.h"
#include "robot.h"
#include "taskallocator.h"
#include <QMainWindow>
#include<QtNetwork/QTcpSocket>

void parseMessage(QString line, int robot);
void readMessage(QTcpSocket &s, int r);


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void sendMessage(const QString &m, QTcpSocket &s);

private slots:

    void on_initialize_clicked();

    void on_Autnomous_clicked();

    void on_message_received1();

    void on_message_received2();

    void on_message_received3();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
