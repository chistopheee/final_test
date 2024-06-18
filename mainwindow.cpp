#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //lineedit
    ui->lineEdit_RB->setText(QString("%1").arg(DEMOD.RB));
    ui->lineEdit_Fc->setText(QString("%1").arg(DEMOD.fc));
    ui->lineEdit_FS->setText(QString("%1").arg(DEMOD.fs));


    connect(ui->actiondat, &QAction::triggered, this, &MainWindow::on_pb_Openfile_clicked);
    connect(ui->actiondat_2, &QAction::triggered, this, &MainWindow::on_pb_Savefile_clicked);
    connect(ui->actionmodu, &QAction::triggered, this, &MainWindow::on_pb_Savemodu_clicked);

    timer = new QTimer(this);
    timer->setInterval(20);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(DrawLine()));



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initChart(){

    Series =new QSplineSeries();
    Chart = new QChart();
//chart
    Chart->addSeries(Series);//把线添加到chart
    Chart->createDefaultAxes();
    Chart->legend()->hide();//隐藏图例
    Chart->axisX()->setRange(0,200);
    Chart->axisY()->setRange(ymin,ymax);
    ui->widget2->setChart(Chart);//把chart显示到窗口上

}

void MainWindow::DrawLine()
{
    uint32_t length=DEMOD.demobincode.length()*DEMOD.N;//调制信号长度
    vector<double> time_data(0);
    Chart->removeSeries(Series);//将图表中的折线删除
    Series->clear();//将折线中的元素清零
    //初始化波形数据
    vector<double>().swap(time_data);//清空动态数组
    for (uint32_t i=0; i<length; i++)
    {
        time_data.push_back(modulatedSignal[i]);
    }
    for (uint32_t i = 0; i < xmax; i++)
    {
        if (cycle+i >= length)
        {
            Series->append(i, time_data.at(cycle+i-length));
//            Series->append((cycle+i)/MOD.fs, time_data.at(cycle+i-length));
        }
        else
        {
            Series->append(i, time_data.at(cycle+i));
//            Series->append((cycle+i)/MOD.fs, time_data.at(cycle+i));

        }
    }
    cycle++;
    if (cycle == length)
    {
        cycle = 0;
    }

    Chart->axisX()->setRange((double)cycle/DEMOD.fs,(xmax+cycle)/DEMOD.fs);
    Chart->addSeries(Series);//将折线添加到图表
   // axisX_1->setRange((double)count1/fs, (double)(number_1+count1)/fs);
    ui->widget2->setChart(Chart);//把chart显示到窗口上



}





void MainWindow::on_pb_Openfile_clicked()  //打开文件 画图
{
    QString filename = QFileDialog::getOpenFileName(this,"打开文件","./","Files(*.txt)");
    ui->statusBar->showMessage(filename);//在状态栏显示路径
    QFile inputFile(filename);// 打开文件

    timer->stop();
    initChart();//初始化画布
    Chart->removeSeries(Series);//将图表中的折线删除
    xmax=400;
    ymax=1.2;
    ymin=0;
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->textEdit_3->clear();
    signalData.clear();
    dataLine.clear();
    i=0;
    if (inputFile.open(QIODevice::ReadOnly)) //文件打开成功
    {

        QTextStream inputFileRead(&inputFile);
        inputFileRead.seek(0);
//        i=0;
        while(!inputFileRead.atEnd())//如果没有读到结尾
        {
            inputFileRead>>modulatedSignal[i];

            signalData+=(QString("%1").arg(modulatedSignal[i])+'\t');
            i++;
//            dataLine=dataLine+inputFileRead.readLine();//每行数据的累加就是完整的数据部分
        }
        ui->textEdit->setText(signalData);//将文件全部内容显示在框里
        DEMOD.length=i+1;
        DEMOD.ASKDemodulation(modulatedSignal);
        //bincode=DEMOD.demobincode;
//      DEMOD.length=bincode.length();
        ui->textEdit_2->setText(DEMOD.demobincode);//二进制数据字符

        hexcode=BinQString_to_HexQString(DEMOD.demobincode);


        dataLine=HexQString_to_QString(hexcode);
        ui->textEdit_3->setText(dataLine);
//        QString temp;
//        temp=QString_to_HexQString(dataLine);//temp存放16进制
//        bincode=HexQString_to_BinQString(temp);

//        bool ok;
//        for(i=0;i<bincode.length();i++)
//        {
//            code_buff[i]=bincode.mid(i,1).toInt(&ok,2);
//            //  signalData=signalData+QString("%1").arg(code_buff[i])+'\t';

//        }
//        ui->textEdit_3->setText(signalData);//载波数据数组

      //  xmax=DEMOD.demobincode.length()*DEMOD.N;
        //确定y的范围
        for(i=0;i<xmax;i++) {
            if(modulatedSignal[i]>ymax)
                ymax=modulatedSignal[i];
            if(modulatedSignal[i]<ymin)
                ymin=modulatedSignal[i];
        }
        //绘图 确定数据
//        for(i=0;i<xmax;++i) {
//            Series->append(i*1.0/DEMOD.fs,modulatedSignal[i]);//x,y
//        }
        timer->start();
        Chart->axisX()->setRange(0,400.0/DEMOD.fs);
        Chart->axisY()->setRange(ymin,ymax);
        ui->widget2->setChart(Chart);//把chart显示到窗口上
        /*

        }
        else
        {
           QMessageBox::warning(this,"警告","每行样本字数错误，文件打开失败");
        }


        */
    }
    else
    {
        QMessageBox::warning(this,"错误","文件打开失败");
        return;
    }
    inputFile.close();

}



void MainWindow::on_pb_Savefile_clicked()//保存文件
{
    QString textContain=ui->textEdit->toPlainText();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./ASK调制文件","Files(*.txt)");
    QFile outputFile(filename);
    if(!outputFile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        QMessageBox::warning(this,"错误","文件保存失败");
    }
    else
    {
        QTextStream outputFileSave(&outputFile);

        outputFileSave<<textContain;
        QMessageBox::information(this,"提示","数据保存成功");
    }
    outputFile.close();

}




void MainWindow::on_pb_Savemodu_clicked()
{
    QString textContain=ui->textEdit_3->toPlainText();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./4解调文本","Files(*.txt)");
    QFile outputFile(filename);
    if(!outputFile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        QMessageBox::warning(this,"错误","文件保存失败");
    }
    else
    {
        QTextStream outputFileSave(&outputFile);
        outputFileSave.setCodec("UTF-8");
        outputFileSave<<textContain;
        QMessageBox::information(this,"提示","文本保存成功");
    }
    outputFile.close();
}


//lineedit编辑参数
void MainWindow::on_lineEdit_Fc_returnPressed()
{
    //QString mString = ui->lineEdit_Fc->text();
    //QMessageBox::about(this, "About", mString);
    DEMOD.fc=ui->lineEdit_Fc->text().toInt();
    //MOD.N=MOD.fs/MOD.RB;
}

void MainWindow::on_lineEdit_RB_returnPressed()
{
    DEMOD.RB=ui->lineEdit_RB->text().toInt();
    DEMOD.N=DEMOD.fs/DEMOD.RB;
}

void MainWindow::on_lineEdit_FS_returnPressed()
{
    DEMOD.fs=ui->lineEdit_FS->text().toInt();
    DEMOD.N=DEMOD.fs/DEMOD.RB;
}
// //移动横坐标
//void MainWindow::on_horizontalScrollBar_valueChanged(int value)
//{
//      Chart->axisX()->setRange(value/100.0*xmax/MOD.fs,value/100.0*xmax/MOD.fs+400.0/MOD.fs);
//}

void MainWindow::on_pb_OpenPSKfile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"打开文件","./","Files(*.txt)");//获取路径 也叫filepath
    ui->statusBar->showMessage(filename);//在状态栏显示路径
    QFile inputFile(filename);//创建文件对象 打开文件

    timer->stop();
    initChart();//初始化画布
    Chart->removeSeries(Series);//将图表中的折线删除
    xmax=400;
    ymax=1.2;
    ymin=0;
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->textEdit_3->clear();
    signalData.clear();
    dataLine.clear();
    i=0;
//    bincode.clear();//二进制数据的字符串
//    hexcode.clear();
//    memset(code_buff,0,sizeof (code_buff));
//    memset(modulatedSignal,0,sizeof (modulatedSignal));
//    memset(DEMOD.decode_buff,0,sizeof (DEMOD.decode_buff));
//    memset(DEMOD.modulatedSignal,0,sizeof (DEMOD.modulatedSignal));

    if (inputFile.open(QIODevice::ReadOnly)) //文件打开成功
    {

        QTextStream inputFileRead(&inputFile);
        inputFileRead.seek(0);
//        i=0;
        while(!inputFileRead.atEnd())//如果没有读到结尾
        {
            inputFileRead>>modulatedSignal[i];
            signalData+=(QString("%1").arg(modulatedSignal[i])+'\t');
            i++;
//            dataLine=dataLine+inputFileRead.readLine();//每行数据的累加就是完整的数据部分
        }
        ui->textEdit->setText(signalData);//将文件全部内容显示在框里
        DEMOD.length=i+1;
        DEMOD.PSKDemodulation(modulatedSignal);
        //bincode=DEMOD.demobincode;
//      DEMOD.length=bincode.length();
        ui->textEdit_2->setText(DEMOD.demobincode);//二进制数据字符

        hexcode=BinQString_to_HexQString(DEMOD.demobincode);


        dataLine=HexQString_to_QString(hexcode);
        ui->textEdit_3->setText(dataLine);

        //确定y的范围
        for(i=0;i<xmax;i++) {
            if(modulatedSignal[i]>ymax)
                ymax=modulatedSignal[i];
            if(modulatedSignal[i]<ymin)
                ymin=modulatedSignal[i];
        }
        //绘图 确定数据
//        for(i=0;i<xmax;++i) {
//            Series->append(i*1.0/DEMOD.fs,modulatedSignal[i]);//x,y
//        }
        timer->start();
        Chart->axisX()->setRange(0,400.0/DEMOD.fs);
        Chart->axisY()->setRange(ymin,ymax);
        ui->widget2->setChart(Chart);//把chart显示到窗口上

    }
    else
    {
        QMessageBox::warning(this,"错误","文件打开失败");
        return;
    }
    inputFile.close();
}

void MainWindow::on_pb_SavePSKfile_clicked()
{
    QString textContain=ui->textEdit->toPlainText();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./PSK调制文件","Files(*.txt)");
    QFile outputFile(filename);
    if(!outputFile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        QMessageBox::warning(this,"错误","文件保存失败");
    }
    else
    {
        QTextStream outputFileSave(&outputFile);

        outputFileSave<<textContain;
        QMessageBox::information(this,"提示","数据保存成功");
    }
    outputFile.close();
}
