/***********************--------------Define For SPL----------****************************/
#pragma ident "file_rw.c,v 1.2 2012/02/29 09:34--Liu Xiaoxian, DML, HUST"

#include "ast.h"
#include "file_rw.h"
GLOBAL Bool gIsFileOperator = FALSE; // �����齨file�ڵ�״̬

PRIVATE Node *MakeModeDecl(int init)
{
	Node *id = NULL, *tmp = NULL;

	id = MakeIdCoord("mode", UnknownCoord);
	tmp = SetDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), PrimSint, Redecl);
	//REFERENCE(tmp);

	if(init)
		return SetDeclInit(tmp, MakeConstSint(1));
	else
		return SetDeclInit(tmp, MakeConstSint(0));
}

GLOBAL Node *MakeFileReaderOperator(Node *output, List *args, Coord coord)
{
	Node *path = NULL, *mode = NULL, *item = NULL, *reader = NULL;
	Node *operHead = NULL, *operBody = NULL, *logic = NULL;
	Node *tmp = NULL, *fileReaderOperator = NULL;
	List *state = NULL;
	ListMarker marker;
	int len = ListLength(args);
	int len1 = 0, len2 = 0, oldLevel = Level;
	char m_text[2][10] = {"\"bin\"", "\"txt\""};

	assert(len == 1 || len == 2); // �ļ����ʹ򿪷�ʽ��ȱʡΪ�ı���ʽ
	//assert(output && (output->typ == Id || output->typ == Decl));
	gIsFileOperator = TRUE;
	Level = 2;

	reader = MakeIdCoord("FileReader", coord);
	operHead = ModifyDeclType(ConvertIdToDecl(reader, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, MakeNewList(output), NULL, NULL, reader->coord));

	item = (Node *)FirstItem(args);
	assert(item->typ == Const && item->u.Const.type->u.adcl.type->u.prim.basic == Char);
	len1 = item->u.Const.type->u.adcl.dim->u.Const.value.i;
	infileName = item->u.Const.text;
	path = MakeIdCoord("path", item->coord);
	path = ConvertIdToDecl(path, EMPTY_TQ, NULL, NULL, NULL);
	tmp = MakeAdclCoord(EMPTY_TQ, NULL, MakeConstSint(len1), item->coord);
	ModifyDeclType(path, tmp);
	SetDeclType(path, MakePrimCoord(EMPTY_TQ, Char, item->coord), Redecl);
	SetDeclInit(path, item);

	if (len == 1) // Ĭ���ı���ʽ��ȡ�ļ�
	{
		mode = MakeModeDecl(1);
	}
	else
	{
		item = (Node *)LastItem(args);
		if (strcmp(item->u.Const.text, m_text[0]) == 0) // λ��ʽ��ȡ�ļ�
			mode = MakeModeDecl(0);
		else if (strcmp(item->u.Const.text, m_text[1] == 0))
			mode = MakeModeDecl(1);
		else
			SyntaxErrorCoord(item->coord, "illegal file access mode!");
	}
	state = AppendItem(state, path);
	state = AppendItem(state, mode);
//	logic = MakeLogicCoord(state, NULL, MakeBlockCoord(PrimVoid, NULL, NULL, UnknownCoord, UnknownCoord), UnknownCoord); // work�������գ������������ɺ���γ�
	operBody = MakeOperBodyCoord(PrimVoid, state, NULL,MakeBlockCoord(PrimVoid, NULL, NULL, UnknownCoord, UnknownCoord),NULL, UnknownCoord, UnknownCoord); // windowΪ�ձ�ʾȡĬ��ֵ

	Level = oldLevel;
	fileReaderOperator = DefineOperator(operHead);
	fileReaderOperator = SetOperatorBody(fileReaderOperator, operBody);

	gIsFileOperator = FALSE;
	return fileReaderOperator;
}

GLOBAL Node *MakeFileWriterOperator(Node *input, List *args, Coord coord)
{
	Node *path = NULL, *mode = NULL, *item = NULL, *writer = NULL;
	Node *operHead = NULL, *operBody = NULL, *logic = NULL;
	Node *tmp = NULL, *fileWriterOperator = NULL;
	List *state = NULL;
	ListMarker marker;
	int len = ListLength(args);
	int len1 = 0, len2 = 0, oldLevel = Level;
	char m_text[2][10] = {"\"bin\"", "\"txt\""};

	assert(len == 1 || len == 2); // �ļ����ʹ򿪷�ʽ��ȱʡΪ�ı���ʽ
	assert(input && input->typ == Id);
	gIsFileOperator = TRUE;
	Level = 2;

	LookupStreamIdsNode(input);//��id�ڵ㲹ȫ��Ϣ
	writer = MakeIdCoord("FileWriter", coord);
	operHead = ModifyDeclType(ConvertIdToDecl(writer, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, MakeNewList(input), NULL, writer->coord));

	item = (Node *)FirstItem(args);
	assert(item->typ == Const && item->u.Const.type->u.adcl.type->u.prim.basic == Char);
	len1 = item->u.Const.type->u.adcl.dim->u.Const.value.i;

	path = MakeIdCoord("path", item->coord);
	path = ConvertIdToDecl(path, EMPTY_TQ, NULL, NULL, NULL);
	tmp = MakeAdclCoord(EMPTY_TQ, NULL, MakeConstSint(len1), item->coord);
	ModifyDeclType(path, tmp);
	SetDeclType(path, MakePrimCoord(EMPTY_TQ, Char, item->coord), Redecl);
	SetDeclInit(path, item);

	if (len == 1) // Ĭ���ı���ʽ��ȡ�ļ�
	{
		mode = MakeModeDecl(1);
	}
	else
	{
		item = (Node *)LastItem(args);
		if (strcmp(item->u.Const.text, m_text[0]) == 0) // λ��ʽ��ȡ�ļ�
			mode = MakeModeDecl(0);
		else if (strcmp(item->u.Const.text, m_text[1] == 0))
			mode = MakeModeDecl(1);
		else
			SyntaxErrorCoord(item->coord, "illegal file access mode!");
	}
	state = AppendItem(state, path);
	state = AppendItem(state, mode);
//	logic = MakeLogicCoord(state, NULL, MakeBlockCoord(PrimVoid, NULL, NULL, UnknownCoord, UnknownCoord), UnknownCoord); // work�������գ������������ɺ���γ�
//	operBody = MakeOperBodyCoord(PrimVoid, logic, NULL, UnknownCoord, UnknownCoord); // windowΪ�ձ�ʾȡĬ��ֵ
	operBody = MakeOperBodyCoord(PrimVoid, state, NULL,MakeBlockCoord(PrimVoid, NULL, NULL, UnknownCoord, UnknownCoord), NULL,UnknownCoord, UnknownCoord);
	Level = oldLevel;
	fileWriterOperator = DefineOperator(operHead);
	fileWriterOperator = SetOperatorBody(fileWriterOperator, operBody);

	gIsFileOperator = FALSE;
	return fileWriterOperator;
}

