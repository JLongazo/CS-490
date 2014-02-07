#include "mainwindow.h"
#include "ui_mainwindow.h"

//QTcpSocket socket2;
//QTcpSocket socket3;
//QTcpSocket socket4;
//Robot r1;
//Robot r2;
//Robot r3;
//Robot robots[] = {r1,r2,r3};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, port1, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    qDebug() << "socket created";
    connect(socket, SIGNAL(readyRead()), this, SLOT(onMessageReceived()));

//    connect(&socket,SIGNAL(readyRead()),this,SLOT(on_message_received1()));
//    connect(&socket2,SIGNAL(readyRead()),this,SLOT(on_message_received2()));
//    connect(&socket3,SIGNAL(readyRead()),this,SLOT(on_message_received3()));
//    socket.connectToHost("localhost",port1,QIODevice::ReadWrite);
//    socket2.connectToHost("localhost",port2,QIODevice::ReadWrite);
//    socket3.connectToHost("localhost",port3,QIODevice::ReadWrite);
//    socket4.connectToHost("localhost",port4,QIODevice::ReadWrite);
//    r1.socket = &socket;
//    r2.socket = &socket2;
//    r3.socket = &socket3;
//    qDebug() << "attempting connection";
//    if(socket.waitForConnected()&& socket2.waitForConnected()&& socket3.waitForConnected()&& socket4.waitForConnected()){
//        qDebug() << "connected";
//    }else{
//        qDebug() << "not connected";
//    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::sendMessage(QByteArray &data, quint16 port){
    socket->writeDatagram(data, QHostAddress::Broadcast, port);

    return true;
}

bool MainWindow::onMessageReceived(){
    QByteArray buf;
    buf.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 sender_port;

    socket->readDatagram(buf.data(), buf.size(), &sender, &sender_port);

    qDebug() << "Message from: " << sender.toString();
    qDebug() << "Message port: " << sender_port;
    qDebug() << "Message: " << buf;

    return true;
}

//void MainWindow::sendMessage(const QString &m, QTcpSocket &s){
//    QByteArray data;
//    data.append(m + "\n");
//    s.write(data, data.length());
//    if(s.waitForBytesWritten()){
//        qDebug() << "message sent:" << data;
//    }else{
//        qDebug() << "message failed";
//    }
//}

//void parseMessage(QString line, int robot){
//    const char *m;
//    QByteArray temp;
//    temp.append(line);
//    m = temp.data();
//    switch(m[0]){
//    case 'A':{
//        line = line.right(line.length() - 1);
//        qDebug() << line;
//        QStringList message = line.split("/");
//        double x = message[0].toDouble();
//        double y = message[1].toDouble();
//        robots[robot - 1].setX(x);
//        robots[robot - 1].setY(y);
//        qDebug() << "robot position set";
//        break;
//    }
//    default:
//        break;
//    }
//}

//void readMessage(QTcpSocket &s, int r){
//    if(s.canReadLine()){
//        QByteArray in = s.readLine();
//        QString message(in);
//        parseMessage(message,r);
//    }
//}


void MainWindow::on_initialize_clicked()
{
    QByteArray data;
    data.append("CONTROLER/");
    sendMessage(data, port1);
//    sendMessage("ROBOT/-1.5/-2.5/",socket);
//    sendMessage("ROBOT/-1.5/-1.0/",socket2);
//    sendMessage("ROBOT/-1.5/0.5/",socket3);
}

void MainWindow::on_Autnomous_clicked()
{

}

//void MainWindow::on_message_received1(){
//        readMessage(socket,1);
//}

//void MainWindow::on_message_received2(){
//        readMessage(socket2,2);
//}

//void MainWindow::on_message_received3(){
//        readMessage(socket3,3);
//}
