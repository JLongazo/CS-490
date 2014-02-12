#ifndef ROBOT_H
#define ROBOT_H

#include"objective.h"
//#include<QtNetwork/QTcpSocket>


class Robot
{
public:
    Robot();
   // QTcpSocket *socket;
    Objective *obj;
    double getX(){return x;}
    double getY(){return y;}
    void setX(double x1){x = x1;}
    void setY(double y1){y = y1;}

private:
    double x;
    double y;
};

#endif // ROBOT_H
