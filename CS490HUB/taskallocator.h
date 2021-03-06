#ifndef TASKALLOCATOR_H
#define TASKALLOCATOR_H
#include <QFile>
#include "mainwindow.h"
#include "objective.h"


class TaskAllocator : public QObject
{
    Q_OBJECT

public:
    TaskAllocator();


    static const int MAX_BIDS = 3;
    static const int MAX_TASKS = 10;

    Objective tasks[MAX_TASKS];
    int taskCount;
    int currentTask;
    int activeBots;

    void addBid(int index, double bid, double strength);
    quint16 getWinner();
    void readTasks(QString file);
    void taskCompleted(int index);
    void assignNextTask();
    void chooseWinner();

private:
    double bids[MAX_BIDS];
    double strengths[MAX_BIDS];
    int bidCount;
    int completeTasks;


signals:
    void tasksCollected();
    void taskAssigned(QString task);
    void winnerFound(int winner, int winner2);
    void tasksComplete();
};

#endif // TASKALLOCATOR_H
