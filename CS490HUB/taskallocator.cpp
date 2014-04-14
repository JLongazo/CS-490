#include "taskallocator.h"

TaskAllocator::TaskAllocator()
{
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
    }
    bidCount = 0;
    activeBots = 0;
    currentTask = -1;
    completeTasks = 0;
    qDebug() << bidCount;
}

void TaskAllocator::assignNextTask(){
    currentTask++;
    if(currentTask >= taskCount){
        currentTask = 0;
    }
    int check = 0;
    bool taskFound = true;
    while(tasks[currentTask].getCompleted()){
        currentTask++;
        if(currentTask >= taskCount){
            currentTask = 0;
        }
        check++;
        if(check >= taskCount){
            taskFound = false;
            break;
        }
    }
    if(taskFound){
        double x = tasks[currentTask].getX();
        double y = tasks[currentTask].getY();
        int r = tasks[currentTask].getRNum();
        int w = tasks[currentTask].getWeight();
        emit taskAssigned("TASK/" + QString::number(currentTask) + "/" + QString::number(x) + "/" + QString::number(y) +
                          "/" + QString::number(r) + "/" + QString::number(w) + "/");
    }
}


void TaskAllocator::readTasks(QString filename){
    QFile file(filename);
    int taskNum = 0;
    completeTasks = 0;
    if(file.open(QIODevice::ReadOnly)){
        QTextStream in(&file);
        while(!in.atEnd()){
            QString line = in.readLine();
            qDebug() << "line read: " << line;
            QStringList task = line.split(",");
            double x = task[0].toDouble();
            double y = task[1].toDouble();
            int r = task[2].toInt();
            int w = task[3].toInt();
            Objective o(x,y,r,w);
            tasks[taskNum] = o;
            tasks[taskNum].setCompleted(false);
            taskNum++;
        }
        file.close();
        taskCount = taskNum;
        currentTask = -1;
        qDebug() << taskCount;
        assignNextTask();
    }else{
        qDebug() << file.errorString();
    }
}

void TaskAllocator::addBid(int index, double bid, double strength){
    bids[index] = bid;
    strengths[index] = strength;
    bidCount++;
    //qDebug() << bidCount << " " << activeBots;
    if(bidCount == activeBots){
       chooseWinner();
    }
}

void TaskAllocator::taskCompleted(int index){
    //tasks[index].setCompleted(true);
    completeTasks++;
    if(completeTasks < taskCount){
        assignNextTask();
    }else{
        emit tasksComplete();
    }
}

void TaskAllocator::chooseWinner(){
    qDebug() << "choosing winner";
    int winner = -1;
    int winner2 = -1;
    for(int i = 0; i < MAX_BIDS; i++){
        qDebug() << strengths[i];
        if(strengths[i] >= tasks[currentTask].getWeight() || tasks[currentTask].getRNum() == 2){
                if(bids[i] < bids[winner] || winner == -1){
                    if(tasks[currentTask].getRNum() == 2 && winner > -1){
                        if(strengths[i] + strengths[winner] >= tasks[currentTask].getWeight()){
                            winner2 = winner;
                        }else{
                            continue;
                        }
                    }
                    winner = i;
            }
        }
    }
    bidCount = 0;
    for(int i = 0; i < MAX_BIDS; i++){
        bids[i] = 100;
        strengths[i] = 0;
    }
    if(winner == -1 || (tasks[currentTask].getRNum() == 2 && winner2 == -1)){
            assignNextTask();
    }else {

        activeBots--;
        if(tasks[currentTask].getRNum() == 2){
            activeBots--;
        }
        tasks[currentTask].setCompleted(true);
        emit winnerFound(winner+1,winner2+1);
    }
}
