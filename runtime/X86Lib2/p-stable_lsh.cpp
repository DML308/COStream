// p-stable_lsh.cpp 

#include "p-stable_lsh.h"
//#include <hash_map>
using namespace std;

#define PI 3.1415926
const int MAX=255;
const int dimention=512;//维度
const int hashcount=1;//哈希表的数量
multimap<int,vector<double> > stda[hashcount];//存放向量和名称
double a[1][512];//p-Stable分布（L=2；高斯分布）的随机向量

//static double a[hashcount][dimention];//p-Stable分布（L=2；高斯分布）的随机向量
double w=4.0;//LSH的w
double b=rand()/(w+1);//LSH的随机数b

double AverageRandom(double min,double max);//平均分布
double Normal(double x,double miu,double sigma);//高斯函数
double NormalRandom(double miu,double sigma,double min,double max);//高斯分布
int hashfamily(double f[],double *a_temp,double b_temp,double w_temp);//f为特征，a_temp为a向量，b_temp为b，w_temp为w
int pstableQuery(double *input_temp, double *output_temp);//查询
int pstableInsert(double *input_temp, double *output_temp);//插入
int pstablePreProccess();//预处理


int pstablePreProccess()
{
#ifdef _PPPROCESS_
#define _PPPROCESS_
	for(int j=0;j<hashcount;j++)//产生hashcount个哈希表
	{
		for(int k=0;k<dimention;k++)
		{
			a[j][k]=NormalRandom(0,1,-1.5,1.5);
		}
	}

#endif
	return 0;
}

int pstableInsert(double *input_temp, double *output_temp, int inDim, int outDim){

	vector<double> vo;
	int outputLength = outDim;
	for(int i=0;i<outputLength;i++)
		vo.push_back(*(output_temp+i));
	//哈希过程
	for(int l=0;l<hashcount;l++)//每一次输入的hashcount个key
	{
		int hash_num=hashfamily(input_temp,&a[l][0],b,w);//哈希
		int key=(int)hash_num/w;//哈希表的key
		//哈希存储
		if(stda[l].count(key)<3)
			stda[l].insert(make_pair(key, vo));
	}
	
	return 0;
}

double AverageRandom(double min,double max)//平均分布
{
    int minInteger = (int)(min*10000);
    int maxInteger = (int)(max*10000);
    int randInteger = rand()*rand();
    int diffInteger = maxInteger - minInteger;
    int resultInteger = randInteger % diffInteger + minInteger;
    return resultInteger/10000.0;
}

double Normal(double x,double miu,double sigma) //概率密度函数
{
	return 1.0/sqrt(2*PI*sigma) * exp(-1*(x-miu)*(x-miu)/(2*sigma*sigma));
}
double NormalRandom(double miu,double sigma,double min,double max)//产生正态分布随机数
{
    double x;
    double dScope;
    double y;
    do
	{
		x = AverageRandom(min,max); 
        y = Normal(x, miu, sigma);
        dScope = AverageRandom(0, Normal(miu,miu,sigma));
     }while( dScope > y);

     return x;
}

int hashfamily(double f[],double *a_temp,double b_temp,double w_temp)//哈希函数
{
	double result=b_temp;
	for(int i=0;i<dimention;i++)
	{
		result+=f[i]*(*(a_temp+i));
	}
	return (int)(result/w_temp);//返回哈希结果
}

int pstableQuery(double *input_temp, double *output_temp)
{
	
	int hash_num=0;
	int key[hashcount]={0};
	for(int l=0;l<hashcount;l++)//hashcount个key
	{
		hash_num=hashfamily(input_temp,&a[l][0],b,w);//哈希
		key[l]=(int)hash_num/w;//哈希表的key
	}
	//处理multimap
	multimap<int,vector<double> >::iterator itit;
	int counts[hashcount]={0};//记录数目
	int count=0;	
	vector<double> search_result;
	for(int i=0;i<hashcount;i++)
	{
		counts[i]=stda[i].count(key[i]);//每个哈希表中关键字的个数 
		count=count+counts[i];//总的匹配数目
	}
	if(count==0)
		return 0;
	else{

		int count_temp=count;
		for(int j=0;j<hashcount;j++)
		{
			if(counts[j]>0){	
				itit=stda[j].find(key[j]);
				search_result=(*itit).second;
			}

		}
		for(int k=0;k<search_result.size();k++)
			*(output_temp+k) = search_result[k];
		return 1;
	}

}