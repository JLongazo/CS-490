#include "mainwindow.h"
#include "ui_mainwindow.h"


QTcpSocket socket;
QTcpSocket socket2;
QTcpSocket socket3;
QTcpSocket socket4;
int port1 = 2000;
int port2 = 2001;
int port3 = 2002;
int port4 = 2003;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "socket created";
    socket.connectToHost("localhost",port1,QIODevice::ReadWrite);
    socket2.connectToHost("localhost",port2,QIODevice::ReadWrite);
    socket3.connectToHost("localhost",port3,QIODevice::ReadWrite);
    socket4.connectToHost("localhost",port4,QIODevice::ReadWrite);
    qDebug() << "attempting connection";
    if(socket.waitForConnected()&& socket2.waitForConnected()&& socket3.waitForConnected()&& socket4.waitForConnected()){
        qDebug() << "connected";

    }else{
        qDebug() << "not connected";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    sendMessage("CONTROLLER/",socket4);

}

void MainWindow::sendMessage(const QString &m, QTcpSocket &s){
    QByteArray data;
    data.append(m + "\n");
    s.write(data, data.length());
    if(s.waitForBytesWritten()){
        qDebug() << "message sent:" << data;
    }else{
        qDebug() << "message failed";
    }
}
