#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QtCharts>
#include <math.h>
#include <QChart>
#include <QWidget>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QMediaPlayer>


#include <QVector>
#include <vector>
#include <QString>

#include <QTimer>
#include <QTime>



#include "modulation.h"
#include "sender.h"

#include "audiolevel.h"
#define num 1928401110

QT_CHARTS_USE_NAMESPACE
using namespace std;
#define PI 3.1415926

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



    //字符用于显示

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
    uint32_t cycle = 0;//进入中断函数的次数
    uint8_t drawwhat;//0 dat 1 cfg
    //时域波形显示


    void initChart();


    //--------调制
    modulation MOD;
    int code_buff[2048];//储存二进制数组
    double modulatedSignal[96000];//储存调制数组
    QString bincode;//二进制数据的字符串


private slots:

    void on_pb_Openfile_clicked();

    void on_pb_Savefile_clicked();

    void on_pb_StartmoduAsk_clicked();

    void on_pb_Savemodu_clicked();

    //void on_horizontalScrollBar_valueChanged(int value);
    void DrawLine();

    void on_pb_StartmoduPsk_clicked();

    void on_pb_SavemoduPsk_clicked();

    void on_btnGetFile_clicked();

    void on_actRecord_triggered();

    void on_actPause_triggered();

    void on_actStop_triggered();

    void on_buttonFile_clicked();

    void on_buttonSend_clicked();

    void on_pb_play_clicked();

    void on_pbStart_clicked();

    void on_pbStop_clicked();

    void on_pbCom_clicked();

private:
    Ui::MainWindow *ui;

    void clearAudioLevels();
    //qcharts

    QAudioRecorder *recorder;               //音频录音
    QAudioProbe *probe;                     //探测器
    QList<AudioLevel*> m_audioLevels;
    QVector<double> tempBuffer;
    const qint64  displayPointsCount=4000;
    QLineSeries *lineSeries;                //曲线序列

    //Tcp通信变量
    QTcpServer *tcpServer; //监听套接字
    QTcpSocket *tcpSocket; //通信套接字

    QString fileName;//文件名字
    qint64 fileSize;//文件大小
    qint64 sendSize;//已经发了多少数据

    //文件对象
    QFile file;

    //发送数据的函数
    void sendDate();

    QTimer *myTimer;



};

#endif // MAINWINDOW_H
