#ifndef _PAOPAGATOR_H_
#define _PAOPAGATOR_H_


#include "ast.h"
#include "math.h"

//��������
typedef struct{
	char *name;//��¼������
	Node *decl;//��¼����Ķ��壬��idNode�е�����
	int dim;//ά��
	int *dimLen;//ÿһά�ĳ���
	int num;//Ԫ�صĸ���
	Node **element;//��¼ÿ��Ԫ��ֵ: Ŀǰֻ֧�ֶ�ά����
	Bool *nac;//false_ ��ʾֵ��ȷ���ģ�ture��ʾֵ������
	Bool *undefine;//��ȷ��ֵ��
	//Bool extern_static;//���ڱ�ʶ�ڵ��Ƿ���ȫ�ֱ����;�̬����
}constArrayNode;

//����򵥱���
typedef struct{
	Bool undefine;//��ȷ��ֵ��
	Bool nac;//false_ ��ʾֵ��ȷ���ģ�ture��ʾֵ������
	Node *n;//�������ݣ���������decl���ת����id��㱣��,����ֵ������id�ڵ��value��
	//Bool extern_static;//���ڱ�ʶ�ڵ��Ƿ���ȫ�ֱ����;�̬����
}constIdNode;


//����Ľṹ��Ϊ����ṹ�壬�����塢������ȸ��Ͻṹ�ĳ�����������Ƶ�   12.2.13

typedef enum{Prim_Id,Prim_Array,SUE_Id,SUE_Array}SUFieldType ;

typedef struct{
	Node *decl;
	List *SUEfields;  // constSUEFieldNode�������ڽṹ��ĳ�Ա���Ͳ��壬��List��elementֵͳһ��constSUEFieldNode�������з��ദ���ڲ���ʱ�����
}constSUEid; //�ṹ�����

typedef struct{
	char *SUEname;//��¼������
	Node *SUEdecl;//��¼����Ķ��壬��idNode�е�����
	int SUEdim;//ά��
	int *SUEdimLen;//ÿһά�ĳ���
	int SUEnum;//�ṹ��Ԫ�صĸ���
	List **SUEelement;//��¼�ṹ��������ÿ��Ԫ�ص�ÿ����Աֵ�������constSUEFieldNode��
}constSUEarray;//�ṹ������

typedef struct{
	List* SUEid;//�ṹ�������   ��constSUEid��
	List* SUEarray;//�ṹ�������� ��constSUEarray��
}constSUENode;

typedef struct{
	SUFieldType sutyp;
	union{
		constIdNode *idnode;
		constArrayNode *arrayNode;
		constSUEid *sueidNode;
		constSUEarray *suearrayNode;
	}u;
}constSUEFieldNode;

typedef struct{
	SUEtype *type;	//�ṹ������
	constSUENode *SUEnode;	//�ṹ�����͵�ʵ��
}constSUE;

typedef struct{
	List *idFlowList;
	List *arrayFlowList;
	List *SUEFlowList;//constSUE��
}propagatorNode;

PRIVATE List *SUEList=NULL;//�����ռ����еĽṹ�����ͣ�������
/*����ṹ������ƵĽṹ����Ϊֹ*/
PRIVATE List *gProgram = NULL;
PRIVATE List *idList = NULL;//�����ռ����е�id�ڵ㣬����idList�е����ݾ������еĳ����ڵ㣻
PRIVATE List *arrayList = NULL;//���ڴ��������㣻

PRIVATE FlowValue initflow;

PRIVATE inline constIdNode* FindIdNode(Node *node,FlowValue v);
PRIVATE inline constArrayNode *FindArrayNode(Node *node ,FlowValue v);
PRIVATE FlowValue TransformConstFlow(Node *node, FlowValue v, Point p, Bool final);
GLOBAL constIdNode *MakeConstIdNode(Node *node);
GLOBAL int CheckAdcl(Node *node);
GLOBAL constArrayNode *MakeConstArrayNode(int dim, int *lenPerDim, Node *node);
GLOBAL List *MakeFieldList(List *fieldsList);
GLOBAL constSUEid* MakeStructNode(Node *node);
GLOBAL constSUEid *MakeTdefNode(Node *node);
GLOBAL constSUEarray* MakeTdefArrayNode(Node *node);
GLOBAL constSUEid* MakeUnionNode(Node *node);
GLOBAL constSUEarray* MakeStructArrayNode(Node *node);
GLOBAL constSUEarray* MakeUnionArrayNode(Node *node);
GLOBAL constSUEid* MakeEnumNode(Node *node);
GLOBAL constSUEarray* MakeEnumArrayNode(Node *node);
//GLOBAL constSUE* FindSUEtype(Node *type,List *SUElist);
GLOBAL constSUE* FindSUEtype(SUEtype *suetype,List *SUElist);
GLOBAL List *InsertSUEidtoFlow(Node *node,List *SUElist);
GLOBAL List *InsertSUEarraytoFlow(Node *node,List *SUElist);
GLOBAL constSUEid *InitStructid(constSUEid *SUEid ,Node *decl);
GLOBAL FlowValue InitSUENode(Node *node);
GLOBAL FlowValue InsertSUEdecltoFlow(Node *decl,FlowValue v);
GLOBAL  FlowValue TransfromDot(Node *node , FlowValue v);
GLOBAL FlowValue AlterDot(Node *node, Node *value, FlowValue v);
//��������
GLOBAL void RWV_listwalk(List *l,FlowValue v);
GLOBAL void ReplaceWorkVar(Node *node,FlowValue v);
GLOBAL  Node *GetValue(Node *node);
GLOBAL void InitConstArrayNode(int dim, int *lenPerDim, int num, constArrayNode *itemArray, Node *init);

#endif /* ifndef _PAOPAGATOR_H_ */