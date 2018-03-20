/***********************--------------Define For SPL----------****************************/
#pragma ident "unfold.c,v 2.0 2011/11/23 14:46"

#include "ast.h"
/****************************************
��Ҫͨ�������������SplitJoin, StreamIf, 
StreamFor, Pipeline�ڵ��չ����
*****************************************/

GLOBAL Node *gCurrentInputStreamNode = NULL; // ��ǰSplitJoin,PipeLine�ڵ�� ���� ��
GLOBAL Node *gCurrentOutputStreamNode = NULL; // ��ǰSplitJoin,PipeLine�ڵ�� ��� ��
GLOBAL List *gCurrentCompositeCallList = NULL; // ��ǰSplitJoin,PipeLine�ڵ���õ�composite�б�
GLOBAL List *gCurrentParamList[64] = {NULL}; // ��ǰ˫��SP�ṹ���ܻ��õ��Ĳ����б�
GLOBAL List *gCurrentDeclList[64] = {NULL}; // ��ǰ˫��SP�ṹ���ܻ��õ��Ĳ��������б�
GLOBAL List *gCurrentSplitList = NULL; // ��ǰSplitJoin�ڵ��Split
GLOBAL List *gCurrentJoinList = NULL; // ��ǰSplitJoin�ڵ��Join
GLOBAL int gMultiSPFlag = 0; // ��ǰpipeline���
GLOBAL int gLevelPipeline = 0; // ��ǰpipeline���
GLOBAL int gLevelSplitjoin = 0; // ��ǰSplitJoin���
GLOBAL Bool gIsInSplitJoin = FALSE; // ��ǰչ���Ƿ���SplitJoin�ڵ�
GLOBAL Bool gIsInPipeline = FALSE; // ��ǰչ���Ƿ���Pipeline�ڵ�
GLOBAL Bool gIsUnfold = FALSE; // ����չ���ڵ�״̬����֪DefineComposite��Ҫ����ĳЩ����
GLOBAL Bool gIsRoundrobin = FALSE; // ��ǰSplitJoin�ڵ��Split�ķ���Ƿ���Roundrobin
GLOBAL Bool gIsDuplicate = FALSE; // ��ǰSplitJoin�ڵ��Split�ķ���Ƿ���Duplicate
PRIVATE int  gCurrentCompositeNum = 0; // ����Ҫ��SplitJoin,PipeLine�ڵ�������ͼģʽ�����Ҫ����ͼȡ���֣��ñ���������ʶ��ţ���ֹ����
PRIVATE int  gMultiSPCompositeNum = 0; // ��������splitjoin��pipeline�ṹ������composite����
PRIVATE char  *gSplitJoinName = "splitjoin"; // ��SplitJoin��ͼ������ǰ׺��������ž���ʵ������
PRIVATE char  *gPipelineName = "pipeline"; // ��Pipeline��ͼ������ǰ׺��������ž���ʵ������

PRIVATE Node *MakeMyDecl(const char *name, Node *type, Node *init, ScopeState declStyle);
GLOBAL Node *MakeNewStream(const char *name, Node *copyStream);

PRIVATE  inline void ResetUnfoldFlags()
{
	gCurrentInputStreamNode = NULL;
	gCurrentOutputStreamNode = NULL; 
	gCurrentCompositeCallList = NULL; 
	//gCurrentSplitList = NULL; 
	//gCurrentJoinList = NULL; 
	//gIsInSplitJoin = FALSE; 
	//gIsInPipeline = FALSE; 
	//gIsUnfold = FALSE; 
	//gIsRoundrobin = FALSE; 
	//gIsDuplicate = FALSE; 
}

GLOBAL inline void ResetCurrentCompositeNum()
{
	gCurrentCompositeNum = 0;
}

GLOBAL inline void IncCurrentCompositeNum()
{
	++gCurrentCompositeNum;
}

GLOBAL  inline char *MakeCompositeName(const char *name)
{
	char *newName = (char *)malloc(strlen(name) + 20); // �ܱ�ʾ���������Ӧ�ò�����20λ

	assert(name);
	sprintf(newName, "%s_%d", name, gCurrentCompositeNum);

	return newName;
}

PRIVATE Node *MakeCompositeHead(const char *name, Node *output, Node *input)
{
	assert(name && output && input && (output->typ == Id || output->typ == Decl) && input->typ == Id); // 2012.02.21

	input = MakeMyDecl(input->u.id.text, input->u.id.decl->u.decl.type, NULL, Commal);
	if(output->typ == Id)
		output = MakeMyDecl(output->u.id.text, output->u.id.decl->u.decl.type, NULL, Commal);
	
	{
		Node *id = MakeIdCoord(name, UnknownCoord);
		Node *comInOut = MakeComInOutCoord(EMPTY_TQ, MakeNewList(input), MakeNewList(output), UnknownCoord);
		Node *comDecl = SetDeclType(ModifyDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, comInOut, UnknownCoord)),
									MakeDefaultPrimType(EMPTY_TQ, UnknownCoord), 
									Redecl);
		return comDecl;
	}
}

PRIVATE Node *MakeCompositeBody(List *operators)
{
	List *comstmts = NULL;
	Node *comBody = NULL;
	assert(operators);

	{
		comstmts = operators;
		comBody = MakeComBodyCoord(PrimVoid, NULL, NULL, comstmts, UnknownCoord, UnknownCoord);

		return comBody;
	}
}

GLOBAL Node *MakeOperatorHead(const char *name, List *outputs, List *inputs)
{
	assert(name && outputs && inputs);

	{
		Node *id = MakeIdCoord(name, UnknownCoord);
		Node *operHead = ModifyDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, outputs, inputs, NULL, UnknownCoord) );

		return operHead;
	}
}

GLOBAL Node *MakeOperatorBody(Node *work, List *windows)
{
	assert(work);

	{
		Node *operBody = MakeOperBodyCoord(PrimVoid, NULL, NULL, work, windows, UnknownCoord, UnknownCoord);
		
		return operBody;
	}
}

GLOBAL Node *MakeNewStream(const char *name, Node *copyStream)
{
	Node *newStream = NULL;

	assert(name && copyStream && copyStream->typ == STRdcl);

	copyStream = NodeCopy(copyStream, Subtree);
	copyStream->coord = UnknownCoord;

	assert(Level == 0 || Level == 1); // ��ҪΪ������DECL_LOCATION
	if (Level == 0)
		newStream = MakeMyDecl(name, copyStream, NULL, Commal);
	else
		newStream = MakeMyDecl(name, copyStream, NULL, Redecl);

	return newStream;
}

GLOBAL Node *MakeNewStreamId(const char *name, Node *decl)
{
	Node *id = NULL;
	
	assert(decl);
	if(decl->typ == Id) return decl;
	id = MakeIdCoord(name, UnknownCoord);
	assert(name && decl && decl->u.decl.type && decl->u.decl.type->typ == STRdcl);
	id->u.id.decl = decl;


	return id;
}

GLOBAL Node *MakeMyDecl(const char *name, Node *type, Node *init, ScopeState declStyle)
{
	Node *id = NULL, *tmp = NULL;

	assert(name && type);
	id = MakeIdCoord(name, UnknownCoord);
	tmp = SetDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), type, declStyle);
	REFERENCE(tmp);

	return SetDeclInit(tmp, init);
}

PRIVATE Node *MakeArrayIndex(Node *array, Node *dim)
{
	assert(array && dim);

	{
		Node *arrayIndex = ExtendArray(array, dim, UnknownCoord);

		return arrayIndex;
	}
}

GLOBAL Node *MakeDuplicateWork(List *outputs, Node *input, Node *argument)
{
	Node *work = NULL, *output = NULL;
	Node *id = MakeIdCoord("i", UnknownCoord);
	Node *decl = MakeMyDecl("i", PrimSint, MakeConstSint(0), Redecl);
	Node *init =NULL, *cond = NULL, *next = NULL, *stmt = NULL, *forNode = NULL;
	List *decls = NULL, *stmts = NULL;
	ListMarker marker;

	//assert(outputs && input && input->typ == Id && Level == 2);  zww:20120319ע��

	REFERENCE(decl);//Ϊ�˲��� unused variable ����
	id->u.id.decl = decl;

	IterateList(&marker, outputs);
	while (NextOnList(&marker, (GenericREF)&output))
	{
		init = MakeBinopCoord('=', id, MakeConstSint(0), UnknownCoord);
		cond = MakeBinopCoord('<', id, argument, UnknownCoord);
		next = MakeUnaryCoord(PREINC, id, UnknownCoord);
		stmt = MakeBinopCoord('=', ExtendArray(output, id, UnknownCoord), ExtendArray(input, id, UnknownCoord), UnknownCoord);
		forNode = MakeForCoord(init, cond, next, stmt, UnknownCoord);

		stmts = AppendItem(stmts, forNode);
	}
	decls = MakeNewList(decl);
	work = MakeBlockCoord(PrimVoid, decls, stmts, UnknownCoord, UnknownCoord);

	return work;
}

GLOBAL Node *MakeRoundrobinWork(List *outputs, Node *input, List *arguments)
{
	Node *work = NULL, *forNode = NULL;
	Node *idI = MakeIdCoord("i", UnknownCoord), *idJ = MakeIdCoord("j", UnknownCoord);
	Node *declI = MakeMyDecl("i", PrimSint, MakeConstSint(0), Redecl), *declJ = MakeMyDecl("j", PrimSint, MakeConstSint(0), Redecl);
	Node *init = NULL, *cond = NULL, *next = NULL, *nextJ = NULL, *stmt = NULL;
	Node *output = NULL, *argument = NULL;
	List *decls = NULL, *stmts = NULL;
	ListMarker marker, marker2;

	assert(outputs && input && input->typ == Id  && Level == 2);
	//assert(ListLength(arguments) == ListLength(gCurrentCompositeCallList));   zww:20120319ע��

	idI->u.id.decl = declI; 
	idJ->u.id.decl = declJ; 
	REFERENCE(declI); REFERENCE(declJ); // Ϊ�˲��� unused variable ����
	decls = MakeNewList(declI);
	decls = AppendItem(decls, declJ);

	IterateList(&marker, arguments);
	IterateList(&marker2, outputs);
	while (NextOnList(&marker, (GenericREF)&argument))
	{
		NextOnList(&marker2, (GenericREF )&output);
		init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
		cond = MakeBinopCoord('<', idI, argument, UnknownCoord);
		next = MakeUnaryCoord(PREINC, idI, UnknownCoord);
		nextJ = MakeUnaryCoord(POSTINC, idJ, UnknownCoord);
		stmt = MakeBinopCoord('=', ExtendArray(output, idI, UnknownCoord), ExtendArray(input, nextJ, UnknownCoord), UnknownCoord);
		forNode = MakeForCoord(init, cond, next, stmt, UnknownCoord);

		stmts = AppendItem(stmts, forNode);
	}
	work = MakeBlockCoord(PrimVoid, decls, stmts, UnknownCoord, UnknownCoord);

	return work;
}

GLOBAL Node *MakeJoinWork(Node *output, List *inputs, List *arguments)
{
	Node *work = NULL, *forNode = NULL;
	Node *idI = MakeIdCoord("i", UnknownCoord), *idJ = MakeIdCoord("j", UnknownCoord);
	Node *declI = MakeMyDecl("i", PrimSint, MakeConstSint(0), Redecl), *declJ = MakeMyDecl("j", PrimSint, MakeConstSint(0), Redecl);
	Node *init = NULL, *cond = NULL, *next = NULL, *nextJ = NULL, *stmt = NULL;
	Node *input = NULL, *argument = NULL;
	List *decls = NULL, *stmts = NULL;
	ListMarker marker, marker2;

	assert(inputs && output && output->typ == Id  && Level == 2);
	//assert(ListLength(arguments) == ListLength(gCurrentCompositeCallList)); zww:20120319ע��

	REFERENCE(declI); REFERENCE(declJ);// Ϊ�˲��� unused variable ����
	idI->u.id.decl = declI;
	idJ->u.id.decl = declJ;
	decls = MakeNewList(declI);
	decls = AppendItem(decls, declJ);

	IterateList(&marker, arguments);
	IterateList(&marker2, inputs);
	while (NextOnList(&marker, (GenericREF)&argument))
	{
		NextOnList(&marker2, (GenericREF )&input);
		init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
		cond = MakeBinopCoord('<', idI, argument, UnknownCoord);
		next = MakeUnaryCoord(PREINC, idI, UnknownCoord);
		nextJ = MakeUnaryCoord(POSTINC, idJ, UnknownCoord);
		stmt = MakeBinopCoord('=', ExtendArray(output, nextJ, UnknownCoord), ExtendArray(input, idI, UnknownCoord), UnknownCoord);
		forNode = MakeForCoord(init, cond, next, stmt, UnknownCoord);

		stmts = AppendItem(stmts, forNode);
	}

	work = MakeBlockCoord(PrimVoid, decls, stmts, UnknownCoord, UnknownCoord);

	return work;
}

GLOBAL Node *MakeMyWindow(Node *id, Node *count, int style)
{
	Node *window = NULL;
	Node *sliding = NULL, *tumbling = NULL;
	Node *trigger = NULL;
	Node *sliding_value = NULL,*tumbling_value = NULL;
	assert(id && count && (style == 0 || style == 1));

	if (style == 0) // sliding window
	{
		Node *eviction = NULL;

		sliding_value = MakeWindowSlidingValueCoord(count,UnknownCoord);
		sliding = MakeWindowSlidingCoord(EMPTY_TQ, sliding_value, UnknownCoord);

		window = MakeWindowCoord(LookupStreamIdsNode(id), sliding, UnknownCoord); 
	}
	else // tumbling window
	{
		tumbling = MakeWindowTumbingCoord(EMPTY_TQ, count, UnknownCoord);
		window = MakeWindowCoord(LookupStreamIdsNode(id), tumbling, UnknownCoord); 
	}

	return window;
}

PRIVATE Node *ExtractStreamTypeOfCompositeInOut(Node *composite, int style)
{
	Node *streamType = NULL, *tmpNode = NULL;
	List *tmpList = NULL;

	// 0��ȡ����stream��StreamType, 1��ȡ���stream��StreamType
	assert(composite && composite->typ == Composite && (style == 0 || style == 1));
	if (style == 0)
	{
		tmpList = composite->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.inputs;
		assert(ListLength(tmpList) == 1);
		streamType = (Node *)FirstItem(tmpList); // ע�⣺������ʵ�Ǹ�Decl�ڵ�
	}
	else
	{
		tmpList = composite->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs;
		assert(ListLength(tmpList) == 1);
		tmpNode = (Node *)FirstItem(tmpList);
		streamType = tmpNode->u.decl.type;
	}

	return streamType;
	
}

// �޸�operdcl�����������
PRIVATE Node *ModifyOperdclInOut(Node *compositeCall, Node *output, Node *input)
{
	Node *operdcl = NULL;
	assert(compositeCall);
	assert(output && (output->typ == Id || output->typ == Decl) && input && input->typ == Id);
	if(compositeCall->typ == CompositeCall){
		operdcl = compositeCall->u.comCall.operdcl;
		operdcl->u.operdcl.inputs = MakeNewList(input);
		operdcl->u.operdcl.outputs = MakeNewList(output);
	}
	else if(compositeCall->typ == Pipeline){
		compositeCall->u.pipeline.input = input;
		compositeCall->u.pipeline.output = output;
	}
	else if(compositeCall->typ == SplitJoin){
		compositeCall->u.splitJoin.input = input;
		compositeCall->u.splitJoin.output = output;
	}
	else
		assert(0);
	return compositeCall;
}

GLOBAL Node *MakeSplitOperator(Node *input, List *arguments, int style)
{
	const char *operName[] = {"Duplicate", "Roundrobin"};
	const char *streamName[] = {"Dup", "Rou"};
	Node *splitOperator = NULL;
	Node *operHead = NULL, *operBody = NULL;
	Node *output = NULL, *arg = NULL, *work = NULL, *outputId = NULL;
	List *outputs = NULL, *inputs = NULL, *windows = NULL, *outputIds = NULL, *tmpList = NULL;
	ListMarker marker;
	Node *item = NULL;
	int len = ListLength(gCurrentCompositeCallList), sum = 0, count = 0;
	static int number = 0;

	// style == 0, Duplicate��ʽ�� style == 1, Roundrobin��ʽ
	assert(input && arguments && Level == 1 && (style == 0 || style == 1));
	assert(input->typ == Id && input->u.id.decl);
	assert(ListLength(arguments) == 1 || len == ListLength(arguments));// ����ƥ��

	inputs = MakeNewList(input);
	arg = (Node *)FirstItem(arguments);//ȡDuplicate����Ĳ���
	assert(NodeConstantIntegralValue(arg) >= 0);

	IterateList(&marker, arguments);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		tmpList = AppendItem(tmpList, MakeConstSint(NodeConstantIntegralValue(item)));
	}
	arguments = tmpList;

	if (ListLength(arguments) == 1) // roundrobin��ʽ��12.14�����޸�ΪDuplicate��ʽҲ���ã�
		for (count = 0; count < (len-1); ++count)
			arguments = AppendItem(arguments, MakeConstSint(NodeConstantIntegralValue(arg)));

	count = 0;	
	IterateList(&marker, arguments);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		char *tmp = (char *)malloc(strlen(streamName[style]) + 20);//һ��Ҫ�ڶ��������ڴ棬�������˳���ͻ��ͷ��ڴ��
		sum += NodeConstantIntegralValue(item);
		assert(NodeConstantIntegralValue(item) >= 0);

		sprintf(tmp, "%s%d_%d", streamName[style], number, count++);
		output = MakeNewStream(tmp, input->u.id.decl->u.decl.type);
		outputId = MakeNewStreamId(tmp, output);
		windows = AppendItem(windows, MakeMyWindow(outputId, item, 1));
		outputs = AppendItem(outputs, output);
		outputIds = AppendItem(outputIds, outputId);
	}

	operHead = MakeOperatorHead(operName[style], outputs, inputs);
	splitOperator = DefineOperator(operHead);

	if(style == 0) sum = NodeConstantIntegralValue(arg);
	windows = AppendItem(windows, MakeMyWindow(input, MakeConstSint(sum), 0));

	if (style == 0) // duplicate��ʽ
	{
		work = MakeDuplicateWork(outputIds, input, arg);
		splitOperator->u.operator_.ot = Duplicate_;
	}
	else // roundrobin��ʽ
	{
		work = MakeRoundrobinWork(outputIds, input, arguments);
		splitOperator->u.operator_.ot = Roundrobin_;
	}

	operBody = MakeOperatorBody(work, windows);
	splitOperator = SetOperatorBody(splitOperator, operBody);

	++number;
#if 0
	PrintNode(stdout, splitOperator, 0);
#endif
	return splitOperator;
}

GLOBAL Node *MakeJoinOperator(Node *output, List *inputs, List *arguments)
{
	const char *operName = "Join";
	Node *joinOperator = NULL;
	Node *operHead = NULL, *operBody = NULL;
	Node *input = NULL, *arg = NULL, *work = NULL, *item = NULL;
	List *outputs = NULL, *windows = NULL, *tmpList = NULL;
	ListMarker marker, marker2;
	int len = ListLength(gCurrentCompositeCallList), sum = 0, count = 0;

	assert(output && inputs && arguments && Level == 1);
	assert(output->typ == Decl || (output->typ == Id && output->u.id.decl));
	assert(ListLength(inputs) == len);
	assert(ListLength(arguments) == 1 || len == ListLength(arguments));// ����ƥ��

	if (output->typ == Decl)
	{
		output = MakeNewStreamId(output->u.decl.name, output);
	}
	outputs = MakeNewList(output);
	arg = (Node *)FirstItem(arguments);
	assert(NodeConstantIntegralValue(arg) >= 0);

	IterateList(&marker, arguments);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		tmpList = AppendItem(tmpList, MakeConstSint(NodeConstantIntegralValue(item)));
	}
	arguments = tmpList; // 2012.02.17

	if (ListLength(arguments) == 1) // roundrobin��ʽ
		for (count = 0; count < (len-1); ++count)
			arguments = AppendItem(arguments, MakeConstSint(NodeConstantIntegralValue(arg)));
	
	IterateList(&marker, arguments);
	IterateList(&marker2, inputs);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		NextOnList(&marker2, (GenericREF)&input);
		sum += NodeConstantIntegralValue(item);
		assert(NodeConstantIntegralValue(item) >= 0);
		windows = AppendItem(windows, MakeMyWindow(input, item, 0));
	}
	operHead = MakeOperatorHead(operName, outputs, inputs);
	joinOperator = DefineOperator(operHead);

	windows = AppendItem(windows, MakeMyWindow(output, MakeConstSint(sum), 1));

	work = MakeJoinWork(output, inputs, arguments);

	operBody = MakeOperatorBody(work, windows);
	joinOperator = SetOperatorBody(joinOperator, operBody);
	joinOperator->u.operator_.ot = Join_;

	return joinOperator;
}

PRIVATE List *MakeOperatorList(List *outputs, List *inputs, List *compositeCalls)
{
	
	ListMarker marker1, marker2, marker3;
	Node *output = NULL, *input = NULL, *compositeCall = NULL;
	int len1 = ListLength(outputs);
	int len2 = ListLength(inputs);
	int len3 = ListLength(compositeCalls);

	assert(outputs && inputs &&  len1 == len2 && len2 == len3);

	IterateList(&marker1, outputs);
	IterateList(&marker2, inputs);
	IterateList(&marker3, compositeCalls);
	while (NextOnList(&marker1, (GenericREF)(&output)))
	{
		NextOnList(&marker2, (GenericREF)(&input));
		NextOnList(&marker3, (GenericREF)(&compositeCall));
		assert(input->typ == Id);
		ModifyOperdclInOut(compositeCall, output, input);
	}
	
	return compositeCalls;
}

PRIVATE Node *UnfoldDuplicate(const char *comName, Node *node)
{
	const char *name = "DStream";
	char *newName = NULL;
	int i = 0, len = ListLength(gCurrentCompositeCallList);
	Node *duplicate = NULL, *comHead = NULL, *comBody = NULL;
	Node *splitOperator = NULL, *joinOperator = NULL;
	Node *graph = NULL;
	List *operators = NULL, *comCallList = NULL, *outputs = NULL, *inputs = NULL, *tmp = NULL, *tmpList = NULL;
	ListMarker marker, marker2;
	Node *item = NULL, *stream = NULL, *streamId = NULL;

	assert(ListLength(gCurrentSplitList) <= 1 && gIsDuplicate == TRUE && Level == 0);   //Duplicate����Ĳ�����Ҫ��
	comHead = MakeCompositeHead(comName, gCurrentOutputStreamNode, gCurrentInputStreamNode);
	duplicate = DefineComposite(comHead);

	if (gCurrentSplitList == NULL) //˵�� duplicate ����Ϊ��, ��Ĭ��ֵ 1
		gCurrentSplitList = MakeNewList(MakeConstSint(1));
	splitOperator = MakeSplitOperator(gCurrentInputStreamNode, gCurrentSplitList, 0);  //  splitOperator��һ��operato_���͵Ľڵ�

	tmpList = splitOperator->u.operator_.decl->u.decl.type->u.operdcl.outputs;
	IterateList(&marker2, tmpList);
	IterateList(&marker, gCurrentCompositeCallList);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		NextOnList(&marker2, (GenericREF)&stream);
		streamId = MakeNewStreamId(stream->u.decl.name, stream);
		inputs = AppendItem(inputs, NodeCopy(streamId, Subtree));
		newName = (char *)malloc(strlen(name) + 20);
		sprintf(newName, "%s%d_%d", name, gCurrentCompositeNum, i);
		stream = ExtractStreamTypeOfCompositeInOut(duplicate, 1);
		stream = MakeNewStream(newName, stream);
		outputs = AppendItem(outputs, stream);
		++i;
	}
	comCallList = MakeOperatorList(outputs, inputs, gCurrentCompositeCallList);

	inputs = NULL;
	if (gCurrentJoinList == NULL) //˵�� roundrobin ����Ϊ��, ��Ĭ��ֵ 1
		gCurrentJoinList = MakeNewList(MakeConstSint(1));
	IterateList(&marker, outputs);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		streamId = MakeNewStreamId(item->u.decl.name, item);
		inputs = AppendItem(inputs, streamId);
	}
	joinOperator = MakeJoinOperator(gCurrentOutputStreamNode, inputs, gCurrentJoinList);

	operators = MakeNewList(splitOperator);
	IterateList(&marker, comCallList);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		operators = AppendItem(operators, item);
	}
	operators = AppendItem(operators, joinOperator);
	comBody = MakeCompositeBody(operators);

	duplicate = SetCompositeBody(duplicate, comBody);
	node->u.splitJoin.splitOperator = splitOperator;
	node->u.splitJoin.joinOperator = joinOperator;
	gIsDuplicate = FALSE;
	
	return duplicate;
}

PRIVATE Node *UnfoldRoundrobin(const char *comName, Node *node)
{
	const char *name = "RStream";
	char *newName = NULL;
	int i = 0, len = ListLength(gCurrentCompositeCallList);
	Node *roundrobin = NULL, *comHead = NULL, *comBody = NULL;
	Node *splitOperator = NULL, *joinOperator = NULL;
	Node *graph = NULL;
	List *operators = NULL, *comCallList = NULL, *outputs = NULL, *inputs = NULL, *tmpList = NULL;
	ListMarker marker, marker2;
	Node *item = NULL, *stream = NULL, *streamId = NULL;

	assert(gIsRoundrobin == TRUE && Level == 0);
	comHead = MakeCompositeHead(comName, gCurrentOutputStreamNode, gCurrentInputStreamNode);
	roundrobin = DefineComposite(comHead);

	if (gCurrentSplitList == NULL) //˵�� roundrobin ����Ϊ��, ��Ĭ��ֵ 1
		gCurrentSplitList = MakeNewList(MakeConstSint(1));
	splitOperator = MakeSplitOperator(gCurrentInputStreamNode, gCurrentSplitList, 1);
	tmpList = splitOperator->u.operator_.decl->u.decl.type->u.operdcl.outputs;

	IterateList(&marker2, tmpList);
	IterateList(&marker, gCurrentCompositeCallList);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		NextOnList(&marker2, (GenericREF)&stream);
		streamId = MakeNewStreamId(stream->u.decl.name, stream);
		inputs = AppendItem(inputs, NodeCopy(streamId, Subtree));
		newName = (char *)malloc(strlen(name) + 20);
		sprintf(newName, "%s%d_%d", name, gCurrentCompositeNum, i);
		stream = ExtractStreamTypeOfCompositeInOut(roundrobin, 1);
		stream = MakeNewStream(newName, stream);
		outputs = AppendItem(outputs, stream);
		++i;
	}
	comCallList = MakeOperatorList(outputs, inputs, gCurrentCompositeCallList);
	
	inputs = NULL;
	if (gCurrentJoinList == NULL) //˵�� roundrobin ����Ϊ��, ��Ĭ��ֵ 1
		gCurrentJoinList = MakeNewList(MakeConstSint(1));
	IterateList(&marker, outputs);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		streamId = MakeNewStreamId(item->u.decl.name, item);
		inputs = AppendItem(inputs, streamId);
	}
	joinOperator = MakeJoinOperator(gCurrentOutputStreamNode, inputs, gCurrentJoinList);

	operators = MakeNewList(splitOperator);
	IterateList(&marker, comCallList);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		operators = AppendItem(operators, item);
	}
	operators = AppendItem(operators, joinOperator);
	comBody = MakeCompositeBody(operators);

	roundrobin = SetCompositeBody(roundrobin, comBody);
	node->u.splitJoin.splitOperator = splitOperator;
	node->u.splitJoin.joinOperator = joinOperator;
	gIsRoundrobin = FALSE;

	return roundrobin;
}


GLOBAL Node *UnfoldPipeline(Node *node)
{
	
	const char *name = "PStream";
	char *newName = NULL;
	int i = 0, len = ListLength(gCurrentCompositeCallList);
	Node *pipeline = NULL, *comHead = NULL, *comBody = NULL;
	Node *graph = NULL;
	List *operators = NULL, *outputs = NULL, *inputs = NULL;
	ListMarker marker;
	Node *item = NULL, *stream = NULL, *streamId = NULL;

	assert(gIsInPipeline == TRUE && Level == 0);
	gIsUnfold = TRUE;
	comHead = MakeCompositeHead(MakeCompositeName(gPipelineName), gCurrentOutputStreamNode, gCurrentInputStreamNode);
	pipeline = DefineComposite(comHead);
	// make inputs and outputs
	stream = ExtractStreamTypeOfCompositeInOut(pipeline, 0);
	streamId = MakeNewStreamId(stream->u.decl.name, stream);
	inputs = AppendItem(inputs, streamId);
	IterateList(&marker, gCurrentCompositeCallList);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		
		if(len == 1) break; // ��pipelineֻ��һ����Աʱ������Ҫ����inputs��outputs lxx.2012.02.22
		newName = (char *)malloc(strlen(name) + 20);
		sprintf(newName, "%s%d_%d", name, gCurrentCompositeNum, i);
		stream = FirstItem(item->u.comCall.call->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs);
		if(stream->typ==Decl)
			stream = stream->u.decl.type;
		stream = MakeNewStream(newName, stream);
		outputs = AppendItem(outputs, stream);
		streamId = MakeNewStreamId(stream->u.decl.name, stream);
		inputs = AppendItem(inputs, streamId);
		++i;
		if(i == (len-1)) break; // ����Ҫ��ȡ���һ��compositeCall���������
	}
	
	stream = gCurrentOutputStreamNode;
	if(stream->typ == Id)
		stream = stream->u.id.decl;
	streamId = MakeNewStreamId(stream->u.decl.name, stream);
	outputs = AppendItem(outputs, streamId);
	// make operators
	operators = MakeOperatorList(outputs, inputs, gCurrentCompositeCallList);
	comBody = MakeCompositeBody(operators);

	pipeline = SetCompositeBody(pipeline, comBody);
	
	IncCurrentCompositeNum();
	gCurrentCompositeCallList = NULL;
	gIsUnfold = FALSE;
	ResetUnfoldFlags(); // lxx.2012.02.22

#if 0
	PrintNode(stdout, pipeline, 0);
#endif

	return pipeline;
}


GLOBAL Node *UnfoldSplitJoin(Node *node) 
{
	Node *tmp = NULL;
	const char *comName = MakeCompositeName(gSplitJoinName);

	assert(gIsUnfold == FALSE);

	gIsUnfold = TRUE;
	if (gIsDuplicate)
		tmp = UnfoldDuplicate(comName, node);
	else
		tmp = UnfoldRoundrobin(comName, node);

	IncCurrentCompositeNum();
	gCurrentCompositeCallList = NULL;
	gIsUnfold = FALSE;
	ResetUnfoldFlags(); // lxx.2012.02.22

#if 0
	PrintNode(stdout, tmp, 0);
#endif

	return tmp;
}

PRIVATE void *FindStreamReplace(List *list, Node **firstComCall, Node **lastComCall, int firstTag){
	ListMarker marker;
	Node *item;
	List *tempList = NULL;
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item)){
		switch(item->typ){
		case Add:
			if(firstTag==0){
				*firstComCall = item->u.add.content;
				*lastComCall = item->u.add.content;
				firstTag = 1;
			}
			else
				*lastComCall = item->u.add.content;
			break;
		case If:
			FindStreamReplace(AppendItem(tempList,item->u.If.stmt), firstComCall, lastComCall, firstTag);
			break;
		case For:
			FindStreamReplace(AppendItem(tempList,item->u.For.stmt), firstComCall, lastComCall, firstTag);
			break;
		case Block:
			FindStreamReplace(item->u.Block.stmts, firstComCall, lastComCall, firstTag);
			break;
		default:
			break;

		}
	}
}
//�ڶ���SP�ṹ�н�����SP�����CompositeCall�ڵ�
GLOBAL Node *CreateCompositeInMultiSP(Node *node){
	Node *stream,*streamId,*InputStream,*OutputStream,*input,*output,*newNode;
	Node *comInOut = NULL,*comdcl = NULL, *comHead = NULL,*composite = NULL,*comBody = NULL,*operdcl = NULL,*comCall = NULL,*param = NULL;
	Node *firstComCall = NULL,*lastComCall = NULL;
	const char *InputName = "In",*OutputName = "Out",*name = "SPComposite";
	char *newName = NULL;
	List *operators = NULL,*inputs = NULL,*outputs = NULL;
	int firstTag = 0;
	assert(node->typ==SplitJoin||node->typ==Pipeline);
	//�ҵ�����first��last��CompositeCall�ڵ㣬������Ϊinput��output����streamtype����
	if(node->typ==SplitJoin)
		FindStreamReplace(node->u.splitJoin.stmts,&firstComCall,&lastComCall,firstTag);
	else if(node->typ==Pipeline)
		FindStreamReplace(node->u.pipeline.stmts,&firstComCall,&lastComCall,firstTag);
	gIsUnfold = TRUE;
	newNode = NodeCopy(node,Subtree);
	stream = FirstItem(firstComCall->u.comCall.call->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.inputs);
	if(stream->typ==Decl)
		stream = stream->u.decl.type;
	InputStream = MakeNewStream(InputName, stream);
	inputs = AppendItem(inputs,InputStream);
	stream = FirstItem(firstComCall->u.comCall.call->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs);
	if(stream->typ==Decl)
		stream = stream->u.decl.type;
	OutputStream = MakeNewStream(OutputName, stream);
	outputs = AppendItem(outputs,OutputStream);
	newName = (char *)malloc(strlen(name) + 20);
	sprintf(newName, "%s_%d", name, gMultiSPCompositeNum++);
	comInOut = MakeComInOutCoord(EMPTY_TQ,inputs,outputs,UnknownCoord);
	comdcl = MakeComdclCoord(EMPTY_TQ,comInOut,UnknownCoord);
	comHead = MakeDeclCoord(newName,T_TOP_DECL,comdcl,NULL,NULL,UnknownCoord);
	composite = DefineComposite(comHead);
	input = MakeNewStreamId(InputStream->u.decl.name, InputStream);
	output = MakeNewStreamId(OutputStream->u.decl.name, OutputStream);

	if(newNode->typ==Pipeline){
		newNode->u.pipeline.input = input;
		newNode->u.pipeline.output = output;
	}
	else{
		newNode->u.splitJoin.input = input;
		newNode->u.splitJoin.output = output;
	}
	param = MakeParamCoord(gCurrentDeclList[gMultiSPFlag],UnknownCoord);
	operators = AppendItem(operators,newNode);
	comBody = MakeComBodyCoord(PrimVoid,param,NULL,operators,UnknownCoord,UnknownCoord);
	composite = SetCompositeBody(composite, comBody);
	operdcl = MakeOperdclCoord(EMPTY_TQ,NULL,NULL,gCurrentParamList[gMultiSPFlag],UnknownCoord);
	comCall = MakeCompositeCallCoord(composite,operdcl,TRUE,UnknownCoord);
	gIsUnfold = FALSE;
	return comCall;
}