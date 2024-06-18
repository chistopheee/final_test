#ifndef DEMODULATION_H
#define DEMODULATION_H
#include <QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QtCharts>
#include <math.h>
#include <QTimer>
#include <math.h>
#include <QVector>
#include <vector>
#include <QString>


class demodulation
{
public:
    demodulation();





    double modulatedSignal[2048];//调制信号
    double decode_buff[2048]; //存放解调后的二进制数组

    int length=256;//二进制数组长度
    int A=2;//载波幅度
    int fc=4800;//载波模拟频率
    int RB=2400;//码元速率
    int fs=96000;//fc*N;//fs/fc=一个周期多少个点//采样频率
    int N=fs/RB;//20;//一个周期用多少个点表示
    double threshold=0;
    QString demobincode;
    void ASKDemodulation(double modulatedbuff[]);
    void PSKDemodulation(double modulatedbuff[]);
 //   void FSKDemodulation();




};

#endif // DEMODULATION_H
