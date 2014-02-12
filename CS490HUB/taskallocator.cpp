#include "taskallocator.h"

TaskAllocator::TaskAllocator()
{
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
    }
    bidCount = 0;
    activeBots = 3;
    currentTask = 0;
    qDebug() << bidCount;
}

void TaskAllocator::assignNextTask(){
    double x = tasks[currentTask].getX();
    double y = tasks[currentTask].getY();
    currentTask++;
    emit taskAssigned("TASK/" + QString::number(currentTask) + "/" + QString::number(x) + "/" + QString::number(y));
}


void TaskAllocator::readTasks(QString filename){
    QFile file(filename);
    int taskNum = 0;
    if(file.open(QIODevice::ReadOnly)){
        QTextStream in(&file);
        while(!in.atEnd()){
            QString line = in.readLine();
            qDebug() << "line read: " << line;
            QStringList task = line.split(",");
            double x = task[0].toDouble();
            double y = task[1].toDouble();
            Objective o(x,y);
            tasks[taskNum] = o;
            taskNum++;
        }
        file.close();
        taskCount = taskNum;
        qDebug() << taskCount;
        assignNextTask();
    }else{
        qDebug() << file.errorString();
    }
}

void TaskAllocator::addBid(int index, double bid){
    bids[index] = bid;
    bidCount++;
    //qDebug() << bidCount << " " << activeBots;
    if(bidCount == activeBots){
       chooseWinner();
    }
}

void TaskAllocator::taskCompleted(int index){
    tasks[index].setCompleted();
    activeBots++;
    assignNextTask();
}

void TaskAllocator::chooseWinner(){
    int winner = 0;
    for(int i = 0; i < MAX_BIDS; i++){
        if(bids[i] < bids[winner]){
            winner = i;
        }
    }
    bidCount = 0;
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
    }
    activeBots--;
    emit winnerFound(winner+1);
}
