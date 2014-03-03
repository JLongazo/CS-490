#ifndef OBJECTIVE_H
#define OBJECTIVE_H

class Objective
{
public:
    Objective(double x1 = 0, double y1 = 0, int rn = 1);
    void setCompleted(){completed = true;}
    bool getCompleted(){return completed;}
    bool getStatus(){return inProgress;}
    void setProgress(bool p){inProgress = p;}
    double getX(){return x;}
    double getY(){return y;}
    int getRNum(){return robotNum;}

private:
    bool completed;
    double x;
    double y;
    int robotNum;
    bool inProgress;
};

#endif // OBJECTIVE_H
