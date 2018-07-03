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
extern const int dimention;//维度
extern const int hashcount;//哈希表的数量
extern multimap<int,vector<double> > stda[];//存放向量和名称

extern double a[1][512];//p-Stable分布（L=2；高斯分布）的随机向量
extern double w;//LSH的w
extern double b;//LSH的随机数b

extern double AverageRandom(double min,double max);//平均分布
extern double Normal(double x,double miu,double sigma);//高斯函数
extern double NormalRandom(double miu,double sigma,double min,double max);//高斯分布
extern int hashfamily(double f[],double *a_temp,double b_temp,double w_temp);//f为特征，a_temp为a向量，b_temp为b，w_temp为w
extern int pstableQuery(double *input_temp, double *output_temp);//查询
extern int pstableInsert(double *input_temp, double *output_temp, int inDim, int outDim);//插入
extern int pstablePreProccess();//预处理

#endif
