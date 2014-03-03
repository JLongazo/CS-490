#include "taskallocator.h"

TaskAllocator::TaskAllocator()
{
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
    }
    bidCount = 0;
    activeBots = 3;
    currentTask = -1;
    qDebug() << bidCount;
}

void TaskAllocator::assignNextTask(){
    currentTask++;
    double x = tasks[currentTask].getX();
    double y = tasks[currentTask].getY();
    int r = tasks[currentTask].getRNum();
    emit taskAssigned("TASK/" + QString::number(currentTask) + "/" + QString::number(x) + "/" + QString::number(y) +
                      "/" + QString::number(r) + "/");
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
            int r = task[2].toInt();
            Objective o(x,y,r);
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
    if(currentTask < taskCount-1){
        assignNextTask();
    }
}

void TaskAllocator::chooseWinner(){
    int winner = 0;
    int winner2 = 0;
    for(int i = 0; i < MAX_BIDS; i++){
        if(bids[i] < bids[winner]){
            winner2 = winner;
            winner = i;
        }
    }
    bidCount = 0;
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
    }
    activeBots--;
    if(tasks[currentTask].getRNum() == 2){
        activeBots--;
    }

    emit winnerFound(winner+1,winner2+1);
}
