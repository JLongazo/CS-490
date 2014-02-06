#include "mainwindow.h"
#include "ui_mainwindow.h"


QUdpSocket socket;
/*
QUdpSocket socket2;
QUdpSocket socket3;
QUdpSocket socket4;
*/
Robot r1;
Robot r2;
Robot r3;
Robot robots[] = {r1,r2,r3};

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
    connect(&socket,SIGNAL(readyRead()),this,SLOT(on_message_received1()));
    //connect(&socket2,SIGNAL(readyRead()),this,SLOT(on_message_received2()));
    //connect(&socket3,SIGNAL(readyRead()),this,SLOT(on_message_received3()));
    qDebug() << "attempting connection";
    if(socket.bind(QHostAddress::LocalHost,port1)){
        qDebug() << "bound";
    }else{
        qDebug() << "could not bind";
    }

    //socket.connectToHost("localhost",port1,QIODevice::ReadWrite);
    /*
    socket2.connectToHost("localhost",port2,QIODevice::ReadWrite);
    socket3.connectToHost("localhost",port3,QIODevice::ReadWrite);
    socket4.connectToHost("localhost",port4,QIODevice::ReadWrite);
    r1.socket = &socket;
    r2.socket = &socket2;
    r3.socket = &socket3;

    if(socket.waitForConnected()){
        qDebug() << "connected";
    }else{
        qDebug() << "not connected";
    }


    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendMessage(const QString &m, QUdpSocket &s){
    QByteArray data;
    data.append(m + "\n");
    int i = s.writeDatagram(data, QHostAddress::LocalHost, port1);
    qDebug() << "message sent" << i;
    /*
    if(s.waitForBytesWritten()){
        qDebug() << "message sent:" << data;
    }else{
        qDebug() << "message failed ";
    }
    */
}

void parseMessage(QString line, int robot){
    const char *m;
    QByteArray temp;
    temp.append(line);
    m = temp.data();
    switch(m[0]){
    case 'A':{
        line = line.right(line.length() - 1);
        qDebug() << line;
        QStringList message = line.split("/");
        double x = message[0].toDouble();
        double y = message[1].toDouble();
        robots[robot - 1].setX(x);
        robots[robot - 1].setY(y);
        qDebug() << "robot position set";
        break;
    }
    default:
        break;
    }
}

void readMessage(QUdpSocket &s, int r){
    if(s.canReadLine()){
        QByteArray in = s.readLine();
        QString message(in);
        parseMessage(message,r);
    }
}


void MainWindow::on_initialize_clicked()
{
    sendMessage("CONTROLLER/",socket);
    //sendMessage("ROBOT/-1.5/-2.5/",socket);
    //sendMessage("ROBOT/-1.5/-1.0/",socket2);
    //sendMessage("ROBOT/-1.5/0.5/",socket3);
}

void MainWindow::on_Autnomous_clicked()
{

}

void MainWindow::on_message_received1(){
        //readMessage(socket,1);
}

void MainWindow::on_message_received2(){
       // readMessage(socket2,2);
}

void MainWindow::on_message_received3(){
       // readMessage(socket3,3);
}
