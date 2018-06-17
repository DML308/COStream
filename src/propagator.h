#ifndef _PAOPAGATOR_H_
#define _PAOPAGATOR_H_


#include "ast.h"
#include "math.h"

//处理数组
typedef struct{
	char *name;//记录数组名
	Node *decl;//记录数组的定义，与idNode中的类似
	int dim;//维数
	int *dimLen;//每一维的长度
	int num;//元素的个数
	Node **element;//记录每个元素值: 目前只支持二维数组
	Bool *nac;//false_ 表示值是确定的；ture表示值不定；
	Bool *undefine;//不确定值；
	//Bool extern_static;//用于标识节点是否是全局变量和静态变量
}constArrayNode;

//处理简单变量
typedef struct{
	Bool undefine;//不确定值；
	Bool nac;//false_ 表示值是确定的；ture表示值不定；
	Node *n;//结点的内容；将变量的decl结点转化成id结点保存,并将值保存在id节点的value中
	//Bool extern_static;//用于标识节点是否是全局变量和静态变量
}constIdNode;


//下面的结构是为处理结构体，共用体、联合体等复合结构的常量传播而设计的   12.2.13

typedef enum{Prim_Id,Prim_Array,SUE_Id,SUE_Array}SUFieldType ;

typedef struct{
	Node *decl;
	List *SUEfields;  // constSUEFieldNode链，由于结构体的成员类型不清，故List的element值统一用constSUEFieldNode，不进行分类处理，在查找时需遍历
}constSUEid; //结构体变量

typedef struct{
	char *SUEname;//记录数组名
	Node *SUEdecl;//记录数组的定义，与idNode中的类似
	int SUEdim;//维数
	int *SUEdimLen;//每一维的长度
	int SUEnum;//结构体元素的个数
	List **SUEelement;//记录结构体数组中每个元素的每个成员值的情况，constSUEFieldNode链
}constSUEarray;//结构体数组

typedef struct{
	List* SUEid;//结构体变量链   ，constSUEid链
	List* SUEarray;//结构体数组链 ，constSUEarray链
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
	SUEtype *type;	//结构体类型
	constSUENode *SUEnode;	//结构体类型的实例
}constSUE;

typedef struct{
	List *idFlowList;
	List *arrayFlowList;
	List *SUEFlowList;//constSUE链
}propagatorNode;

PRIVATE List *SUEList=NULL;//用于收集所有的结构体类型，并处理
/*处理结构体所设计的结构到此为止*/
PRIVATE List *gProgram = NULL;
PRIVATE List *idList = NULL;//用于收集所有的id节点，最终idList中的内容就是所有的常量节点；
PRIVATE List *arrayList = NULL;//用于处理数组结点；

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
//函数声明
GLOBAL void RWV_listwalk(List *l,FlowValue v);
GLOBAL void ReplaceWorkVar(Node *node,FlowValue v);
GLOBAL  Node *GetValue(Node *node);
GLOBAL void InitConstArrayNode(int dim, int *lenPerDim, int num, constArrayNode *itemArray, Node *init);

#endif /* ifndef _PAOPAGATOR_H_ */