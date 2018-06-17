#ifndef _CLUSTER_BASICS_H
#define _CLUSTER_BASICS_H
/*用于定义在生成集群后端代码时用到的常量等信息**/

/*集群间数据传输的启动时间*/
#define BOOTTIME 0

/*集群间数据传输的带宽*/
#define BANDWITH 1

/*设定一个通信调整因子(可以忍受（对内对外）通信比的上界)*/
#define COMMUNICATION_FACTOR 65535

/*如果计算通信比大于delt_radio则进行融合，否则不融合*/
#define THRESHOD_COMP_COMM_RADIO 0

//设定MPI缓冲区的大小
#define MPI_BUFFER_LEN 3000

//平衡因子
#define MINBALANCEFACTOR 0.9 
#define MAXBALANCEFACTOR 1.1

#define HFISSIONBALANCE_FACTOE 1.5


#endif