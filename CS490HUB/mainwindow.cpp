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

int id = 0;

QStandardItemModel *model;
QStandardItem *r1ID;
QStandardItem *r1Type;
QStandardItem *r1Status;
QStandardItem *r2ID;
QStandardItem *r2Type;
QStandardItem *r2Status;
QStandardItem *r3ID;
QStandardItem *r3Type;
QStandardItem *r3Status;
QStandardItem *wID;
QStandardItem *wType;
QStandardItem *wStatus;

bool waiting = false;
bool world = false;
bool saOn = false;
bool taOn = false;
bool aOn = false;
QString taskFile = "tasks.txt";

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
    connect(ta, SIGNAL(winnerFound(int,int)),this,SLOT(onWinnerFound(int,int)));
    connect(ta, SIGNAL(taskAssigned(QString)),this,SLOT(onTaskAssigned(QString)));
    connect(ta, SIGNAL(tasksComplete()),this,SLOT(missionComplete()));
    for(int i = 0; i < 3; i++){
        manual[i] = false;
        stopped[i] = false;
    }
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
    QStringList hHeader;
    hHeader.append("ID");
    hHeader.append("TYPE");
    hHeader.append("STATUS");
    //model.index(1,1,model.index(0,0));
    model = new QStandardItemModel(0,3,this);
    model->setHorizontalHeaderLabels(hHeader);
    ui->robotTable->setModel(model);
    ui->robotTable->setColumnWidth(0,30);
    ui->robotTable->setColumnWidth(1,50);
    ui->robotTable->horizontalHeader()->setStretchLastSection(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    timer.start(10);
    mStart = false;
    //ui->robotTable->resizeColumnsToContents();
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

    if(wPressed){right += 0.4; left += 0.4;}
    if(aPressed){right += 0.4; left -= 0.4;}
    if(sPressed){right -= 0.4; left -= 0.4;}
    if(dPressed){right -= 0.4; left += 0.4;}

    QString msg = "DRIVE/" + QString::number(ui->rselect->value()) + "/" + QString::number(right) + "/" + QString::number(left) + "/";

    buf.append(msg);
    if(saOn){
        //ui->textEdit->append("Robot 1: Drive right: " + QString::number(right) + " | left: " + QString::number(left));
        sendMessage(buf, port1);
    }

}

bool MainWindow::sendMessage(QByteArray &data, quint16 port){
    socket->writeDatagram(data, QHostAddress::Broadcast, port);
    //ui->textEdit->append("Message Sent: " + QString(data));
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

    //ui->textEdit->append("Message Received: " + QString(buf));

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
    QString time = " " + ui->missionTime->text();
    switch(m[0]){
    case 'B':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        ta->addBid(message[1].toInt()-1,message[2].toDouble(),message[3].toDouble());
        //qDebug() << "Bid recieved: " + message[2];
        //ui->textEdit->append("Bid Recieved: Robot - " + message[1]);
        break;
    }
    case 'N':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        ui->textEdit->append("Task Completed: Task #" + message[1] + " " + ui->missionTime->text());
        ta->taskCompleted(message[1].toInt());
        switch(message[2].toInt()){
        case 1:{
            //ui->status1->setText("Idle");
            break;
        }
        case 2:{
            //ui->status2->setText("Idle");
            break;
        }
        case 3:{
            //ui->status3->setText("Idle");
            break;
        }
        }
        break;
    }
    case 'A':{
        ta->activeBots++;
        break;
    }
    case 'D':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        if(message[1].toInt() == ui->rselect->value()){
            double x = message[2].toDouble();
            double y = message[3].toDouble();
            int task = message[4].toInt();
            ui->rStatus->setText("R:" + QString::number(x) + ", " + QString::number(y) + "; T:" +
                                 QString::number(ta->tasks[task].getX()) + ", " + QString::number(ta->tasks[task].getY()));
        }
        break;
    }
    case 'G':{
        waiting = false;
        replys++;
        //ui->textEdit->append("Winner Accepts");
        int tasknum = ta->currentTask;
         ui->textEdit->append("Task# " + QString::number(tasknum) + " Assigned" + time);
        if(ta->activeBots > 0 && replys == ta->tasks[ta->currentTask].getRNum()){
            ta->assignNextTask();
        }
        break;
    }
    case 'H':{
        QString line(buf);
        line = line.right(line.length() - 1);
        QStringList message = line.split("/");
        qDebug() << "teleop";
        ui->textEdit->append("Teleoperation Request: Robot " + message[1] + " error: " + message[2] + " " + ui->missionTime->text());
        switch(message[1].toInt()){
        case 1:{
            r1Status->setText("Requires Teleoperation");
            break;
        }
        case 2:{
            r2Status->setText("Requires Teleoperation.");
            break;
        }
        case 3:{
            r3Status->setText("Requires Teleoperation");
            break;
        }
        }
        break;
    }
    case 'F':{
        QString line(buf);
        line = line.right(line.length() - 1);
        buf = NULL;
        buf.append("R" + line);
        sendMessage(buf,port1);
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


void MainWindow::on_initialize_clicked() // create button
{
    QByteArray data;
    if(!(ui->robotSel->isChecked() && ui->worldSel->isChecked())){
        QString iridium;
        //QStringList args;
        iridium.append("java -jar C:\\Qt\\CS490HUB\\IridiumSim.jar ");
        QProcess *irid = new QProcess(this);
        if(ui->robotSel->isChecked() && ta->activeBots < 3){
            id++;
            if(id > 1){
                ui->rselect->setMaximum(id);
            }
            ta->activeBots++;
            iridium.append(QString::number(id));
            switch(ta->activeBots){
            case 1:
                iridium.append(" 40");
                data.append("ROBOT/1/5.0/3.0/");
                r1ID = new QStandardItem(QString::number(id));
                r1ID->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                model->setItem(id-1, 0, r1ID);
                r1Type = new QStandardItem("ROBOT");
                r1Type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 1, r1Type);
                r1Status = new QStandardItem("Connected");
                r1Status->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 2, r1Status);
                break;
            case 2:
                iridium.append(" 50");
                data.append("ROBOT/2/5.0/-1.0/");
                r2ID = new QStandardItem(QString::number(id));
                r2ID->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                model->setItem(id-1, 0, r2ID);
                r2Type = new QStandardItem("ROBOT");
                r2Type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 1, r2Type);
                r2Status = new QStandardItem("Connected");
                r2Status->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 2, r2Status);
                break;
            case 3:
                iridium.append(" 60");
                data.append("ROBOT/3/5.0/-4.0/");
                r3ID = new QStandardItem(QString::number(id));
                r3ID->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                model->setItem(id-1, 0, r3ID);
                r3Type = new QStandardItem("ROBOT");
                r3Type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 1, r3Type);
                r3Status = new QStandardItem("Connected");
                r3Status->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 model->setItem(id-1, 2, r3Status);
                break;
            }
            irid->start(iridium);
            qDebug()<<irid->errorString();
            QThread::sleep(3);
            sendMessage(data, port1);
        } else if(ui->worldSel->isChecked() && !world){
            id++;
            world = true;
            iridium.append(QString::number(id));
            iridium.append(" 0");
            data.append("CONTROLLER/" + QString::number(id) + "/" + taskFile + "/");
            irid->start(iridium);
            QThread::sleep(3);
            wID = new QStandardItem(QString::number(id));
            wID->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            model->setItem(id-1, 0, wID);
            wType = new QStandardItem("WORLD");
            wType->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
             model->setItem(id-1, 1, wType);
            wStatus = new QStandardItem("Connected");
            wStatus->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
             model->setItem(id-1, 2, wStatus);
            sendMessage(data, port1);
        }
    }
}

void MainWindow::on_Autnomous_clicked()
{
    t.start();
    mStart = true;
    if(ui->saToggle->isChecked()){
        saOn = true;
    }
    if(ui->taToggle->isChecked()){
        taOn = true;
    }
    if(ui->autonomy->isChecked()){
        aOn = true;
    }else{
        for(int i = 0; i < 3; i++){
            manual[i] = true;
            stopped[i] = true;
        }
    }
    QByteArray data;
    data.append("TEST/" + QString(saOn?"true":"false") + "/" + QString(taOn?"true":"false") + "/" + QString(aOn?"true":"false") + "/");
    sendMessage(data,9001);
    if(aOn){
        qDebug() << "Beginning Task Allocation";
        ta->readTasks("C:/Qt/CS490HUB/" + taskFile);
    }
}

void MainWindow::onTaskAssigned(QString message){
    QByteArray m;
    m.append(message);
    sendMessage(m,9001);
    QString time = " " + ui->missionTime->text();

}

void MainWindow::onWinnerFound(int winner, int winner2){
    //qDebug() << winner;
    QByteArray data;
    replys = 0;
    data.append("WINNER/" + QString::number(winner) + "/1/");
    //ui->textEdit->append("Winner Chosen: Robot - " + QString::number(winner));
    switch(winner){
    case 1:{
        r1Status->setText("On Auto");
        break;
    }
    case 2:{
        r2Status->setText("On Auto");
        break;
    }
    case 3:{
        r3Status->setText("On Auto");
        break;
    }
    }
    sendMessage(data,port1);
    if(ta->tasks[ta->currentTask].getRNum() == 2){
        data = NULL;
        data.append("WINNER/" + QString::number(winner2) + "/2/");
        //ui->textEdit->append("Winner 2 Chosen: Robot - " + QString::number(winner2));
        switch(winner2){
        case 1:{
            r1Status->setText("On Auto");
            break;
        }
        case 2:{
            r2Status->setText("On Auto");
            break;
        }
        case 3:{
            r3Status->setText("On Auto");
            break;
        }
        }

        sendMessage(data,port1);
    }

}

void MainWindow::on_EStop_clicked()
{
    int selection = ui->rselect->value();
    QByteArray data;
    data.append("ESTOP/" + QString::number(selection) + "/");
    sendMessage(data,port1);
    QString time = " " + ui->missionTime->text();
    ui->textEdit->append("Emergency Stop: Robot " + selection + time);
    switch(selection){
    case 1:{
        if(stopped[0]){
            r1Status->setText("On Auto");
        }else{
            r1Status->setText("Idle");
        }
        stopped[0] = !stopped[0];
        break;
    }
    case 2:{
        if(stopped[1]){
            r2Status->setText("On Auto");
        }else{
            r2Status->setText("Idle");
        }
        stopped[1] = !stopped[1];
        break;
    }
    case 3:{
        if(stopped[2]){
            r3Status->setText("On Auto");
        }else{
            r3Status->setText("Idle");
        }
        stopped[2] = !stopped[2];
        break;
    }
    }
}

void MainWindow::on_Control_clicked()
{
    int selection = ui->rselect->value();
    QByteArray data;
    data.append("SWITCH/" + QString::number(selection) + "/");
    sendMessage(data,port1);
    QString time = " " + ui->missionTime->text();
    switch(selection){
    case 1:{
        if(!manual[0]){
            r1Status->setText("On Teleoperation");
            ui->textEdit->append("Switch to Manual: Robot 1" + time);
        }else{
            r1Status->setText("On Auto");
            ui->textEdit->append("Switch to Autonomous: Robot 1" + time);
        }
        manual[0] = !manual[0];
        break;
    }
    case 2:{
        if(!manual[1]){
            r2Status->setText("On Teleoperation");
            ui->textEdit->append("Switch to Manual: Robot 2" + time);
        }else{
            r2Status->setText("On Auto");
            ui->textEdit->append("Switch to Autonomous: Robot 2" + time);
        }
        manual[1] = !manual[1];
        break;
    }
    case 3:{
        if(!manual[2]){
            r3Status->setText("On Teleoperation");
            ui->textEdit->append("Switch to Manual: Robot 3" + time);
        }else{
            r3Status->setText("On Auto");
            ui->textEdit->append("Switch to Autonomous: Robot 3" + time);
        }
        manual[2] = !manual[2];
        break;
    }
    }
}

void MainWindow::on_complete_clicked()
{
    if(ui->autonomy->isChecked()){
        int selection = ui->rselect->value();
        QByteArray data;
        data.append("COMPLETE/" + QString::number(selection) + "/");
        sendMessage(data,port1);
    }else{
        missionComplete();
    }
}

void MainWindow::on_startSim_clicked()
{
    QProcess *sim = new QProcess(this);
    sim->startDetached("C:\\UDK\\UDK-2013-07\\USARRunMaps\\ExampleMap.bat");
    //QThread::sleep(3);
    qDebug()<<sim->errorString();
}

void MainWindow::on_robotTable_clicked(const QModelIndex &index)
{
    ui->rselect->setValue(index.row()+1);
}

void MainWindow::missionComplete(){
    mTime = t.elapsed();
    mStart = false;
    QString time = " " + ui->missionTime->text();
    ui->textEdit->append("Mission Complete" + time);
}

void MainWindow::updateTimer(){
    qint64 time = t.elapsed();
    int seconds = time/1000;
    time -= seconds*1000;
    int minutes = seconds/60;
    seconds -= minutes*60;
    QString ms = QString::number(time);
    QString s = QString::number(seconds);
    QString m = QString::number(minutes);
    if(ms.length()<2){
        ms = "0" + ms;
    }else{
        ms = ms.left(2);
    }
    if(s.length() < 2){
        s = "0" + s;
    }
    if(m.length() < 2){
        m = "0" + m;
    }
    if(mStart){
        ui->missionTime->setText(m + ":" + s + ":" + ms);
        timer.start(10);
    }
}
