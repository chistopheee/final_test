#include "receiver.h"


QString HexQString_to_QString(const QString &text)
{
    QTextCodec *tc = QTextCodec::codecForName("System");
    QString temp;
    QByteArray ascii_data;
    for(int i = 0; i <text.length() ; i+=8)
    {
    temp=text.mid(i,8);
    ascii_data.append(temp);
    }
    temp = tc->toUnicode(QByteArray::fromHex(ascii_data));
	return temp;
}

QString BinQString_to_HexQString(const QString &temp)
{
	QString hexMessage;
    QString hexcode;
    bool OK;
    int val;
    for(int i = 0; i <temp.length() ; i+=8)
    {
        hexMessage = temp.mid(i,8);
        val = hexMessage.toInt(&OK,2);
        hexMessage = hexMessage.setNum(val,16);
        hexMessage=QString("%1").arg(hexMessage,2,QLatin1Char('0'));
        hexcode+=hexMessage;
    }
    return hexcode;
}

