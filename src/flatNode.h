#ifndef _FLAT_NODE_H_
#define _FLAT_NODE_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>

#ifdef WIN32
#include <direct.h> 
#else  /* UNIX */
#include <unistd.h>
#endif /* WIN32 */

extern "C" {
#include "ast.h"
};


/*************************************************************************/
/*                                                                       */
/*                          SDF nodes                                    */
/*                                                                       */
/*************************************************************************/

class  FlatNode
{
public:
	std::string name; // opeator����
	std::string PreName;//cwb��¼Operator��������ǰ������
	int visitTimes; // ��ʾ�ý���Ƿ��Ѿ������ʹ�

	operatorNode *contents; // ָ��operator(�������������)
	compositeNode *composite; // ָ��operator ���ڵ� composite ��ʵ��չ���ģ�
	operatorNode *oldContents; // ָ��ԭʼoperator
	compositeNode *oldComposite; // ָ��ԭʼoperator ���ڵ� composite  ��Ϊoperator�ṩcomposite�е�param��var�еĲ�����

	int nOut; // �� �� �߸���
	int nIn; // �� �� �߸���


	//���������㷨��,actor���ڵ�place�š�thread�š�thread�е����к�
	int place_id;
	int thread_id, post_thread_id;
	int serial_id;

	std::vector<FlatNode *> outFlatNodes; // �� �� �߸�operator
	std::vector<FlatNode *> inFlatNodes; // �� �� �߸�operator
	
	//std::vector<int> AddPopAtCodeGen;//	zww:20120313,һ�������Ϊ�㣬������Horizontalfissionʱ����������popֵ���ô���������¼
	
	std::vector<int> outPushWeights; // �� �� �߸�Ȩ��
	std::vector<int> inPopWeights; // �� �� �߸�Ȩ��
	std::vector<int> inPeekWeights; // �� �� �߸�Ȩ��

	// Ϊ��ģ��������ɷ������
	std::vector<std::string> outPushString; 
	std::vector<std::string> inPopString; 
	std::vector<std::string> inPeekString;

	std::vector<std::string> pushString; 
	std::vector<std::string> peekString; 

	//�ڵ�work�����ľ�̬������
	long work_estimate;

	// opeator��ssg��flatnodes�е�˳����
	int num;

	//GPU�ڵ㻮��ʱר�ñ��� cwb
	int GPUPart;
	bool BorderFlag;

	// ���²����������չ��
	int currentIn;
	int currentOut;
	int schedMult;
	int schedDivider;
	int uin;
	int label;
	

public:
	FlatNode(operatorNode *node, compositeNode *com, compositeNode *newCom);
	FlatNode(operatorNode *node);//���ع��캯����Ϊ���Ժ��ڹ����µĽڵ��ʱ��ʹ��
	void AddOutEdges(FlatNode *dest);
	void AddInEdges(FlatNode *src);
	std::string GetOperatorName(); // ��ȡ��flatnode����operator��name
	void VisitNode(); // ���ʸý��
	void ResetVistTimes(); // ����visitTimes��Ϣ
	int GetVisitTimes(); // ��ȡ�ý��ķ�����Ϣ
	void SetIOStreams();
};

#endif /* _FLAT_NODE_H_ */