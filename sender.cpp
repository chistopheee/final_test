#include "sender.h"

QString QString_to_HexQString(const QString &text)
{
    QTextCodec *gbk = QTextCodec::codecForName("System");
    QString temp;
    QByteArray hex_data;
    foreach(QChar byte, text)
    {
        hex_data.append(gbk->fromUnicode(byte));
    }
    temp = gbk->toUnicode(hex_data.toHex()).toUpper();
    return temp;
}

QString HexQString_to_BinQString(const QString &temp)
{
	QString hexMessage;
	QString bincode;
	bool OK;
    int val;
    for(int i = 0; i <temp.length() ; i+=2)
    {
        hexMessage = temp.mid(i,2) ;
        val = hexMessage.toInt(&OK,16);
        hexMessage = hexMessage.setNum(val,2);
        hexMessage=QString("%1").arg(hexMessage,8,QLatin1Char('0'));
        bincode+=hexMessage;
    }
/*
    for(i=0;i<bincode.length();i++)
    {
        val = bincode.mid(i,1).toInt(&ok,2);
        modu.code_buff[i]=val;
    }
  */

    return bincode;
}
				

