#ifndef OBJECTIVE_H
#define OBJECTIVE_H

class Objective
{
public:
    Objective(double x1 = 0, double y1 = 0);
    void setCompleted(){completed = true;}
    bool getCompleted(){return completed;}
    bool getStatus(){return inProgress;}
    void setProgress(bool p){inProgress = p;}
    double getX(){return x;}
    double getY(){return y;}

private:
    bool completed;
    double x;
    double y;
    bool inProgress;
};

#endif // OBJECTIVE_H
