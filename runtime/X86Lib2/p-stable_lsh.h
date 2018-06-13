// p-stable_lsh.h
//
#ifndef _PSTABLELSH_H_
#define _PSTABLELSH_H_

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <map>
//#include <hash_map>
using namespace std;

#define PI 3.1415926
extern const int MAX;
extern const int dimention;//ά��
extern const int hashcount;//��ϣ�������
extern multimap<int,vector<double> > stda[];//�������������

extern double a[1][512];//p-Stable�ֲ���L=2����˹�ֲ������������
extern double w;//LSH��w
extern double b;//LSH�������b

extern double AverageRandom(double min,double max);//ƽ���ֲ�
extern double Normal(double x,double miu,double sigma);//��˹����
extern double NormalRandom(double miu,double sigma,double min,double max);//��˹�ֲ�
extern int hashfamily(double f[],double *a_temp,double b_temp,double w_temp);//fΪ������a_tempΪa������b_tempΪb��w_tempΪw
extern int pstableQuery(double *input_temp, double *output_temp);//��ѯ
extern int pstableInsert(double *input_temp, double *output_temp, int inDim, int outDim);//����
extern int pstablePreProccess();//Ԥ����

#endif
