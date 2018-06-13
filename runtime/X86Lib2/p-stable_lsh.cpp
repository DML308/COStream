// p-stable_lsh.cpp 

#include "p-stable_lsh.h"
//#include <hash_map>
using namespace std;

#define PI 3.1415926
const int MAX=255;
const int dimention=512;//ά��
const int hashcount=1;//��ϣ�������
multimap<int,vector<double> > stda[hashcount];//�������������
double a[1][512];//p-Stable�ֲ���L=2����˹�ֲ������������

//static double a[hashcount][dimention];//p-Stable�ֲ���L=2����˹�ֲ������������
double w=4.0;//LSH��w
double b=rand()/(w+1);//LSH�������b

double AverageRandom(double min,double max);//ƽ���ֲ�
double Normal(double x,double miu,double sigma);//��˹����
double NormalRandom(double miu,double sigma,double min,double max);//��˹�ֲ�
int hashfamily(double f[],double *a_temp,double b_temp,double w_temp);//fΪ������a_tempΪa������b_tempΪb��w_tempΪw
int pstableQuery(double *input_temp, double *output_temp);//��ѯ
int pstableInsert(double *input_temp, double *output_temp);//����
int pstablePreProccess();//Ԥ����


int pstablePreProccess()
{
#ifdef _PPPROCESS_
#define _PPPROCESS_
	for(int j=0;j<hashcount;j++)//����hashcount����ϣ��
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
	//��ϣ����
	for(int l=0;l<hashcount;l++)//ÿһ�������hashcount��key
	{
		int hash_num=hashfamily(input_temp,&a[l][0],b,w);//��ϣ
		int key=(int)hash_num/w;//��ϣ���key
		//��ϣ�洢
		if(stda[l].count(key)<3)
			stda[l].insert(make_pair(key, vo));
	}
	
	return 0;
}

double AverageRandom(double min,double max)//ƽ���ֲ�
{
    int minInteger = (int)(min*10000);
    int maxInteger = (int)(max*10000);
    int randInteger = rand()*rand();
    int diffInteger = maxInteger - minInteger;
    int resultInteger = randInteger % diffInteger + minInteger;
    return resultInteger/10000.0;
}

double Normal(double x,double miu,double sigma) //�����ܶȺ���
{
	return 1.0/sqrt(2*PI*sigma) * exp(-1*(x-miu)*(x-miu)/(2*sigma*sigma));
}
double NormalRandom(double miu,double sigma,double min,double max)//������̬�ֲ������
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

int hashfamily(double f[],double *a_temp,double b_temp,double w_temp)//��ϣ����
{
	double result=b_temp;
	for(int i=0;i<dimention;i++)
	{
		result+=f[i]*(*(a_temp+i));
	}
	return (int)(result/w_temp);//���ع�ϣ���
}

int pstableQuery(double *input_temp, double *output_temp)
{
	
	int hash_num=0;
	int key[hashcount]={0};
	for(int l=0;l<hashcount;l++)//hashcount��key
	{
		hash_num=hashfamily(input_temp,&a[l][0],b,w);//��ϣ
		key[l]=(int)hash_num/w;//��ϣ���key
	}
	//����multimap
	multimap<int,vector<double> >::iterator itit;
	int counts[hashcount]={0};//��¼��Ŀ
	int count=0;	
	vector<double> search_result;
	for(int i=0;i<hashcount;i++)
	{
		counts[i]=stda[i].count(key[i]);//ÿ����ϣ���йؼ��ֵĸ��� 
		count=count+counts[i];//�ܵ�ƥ����Ŀ
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