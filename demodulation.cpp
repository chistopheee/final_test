#include "demodulation.h"

demodulation::demodulation()
{

}


void demodulation::ASKDemodulation(double modulatedbuff[])
{
    demobincode.clear();
    int i;
    int temp;//采样位置的幅值
    int N=fs/RB;//一个周期用多少个点表示【可改20对应调制函数】
    //int length = sizeof(modulatedbuff)/sizeof(double)/N;//二进制数组长度【可改8对应调制函数】
    //QString str;

    for(i=0;i<length/N;i++)
    {
        temp=abs(modulatedbuff[fs/4/fc+N*i]);//N/4+N*i//采样位置的幅值
        if(temp>0)
            decode_buff[i]=1;
        else
            decode_buff[i]=0;
       // qDebug()<<decode_buff[i];
        //str=QString("%1").arg(decode_buff[i]);

        demobincode+=QString("%1").arg(decode_buff[i]);
    }
}


void demodulation::PSKDemodulation(double modulatedbuff[])
{
    demobincode.clear();
    int i;
    int temp;//采样位置的幅值
    int N=fs/RB;//一个周期用多少个点表示【可改20对应调制函数】
    //int length = sizeof(modulatedbuff)/sizeof(double)/N;//二进制数组长度【可改8对应调制函数】
    //QString str;

    for(i=0;i<length/N;i++)
    {
        temp=modulatedbuff[fs/4/fc+N*i];//N/4+N*i//采样位置的幅值
        if(temp>0)
            decode_buff[i]=1;
        else
            decode_buff[i]=0;
        //qDebug()<<decode_buff[i];
        //str=QString("%1").arg(decode_buff[i]);

        demobincode+=QString("%1").arg(decode_buff[i]);
    }
}
