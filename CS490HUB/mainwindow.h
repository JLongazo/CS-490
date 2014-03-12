#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "objective.h"
//#include "robot.h"
//#include "taskallocator.h"
//#include <QtNetwork/QTcpSocket>
#include <QMainWindow>
#include <QThread>
#include <QtNetwork/QUdpSocket>
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

private slots:

    void on_initialize_clicked();
    bool onMessageReceived();
    void on_Autnomous_clicked();
    void onTaskAssigned(QString message);


    void on_EStop_clicked();

    void on_Control_clicked();

    void on_complete_clicked();

private:
    Ui::MainWindow *ui;
    QUdpSocket* socket;
    int winnerB;
<<<<<<< HEAD

=======
    int replys;
    bool stopped[3];
    bool manual[3];
>>>>>>> Autonomous
    bool aPressed, wPressed, sPressed, dPressed;
//    int port2 = 2001;
//    int port3 = 2002;
//    int port4 = 2003;
};

#endif // MAINWINDOW_H


