#ifndef _CLUSTER_BASICS_H
#define _CLUSTER_BASICS_H
/*���ڶ��������ɼ�Ⱥ��˴���ʱ�õ��ĳ�������Ϣ**/

/*��Ⱥ�����ݴ��������ʱ��*/
#define BOOTTIME 0

/*��Ⱥ�����ݴ���Ĵ���*/
#define BANDWITH 1

/*�趨һ��ͨ�ŵ�������(�������ܣ����ڶ��⣩ͨ�űȵ��Ͻ�)*/
#define COMMUNICATION_FACTOR 65535

/*�������ͨ�űȴ���delt_radio������ںϣ������ں�*/
#define THRESHOD_COMP_COMM_RADIO 0

//�趨MPI�������Ĵ�С
#define MPI_BUFFER_LEN 3000

//ƽ������
#define MINBALANCEFACTOR 0.9 
#define MAXBALANCEFACTOR 1.1

#define HFISSIONBALANCE_FACTOE 1.5


#endif