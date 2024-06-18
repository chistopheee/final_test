#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QtCharts>
#include <math.h>
#include <QChart>
#include <string.h>

#include <QVector>
#include <vector>
#include <QString>

#include <QTimer>
#include <QTime>

#include "modulation.h"
#include "sender.h"

#include "receiver.h"
#include "demodulation.h"

QT_CHARTS_USE_NAMESPACE
#define PI 3.1415926
#define samplelength 96000
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //用于显示字符
    QString dataLine; //文本内容
    QString signalData;
    int i=0;//变量
    //画图
    QTimer *timer;//定时器
    QSplineSeries* Series;
    QChart* Chart;
    double ymax=20;
    double ymin=0;
    double xmax=20;
    double xmin=0;
    int j=0;
    //uint8_t drawwhat;//0 dat 1 cfg
    uint32_t cycle = 0;//进入中断函数的次数

    void initChart();
    void updateShow();
    //-----------------------------------------------------------------------调制
    //modulation MOD;
    int code_buff[2048];//储存二进制数组
    double modulatedSignal[samplelength];//储存调制数组
    QString bincode;//二进制数据的字符串
//解调
    demodulation DEMOD;
    QString hexcode;

private slots:

    void on_pb_Openfile_clicked();

    void on_pb_Savefile_clicked();

//    void on_pb_Startmodu_clicked();

    void on_pb_Savemodu_clicked();

    void on_lineEdit_Fc_returnPressed();

    void on_lineEdit_RB_returnPressed();

    void on_lineEdit_FS_returnPressed();

    //void on_horizontalScrollBar_valueChanged(int value);

    void DrawLine();

    void on_pb_OpenPSKfile_clicked();

    void on_pb_SavePSKfile_clicked();

private:
    Ui::MainWindow *ui;

    //qcharts




};

#endif // MAINWINDOW_H
