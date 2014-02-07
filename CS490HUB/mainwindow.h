#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "objective.h"
//#include "robot.h"
//#include "taskallocator.h"
//#include <QtNetwork/QTcpSocket>
#include <QMainWindow>
#include <QtNetwork/QUdpSocket>

//void parseMessage(QString line, int robot);
//void readMessage(QTcpSocket &s, int r);


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //sendMessage        data      port
    bool sendMessage(QByteArray&, quint16);

private slots:

    void on_initialize_clicked();
    bool onMessageReceived();
    void on_Autnomous_clicked();

private:
    Ui::MainWindow *ui;
    QUdpSocket* socket;

    quint16  port1 = 9001;
//    int port2 = 2001;
//    int port3 = 2002;
//    int port4 = 2003;
};

#endif // MAINWINDOW_H
