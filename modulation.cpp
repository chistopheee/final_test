#include "modulation.h"

modulation::modulation()
{

}
void modulation::clear()
{
    for(int i=0;i<samplelength;i++)
    {
     modulatedSignal[i]=0;
    }
}

void modulation::ASKmodulation(int code[])
{
    int n=0;
    int i=1;
    int pos=0;//二进制码元位置
    int temp=N*i;

    //生成调制信号 1:有载波 0：无载波
    i=1;//几个N
    while(pos<length)
    {
        temp=N*i;
        if(code[pos]==1)
        {
            while(n<temp)
            {
                modulatedSignal[n]=A*sin(2*PI*fc*n/fs);
                n++;
            }

        }
        else if(code[pos]==0)
        {
            while(n<temp)
            {
                modulatedSignal[n]=0;
                n++;
            }
        }
        else //其他数 用于检测是否全为0 1
        {
            while(n<temp)
            {
                modulatedSignal[n]=0.5*A*sin(2*PI*fc*n/fs);
                n++;
            }
        }

        i++;
        pos++;
    }
}


void modulation::PSKmodulation(int code[],int phase)
{
    int n=0;
    int i=1;
    int pos=0;//二进制码元位置
 //   int N=20;//一个周期用多少个点表示
    int temp=N*i;
 //   int length=200;//二进制数组长度

    //int sampleNum=2048;//载波数组长度
    //int A=2;//载波幅度
   // int fc=100;
 //   int fs=fc*N;//fs/fc=一个周期多少个点

 //生成载波  1：0相位  0：180°相位

//生成调制信号
    i=1;//几个N
    phase=180;//调制度数
    while(pos<length)
    {
        temp=N*i;
        if(code[pos]==1)
        {
            while(n<temp)
            {
            modulatedSignal[n]=A*sin(2*PI*fc*n/fs);
            n++;
            }

        }
        else if(code[pos]==0)
        {
            while(n<temp)
            {
            modulatedSignal[n]=A*sin(2*PI*fc*n/fs+phase/180*PI);
            n++;
            }
        }
        else //其他数 用于检测是否全为0 1
        {
            while(n<temp)
            {
            modulatedSignal[n]=0.5*A*sin(2*PI*fc*n/fs);
            n++;
            }
        }

        i++;
        pos++;

            }
}


