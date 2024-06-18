#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtCharts>
#include <QChartView>

using namespace QtCharts;
//using namespace QChartView;

#define cout qDebug() << "[" << __FILE__ <<":" << __LINE__ << "]"

// 返还给定音频格式信号的采样峰值
qreal getPeakValue(const QAudioFormat& format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

template <class T>
QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

// returns the audio level for each channel
QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //lineedit
    ui->lineEdit_RB->setText(QString("%1").arg(MOD.RB));
    ui->lineEdit_Fc->setText(QString("%1").arg(MOD.fc));
    ui->lineEdit_FS->setText(QString("%1").arg(MOD.fs));


    connect(ui->actiondat, &QAction::triggered, this, &MainWindow::on_pb_Openfile_clicked);
    connect(ui->actiondat_2, &QAction::triggered, this, &MainWindow::on_pb_Savefile_clicked);
    connect(ui->actionmodu, &QAction::triggered, this, &MainWindow::on_pb_Savemodu_clicked);
    connect(ui->actionbao, &QAction::triggered, this, &MainWindow::on_pb_SavemoduPsk_clicked);

    //connect(timer,SIGNAL(timeout()),this,SLOT(DrawLine()));
    timer = new QTimer(this);
    timer->setInterval(20);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(DrawLine()));



    //ui->buttonFile->setEnabled(false);
    //ui->buttonSend->setEnabled(false);


   //........................................//

    //创建显示图表
    QChart *chart = new QChart;
    chart->setTitle("音频数据波形图");
    ui->chartView->setChart(chart);

    //序列
    lineSeries= new QLineSeries();
    chart->addSeries(lineSeries);

    QValueAxis *axisX = new QValueAxis;  //坐标轴
    axisX->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX->setLabelFormat("%g");
    axisX->setTitleText("FrameCount");

    QValueAxis *axisY = new QValueAxis;  //坐标轴
    axisY->setRange(-1, 1);
    axisY->setTitleText("Audio level");

    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    chart->legend()->hide();

    recorder = new QAudioRecorder(this);
    probe = new QAudioProbe;
    probe->setSource(recorder);

    //音频缓冲区探测，buffer中包含了缓冲区信息和音频原始数据
    connect(probe, &QAudioProbe::audioBufferProbed, this, [=](QAudioBuffer buffer){

        /*ui->spin_byteCount->setValue(buffer.byteCount());             //缓冲区字节数
        ui->spin_duration->setValue(buffer.duration()/1000);          //缓冲区时长
        ui->spin_frameCount->setValue(buffer.frameCount());           //缓冲区帧数
        ui->spin_sampleCount->setValue(buffer.sampleCount());         //缓冲区采样数*/

        QAudioFormat audioFormat=buffer.format();                     //缓冲区格式
        /*ui->spin_channelCount->setValue(audioFormat.channelCount());  //通道数
        ui->spin_sampleSize->setValue(audioFormat.sampleSize());      //采样大小
        ui->spin_sampleRate->setValue(audioFormat.sampleRate());      //采样率
        ui->spin_bytesPerFrame->setValue(audioFormat.bytesPerFrame());//每帧字节数*/

        /*if (audioFormat.byteOrder()==QAudioFormat::LittleEndian)
            ui->edit_byteOrder->setText("LittleEndian");              //字节序
        else
            ui->edit_byteOrder->setText("BigEndian");

        ui->edit_codec->setText(audioFormat.codec());                 //编码格式

        if (audioFormat.sampleType()==QAudioFormat::SignedInt)        //采样点类型
            ui->edit_sampleType->setText("SignedInt");
        else if(audioFormat.sampleType()==QAudioFormat::UnSignedInt)
            ui->edit_sampleType->setText("UnSignedInt");
        else if(audioFormat.sampleType()==QAudioFormat::Float)
            ui->edit_sampleType->setText("Float");
        else
            ui->edit_sampleType->setText("Unknown");*/

        if (m_audioLevels.count() != buffer.format().channelCount()) {
            qDeleteAll(m_audioLevels);
            m_audioLevels.clear();
            for (int i = 0; i < buffer.format().channelCount(); ++i) {
                AudioLevel *level = new AudioLevel();
                m_audioLevels.append(level);
                ui->levelsLayout->addWidget(level);
            }
        }

        QVector<qreal> levels = getBufferLevels(buffer);
        for (int i = 0; i < levels.count(); ++i)
            m_audioLevels.at(i)->setLevel(levels.at(i));

        //chart
        int nFrameCount = buffer.frameCount();

        if(buffer.format().sampleType() == QAudioFormat::UnSignedInt)
        {
            if (buffer.format().sampleSize() == 8)
            {
                quint8 *data = buffer.data<quint8>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else if(buffer.format().sampleSize() == 16)
            {
                quint16 *data = buffer.data<quint16>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else if(buffer.format().sampleSize() == 32)
            {
                quint32 *data = buffer.data<quint32>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else
                return;
        }
        else if(buffer.format().sampleType() == QAudioFormat::SignedInt)
        {
            if (buffer.format().sampleSize() == 8)
            {
                qint8 *data = buffer.data<qint8>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else if(buffer.format().sampleSize() == 16)
            {
                qint16 *data = buffer.data<qint16>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else if(buffer.format().sampleSize() == 32)
            {
                qint32 *data = buffer.data<qint32>();
                const qreal max = getPeakValue(buffer.format());
                tempBuffer.fill(0, nFrameCount);
                for (int i=0;i < nFrameCount;i++)
                    tempBuffer[i] = data[i]/max;
            }
            else
                return;
        }
        else
        {
            return;
        }

        QVector<QPointF> oldPoints = lineSeries->pointsVector();
        QVector<QPointF> points;

        if (oldPoints.count() < displayPointsCount)
        {
            if(displayPointsCount - oldPoints.count() >= nFrameCount)
            {
                points = lineSeries->pointsVector();
            }
            else
            {
                int nMoveCount = nFrameCount - (displayPointsCount - oldPoints.count());
                for (int i = nMoveCount; i < oldPoints.count(); i++)
                    points.append(QPointF(i - nMoveCount, oldPoints.at(i).y()));
            }
        }
        else
        {
            for (int i = nFrameCount; i < oldPoints.count(); i++)
                points.append(QPointF(i - nFrameCount, oldPoints.at(i).y()));
        }

        qint64 size = points.count();
        for (int k = 0; k < nFrameCount; k++)
            points.append(QPointF(k + size,tempBuffer[k]));

        lineSeries->replace(points);
    });

    //recorder状态发生变化时发射信号：录音、暂停、停止等状态变化
    connect(recorder, &QAudioRecorder::stateChanged, this, [=](QMediaRecorder::State state){
        ui->pbStart->setEnabled(state != QMediaRecorder::RecordingState);
        ui->pbStop->setEnabled(state == QMediaRecorder::RecordingState);
        ui->pbCom->setEnabled(state == QMediaRecorder::RecordingState);

        ui->btnGetFile->setEnabled(state == QMediaRecorder::StoppedState);
        ui->editOutputFile->setEnabled(state == QMediaRecorder::StoppedState);
    });

    connect(recorder, &QAudioRecorder::statusChanged, this, [=](QMediaRecorder::Status status){
        QString statusMessage;

        switch (status) {
        case QMediaRecorder::RecordingStatus:
            statusMessage = tr("Recording to %1").arg(recorder->actualLocation().toString());
            break;
        case QMediaRecorder::PausedStatus:
            clearAudioLevels();
            statusMessage = tr("Paused");
            break;
        case QMediaRecorder::UnloadedStatus:
        case QMediaRecorder::LoadedStatus:
            clearAudioLevels();
            statusMessage = tr("Stopped");
        default:
            break;
        }

        /*if (recorder->error() == QMediaRecorder::NoError)
            ui->statusbar->showMessage(statusMessage);*/
    });

    //录制的持续时间变化时发射
    connect(recorder, &QAudioRecorder::durationChanged, this, [=](qint64 duration){
        ui->LabPassTime->setText(QString("已录制 %1 秒").arg(duration / 1000));
    });

    /*connect(recorder, QOverload<QMediaRecorder::Error>::of(&QAudioRecorder::error), this, [=](){
        ui->statusbar->showMessage(recorder->errorString());
    });*/

    //判断是否有音频输入设备
    if(recorder->defaultAudioInput().isEmpty())
        return;

    //获取音频输入设备列表
    foreach (const QString &device, recorder->audioInputs())
        ui->comboDevices->addItem(device);

    //获取支持的音频编码
    foreach (const QString &codecName, recorder->supportedAudioCodecs())
        ui->comboCodec->addItem(codecName);

    //获取支持的采样率
    foreach (int sampleRate, recorder->supportedAudioSampleRates())
        ui->comboSampleRate->addItem(QString::number(sampleRate));

    //channels：单声道、立体声、四声环绕
    ui->comboChannels->addItem("1");
    ui->comboChannels->addItem("2");
    ui->comboChannels->addItem("4");

/*
    //quality
    ui->sliderQuality->setRange(0, int(QMultimedia::VeryHighQuality));
    ui->sliderQuality->setValue(int(QMultimedia::NormalQuality));

    //bitrates:
    ui->comboBitrate->addItem("32000");
    ui->comboBitrate->addItem("64000");
    //ui->comboBitrate->addItem("96000");
    ui->comboBitrate->addItem("128000");
    */

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initChart()
{

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
    uint32_t length=bincode.length()*MOD.N;//调制信号长度
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

    Chart->axisX()->setRange((double)cycle/MOD.fs,(xmax+cycle)/MOD.fs);
    Chart->addSeries(Series);//将折线添加到图表
   // axisX_1->setRange((double)count1/fs, (double)(number_1+count1)/fs);
    ui->widget2->setChart(Chart);//把chart显示到窗口上

}



void MainWindow::on_pb_Openfile_clicked()  //打开文件并画图
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

    if (inputFile.open(QIODevice::ReadOnly)) //文件打开成功
    {

        QTextStream inputFileRead(&inputFile);
        //支持中文
        QTextCodec *codec=QTextCodec::codecForName("UTF-8");//GBK转UTF8显示
        inputFileRead.setCodec(codec);

        inputFileRead.seek(0);
        while(!inputFileRead.atEnd())//如果没有读到结尾
        {
            dataLine=dataLine+inputFileRead.readLine()+'\n';//每行数据的累加就是完整的数据部分
        }
        ui->textEdit->setText(dataLine);//将文件全部内容显示在框里

        QString temp;
        temp=QString_to_HexQString(dataLine);//temp存放16进制
        bincode=HexQString_to_BinQString(temp);
        //MOD.length=bincode.length();
        ui->textEdit_2->setText(bincode);//二进制数据字符
        bool ok;
        for(i=0;i<bincode.length();i++)
        {
            code_buff[i]=bincode.mid(i,1).toInt(&ok,2);
            //  signalData=signalData+QString("%1").arg(code_buff[i])+'\t';

        }
        ui->textEdit_3->setText(signalData);//载波数据数组


        //确定y的范围
        for(i=0;i<xmax;i++) {
            if(modulatedSignal[i]>ymax)
                ymax=modulatedSignal[i];
            if(modulatedSignal[i]<ymin)
                ymin=modulatedSignal[i];
        }
        //绘图 确定数据
        for(i=0;i<xmax;++i) {
            Series->append(i,modulatedSignal[i]);//x,y
        }


        Chart->axisX()->setRange(0,xmax);
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
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./文本文件","Files(*.txt)");
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


void MainWindow::on_pb_StartmoduAsk_clicked()
{
    MOD.fc=ui->lineEdit_Fc->text().toInt();
    MOD.RB=ui->lineEdit_RB->text().toInt();
    MOD.fs=ui->lineEdit_FS->text().toInt();
    MOD.N=MOD.fs/MOD.RB;
    MOD.clear();
    MOD.length=bincode.length();
    MOD.ASKmodulation(code_buff);
    for(i=0;i<bincode.length()*MOD.N;i++) {
         modulatedSignal[i]=MOD.modulatedSignal[i];
    }


 //画图
    initChart();
    xmax=400;
    ymax=1.2;
    ymin=0;
    //确定y的范围
   for(i=0;i<xmax;i++) {
         if(modulatedSignal[i]>ymax)
             ymax=modulatedSignal[i];
         if(modulatedSignal[i]<ymin)
             ymin=modulatedSignal[i];
   }
   //绘图 确定数据
     timer->start();
 //  for(i=0;i<xmax;++i) {
 //        Series->append(i*1.0/MOD.fs,modulatedSignal[i]);//x,y
 //     }
 //  Chart->axisX()->setRange(0,400.0/MOD.fs);


   Chart->axisX()->setRange(0,xmax/MOD.fs);
   Chart->axisY()->setRange(ymin,ymax);
   Chart->setTitle("ASK调制信号");
   ui->widget2->setChart(Chart);//把chart显示到窗口上


  //显示
   ui->textEdit_3->clear();
   signalData.clear();
   for(i=0;i<bincode.length()*MOD.N;i++) {
       signalData=signalData+QString("%1").arg(MOD.modulatedSignal[i])+'\t';
   }

   //qDebug()<<bincode.length();
   qDebug()<<i;
   ui->textEdit_3->setText(signalData);//载波数据数组

}


void MainWindow::on_pb_Savemodu_clicked()
{
    QString textContain=ui->textEdit_3->toPlainText();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./2ASK调制文件","Files(*.txt)");
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

void MainWindow::on_pb_SavemoduPsk_clicked()
{

    QString textContain=ui->textEdit_3->toPlainText();
    QString filename = QFileDialog::getSaveFileName(this,"保存文件","./2PSK调制文件","Files(*.txt)");
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

void MainWindow::on_pb_StartmoduPsk_clicked()
{

//    on_pb_StartmoduAsk_clicked();
    //modulation MOD;
    MOD.fc=ui->lineEdit_Fc->text().toInt();
    MOD.RB=ui->lineEdit_RB->text().toInt();
    MOD.fs=ui->lineEdit_FS->text().toInt();
    MOD.N=MOD.fs/MOD.RB;
    MOD.clear();
    MOD.length=bincode.length();
    MOD.PSKmodulation(code_buff,180);//PSK调制
    for(i=0;i<bincode.length()*MOD.N;i++) {
        modulatedSignal[i]=MOD.modulatedSignal[i];
    }


    //画图
    initChart();
    xmax=400;
    ymax=1.2;
    ymin=0;
    //确定y的范围
    for(i=0;i<xmax;i++) {
        if(modulatedSignal[i]>ymax)
            ymax=modulatedSignal[i];
        if(modulatedSignal[i]<ymin)
            ymin=modulatedSignal[i];
    }
    //显示
    ui->textEdit_3->clear();
    signalData.clear();
    for(i=0;i<bincode.length()*MOD.N;i++) {
        signalData=signalData+QString("%1").arg(MOD.modulatedSignal[i])+'\t';
    }

    //qDebug()<<bincode.length();
    qDebug()<<i;
    ui->textEdit_3->setText(signalData);//载波数据数组

    //绘图 确定数据
    timer->start();

    Chart->axisX()->setRange(0,xmax/MOD.fs);
    Chart->axisY()->setRange(ymin,ymax);
    Chart->setTitle("PSK调制信号");
    ui->widget2->setChart(Chart);//把chart显示到窗口上
}







//清空音频文件
void MainWindow::clearAudioLevels()
{
    for (int i = 0; i < m_audioLevels.size(); ++i)
        m_audioLevels.at(i)->setLevel(0);
}

//选择录音输出文件
void MainWindow::on_btnGetFile_clicked()
{
    QString curPath = QDir::homePath();
    QString dlgTitle = "设置音频输出文件";
    QString filter = "wav文件(*.wav)";
    QString selectedFile = QFileDialog::getSaveFileName(this, dlgTitle, curPath, filter);
    if (!selectedFile.isEmpty())
        ui->editOutputFile->setText(selectedFile);
}

/*
//开始录音
void MainWindow::on_actRecord_triggered()
{

}

//暂停录音
void MainWindow::on_actPause_triggered()
{
    recorder->pause();
}

//停止录音
void MainWindow::on_actStop_triggered()
{
    recorder->stop();
}
*/

//打开文件 获取文件的基本信息
void MainWindow::on_buttonFile_clicked()
{

}

//先发送文件信息 再发送数据
void MainWindow::on_buttonSend_clicked()
{

}

//数据发送函数

void MainWindow::on_pb_play_clicked()
{
    QMediaPlayer *player = new QMediaPlayer;
    QString file_name = QFileDialog::getOpenFileName(this,"打开文件","test","*(*.wav)");
    player->setMedia(QUrl::fromLocalFile(file_name));
    player->setVolume(50); //0~100音量范围,默认是100
    player->play();
}

//开始录音
void MainWindow::on_pbStart_clicked()
{
    if (recorder->state() == QMediaRecorder::StoppedState)
    {
        QString selectedFile = ui->editOutputFile->text().trimmed();
        if (selectedFile.isEmpty())
        {
            QMessageBox::critical(this,"错误","请先设置录音输出文件");
            return;
        }

        if (QFile::exists(selectedFile) && !QFile::remove(selectedFile))
        {
            QMessageBox::critical(this,"错误","所设置录音输出文件被占用，无法删除");
            return;
        }

        //音频录入设置：

        //设置音频输出文件
        recorder->setOutputLocation(QUrl::fromLocalFile(selectedFile));
        //选择录入设备
        recorder->setAudioInput(ui->comboDevices->currentText());
        //音频编码设置
        QAudioEncoderSettings settings;
        settings.setCodec(ui->comboCodec->currentText());                              //编码
        settings.setSampleRate(ui->comboSampleRate->currentText().toInt());            //采样率
        settings.setBitRate(32000);                                                     //比特率
        settings.setChannelCount(ui->comboChannels->currentText().toInt());            //通道数
        settings.setQuality(QMultimedia::EncodingQuality(int(QMultimedia::NormalQuality))); //品质
        settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);


        recorder->setAudioSettings(settings);
    }

    recorder->record();
}


//暂停录音
void MainWindow::on_pbStop_clicked()
{
    recorder->pause();
}

//停止录音
void MainWindow::on_pbCom_clicked()
{
    recorder->stop();
}
