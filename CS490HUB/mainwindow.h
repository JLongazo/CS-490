#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "objective.h"
//#include "robot.h"
//#include "taskallocator.h"
//#include <QtNetwork/QTcpSocket>
#include <QMainWindow>
#include <QThread>
#include <QStandardItemModel>
#include <QProcess>
#include <QtNetwork/QUdpSocket>
#include <QElapsedTimer>
#include <QTimer>
#include "taskallocator.h"

#include <QKeyEvent>

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
    void parseMessage(QByteArray buf);

    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    void updateMotion();


public slots:
     void onWinnerFound(int winner, int winner2);
     void missionComplete();

private slots:

    void on_initialize_clicked();
    bool onMessageReceived();
    void on_Autnomous_clicked();
    void onTaskAssigned(QString message);


    void on_EStop_clicked();

    void on_Control_clicked();

    void on_complete_clicked();

    void on_startSim_clicked();

    void on_robotTable_clicked(const QModelIndex &index);

    void updateTimer();

    void on_reset_clicked();

private:
    Ui::MainWindow *ui;
    QUdpSocket* socket;
    QElapsedTimer t;
    QTimer timer;
    int winnerB;
    int replys;
    bool stopped[3];
    bool manual[3];
    bool aPressed, wPressed, sPressed, dPressed;
    bool mStart;
    qint64 mTime;
//    int port2 = 2001;
//    int port3 = 2002;
//    int port4 = 2003;
};

#endif // MAINWINDOW_H


