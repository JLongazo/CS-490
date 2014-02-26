#include "mainwindow.h"
#include "ui_mainwindow.h"


//QTcpSocket socket2;
//QTcpSocket socket3;
//QTcpSocket socket4;
//Robot r1;
//Robot r2;
//Robot r3;
//Robot robots[] = {r1,r2,r3};

quint16 port1 = 9001;

TaskAllocator* ta;

bool waiting = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    winnerB = 0;
    ui->setupUi(this);
    ta = new TaskAllocator();
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, port1, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    qDebug() << "socket created";
    connect(socket, SIGNAL(readyRead()), this, SLOT(onMessageReceived()));
    connect(ta, SIGNAL(winnerFound(int)),this,SLOT(onWinnerFound(int)));
    connect(ta, SIGNAL(taskAssigned(QString)),this,SLOT(onTaskAssigned(QString)));

    //Make the text area non-editable so we can read key events
    ui->textEdit->setReadOnly(true);

    //Initialize key press trackers to false
    aPressed = wPressed = sPressed = dPressed = false;

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

void MainWindow::keyPressEvent(QKeyEvent* event){
    switch(event->key()){
        case Qt::Key_W:
            wPressed = true;
            break;
        case Qt::Key_A:
            aPressed = true;
            break;
        case Qt::Key_S:
            sPressed = true;
            break;
        case Qt::Key_D:
            dPressed = true;
            break;
        default:
            //Do nothing
            break;
    }
    updateMotion();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event){
    switch(event->key()){
        case Qt::Key_W:
            wPressed = false;
            break;
        case Qt::Key_A:
            aPressed = false;
            break;
        case Qt::Key_S:
            sPressed = false;
            break;
        case Qt::Key_D:
            dPressed = false;
            break;
        default:
            //Do nothing
            break;
    }
    updateMotion();
}

void MainWindow::updateMotion(){
    QByteArray buf;
    double right = 0, left = 0;

    if(wPressed){right += 0.7; left += 0.7;}
    if(aPressed){right += 0.5; left -= 0.5;}
    if(sPressed){right -= 0.7; left -= 0.7;}
    if(dPressed){right -= 0.5; left += 0.5;}

    QString msg = "DRIVE/" + QString::number(ui->rselect->value()) + "/" + QString::number(right) + "/" + QString::number(left) + "/";

    ui->textEdit->append("Robot " + QString::number(ui->rselect->value()) + ": Drive right: " + QString::number(right) + " | left: " + QString::number(left));

    buf.append(msg);
    sendMessage(buf, port1);

}

bool MainWindow::sendMessage(QByteArray &data, quint16 port){
    socket->writeDatagram(data, QHostAddress::Broadcast, port);
    if(socket->waitForBytesWritten()){}
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

    parseMessage(buf);

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

void MainWindow::parseMessage(QByteArray buf){
    const char *m;
    m = buf.data();
    switch(m[0]){
    case 'B':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        ta->addBid(message[1].toInt()-1,message[2].toDouble());
        //qDebug() << "Bid recieved: " + message[2];
        break;
    }
    case 'N':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        ta->taskCompleted(message[1].toInt());
        switch(message[2].toInt()){
        case 1:{
            ui->status1->setText("Idle");
            break;
        }
        case 2:{
            ui->status2->setText("Idle");
            break;
        }
        case 3:{
            ui->status3->setText("Idle");
            break;
        }
        }
        break;
    }
    case 'G':{
        waiting = false;
        if(ta->getBotCount() > 0){
            ta->assignNextTask();
        }
    }
    case 'H':{
        //switch to teleoperation
    }
    default:
        if(waiting){
            //onWinnerFound(winnerB);
        }
        break;
    }
}

//void readMessage(QTcpSocket &s, int r){
//    if(s.canReadLine()){
//        QByteArray in = s.readLine();
//        QString message(in);
//        parseMessage(message,r);
//    }
//}


void MainWindow::on_initialize_clicked()
{
    QByteArray data,data2,data3,data4;
    data.append("CONTROLLER/");
    data2.append("ROBOT/1/5.0/2.0/");
    data3.append("ROBOT/2/5.0/-2.0/");
    data4.append("ROBOT/3/5.0/-6.0/");
    ui->status1->setText("Connected");
    ui->status2->setText("Connected");
    ui->status3->setText("Connected");
    sendMessage(data, port1);
    sendMessage(data2,port1);
    sendMessage(data3, port1);
    sendMessage(data4,port1);
}

void MainWindow::on_Autnomous_clicked()
{
    qDebug() << "Beginning Task Allocation";
    ta->readTasks("C:/Qt/CS490HUB/tasks.txt");
}

void MainWindow::onTaskAssigned(QString message){
    QByteArray m;
    m.append(message);
    sendMessage(m,9001);
}

void MainWindow::onWinnerFound(int winner){
    qDebug() << winner;
    QByteArray data;
    data.append("WINNER/" + QString::number(winner) + "/");
    switch(winner){
    case 1:{
        ui->status1->setText("On Task");
        break;
    }
    case 2:{
        ui->status2->setText("On Task");
        break;
    }
    case 3:{
        ui->status3->setText("On Task");
        break;
    }
    }
    sendMessage(data,port1);
}

void MainWindow::on_EStop_clicked()
{
    int selection = ui->rselect->value();
    QByteArray data;
    data.append("ESTOP/" + QString::number(selection) + "/");
    sendMessage(data,port1);
}

void MainWindow::on_Control_clicked()
{
    //int selection = ui->rselect->value();
    //switch robot selected to teleoperation
}
