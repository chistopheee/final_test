#ifndef MODULATION_H
#define MODULATION_H
#include <QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QtCharts>
#include <math.h>
#include <QTimer>
#define PI 3.1415926
#define samplelength 96000

class modulation
{


public:

//载波参数

    int length=256;//二进制数组长度
    int A=2;//载波幅度
    int fc=4800;//载波模拟频率
    int RB=2400;//码元速率
    int fs=96000;//fc*N;//fs/fc=一个周期多少个点//采样频率
    int N=fs/fc;//20;//一个周期用多少个点表示
   // double carrierSignal[5000]; //载波信号
    double modulatedSignal[samplelength];//调制信号

    void clear();
    void ASKmodulation(int code[]);
    void PSKmodulation(int code[],int phase);
  //  void FSKmodulation();



public:
    modulation();
};

#endif // MODULATION_H
