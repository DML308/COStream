#ifndef _DuplicatFission_H_
#define _DuplicatFission_H_

#include "schedulerSSG.h"
#include "ActorStateDetector.h"
#include <fstream>
#include <regex>
#include <vector>
using namespace std;
extern "C"
{
	extern GLOBAL int CpuCoreNum;
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
	extern GLOBAL Bool DynamicX86Backend;
	extern GLOBAL double BalanceFactor;
	extern GLOBAL const char *input_file;
	extern GLOBAL const char *output_file;
	extern GLOBAL Bool GenerateOutput;
	extern GLOBAL const char *with_suffix(const char *filename,
		const char *old_suffix,
		const char *new_suffix);
	extern GLOBAL FILE *yyin;
	extern GLOBAL FILE *get_preprocessed_input();
	extern GLOBAL void init_symbol_tables(Bool shadow_warnings);
	extern Bool parsed_ok;
	extern GLOBAL Bool PrintAST;
	extern GLOBAL char tmpname[L_tmpnam];
	extern GLOBAL Bool SemanticCheck;
	extern GLOBAL Bool Analyze;
	extern GLOBAL Bool VariableRename;
	extern GLOBAL Bool Propagate;
	extern GLOBAL Bool AST2FlatSSG;
	extern GLOBAL Bool HeterogeneousClusterBackend;
	extern GLOBAL Bool DuplicatFission;
	extern int numOfMain;
	extern GLOBAL Bool DivideSSG;
	//extern GLOBAL void dynamicX86Backend(string str);
	extern GLOBAL Bool WorkEstimate;
	extern GLOBAL Bool WorkEstimateByDataFlow;
	//extern GLOBAL void hClusterBanckend(string str);
	extern GLOBAL Bool SchedulingFlatSSG;
	extern GLOBAL SchedulerSSG *SSSG;
};

SchedulerSSG *makeDuplicatFission(SchedulerSSG *sssg, int i, int k, string InputFileName);

SchedulerSSG *duplicatFission(SchedulerSSG *sssg, string InputFileName){//���Ʒ����㷨
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg���ж����vector
	int nvtxs = V.size();
	vector<int> vwgt(nvtxs, 0); //ÿ�������Ȩ��(�ܹ�����)
	map<FlatNode *, int> stadyWork = sssg->GetSteadyWorkMap();//��Ÿ���operator����̬����������
	map<FlatNode *, int> steadyCount = sssg->mapSteadyCount2FlatNode; // SDFͼ���нڵ��ȶ�״̬��������<�ڵ㣬ִ�д���>
	map<FlatNode *, int> initCount = sssg->mapInitCount2FlatNode;
	sssg->total_work = 0.0;//����SDF�ܵĽڵ㹤����
	for (int i = 0; i < nvtxs; i++){
		if (X86Backend || DynamicX86Backend)
		{
			vwgt[i] = stadyWork[V[i]] * steadyCount[V[i]];
		}
		else if (GPUBackend)
		{
			if (DetectiveActorState(sssg->GetFlatNodes()[i]))
			{
				vwgt[i] = 0;
			}
			else
			{
				vwgt[i] = stadyWork[V[i]] * steadyCount[V[i]];
			}
		}
		//cout << i << "  " << V[i]->name << "  " << vwgt[i] << endl;
		sssg->total_work += vwgt[i];
	}
	int partnum = CpuCoreNum;
	//if (CpuCoreNum > nvtxs)partnum = nvtxs;
	double we = sssg->total_work / partnum;//��ͼ��ƽ��������
	int vwmax = 0, choose = 0;
	for (int i = 1; i < vwgt.size() - 1; i++){//�׽ڵ��β�ڵ��޷�����
		if (vwgt[i] > vwmax){
			vwmax = vwgt[i];
			choose = i;
		}
	}

	SchedulerSSG *res = sssg;
	if (vwmax > we * BalanceFactor){
		int k = vwmax / we + 1;//�����Ϸ���k��
		int sum = initCount[V[choose]] + steadyCount[V[choose]];
		//�ܵ����������� = sum * pop,ÿ�θ�һ���ڵ�pop����Ҫ��sum��
		if (k >= sum)k = sum;
		else {//��sum = 9,k = 3,��k = 3����sum = 8��k = 3,��k��Ϊ4 
			int t = sum / k;
			k = sum / t;
		}
		if (k > 1)res = makeDuplicatFission(sssg, choose, k, InputFileName);
		cout << endl;
	}
	return res;
}

SchedulerSSG *makeDuplicatFission(SchedulerSSG *sssg, int i, int k, string InputFileName){//����i���ڵ㸴�Ʒ���Ϊk��

	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg���ж����vector
	if (V[i]->nIn != 1 && V[i]->nOut != 1)return sssg;//Ŀǰֻʵ�ֵ�����͵�����ڵ�ķ��Ѳ���
	string operatorName = V[i]->name;
	operatorNode *q = V[i]->contents;
	//cout << "�ڵ�body���ͣ�" << q->body->typ << endl;
	cout << i << "�Žڵ�" << operatorName << "��������Ϊ" << k << "��" << endl;

	string inStreamName, outStreamName;
	//	cout << "AAAAAA" << q->decl->u.decl.type->typ << endl;
	Node *t1 = (Node *)FirstItem(q->decl->u.decl.type->u.operdcl.outputs);
	outStreamName = t1->u.id.text;
	cout << "�����:" << outStreamName << endl;

	Node *t2 = (Node *)FirstItem(q->decl->u.decl.type->u.operdcl.inputs);
	inStreamName = t2->u.id.text;
	cout << "�����:" << inStreamName << endl;

	ChildList *windowList = q->body->u.operBody.window;
	int length = ListLength(windowList);
	//cout << "���������ȣ�" << length << endl;
	int peek, pop, push;
	ListMarker mm;
	Node *nn;
	IterateList(&mm, windowList);
	while (NextOnList(&mm, (GenericREF)&nn)){
		string windowName = nn->u.window.id->u.id.text;
		Node *slidingOrumbling = nn->u.window.wtype;
		if (windowName == outStreamName){
			if (slidingOrumbling->typ == Sliding){
				cout << "����ʧ��:���ѽڵ��������ڱ���Ϊtumbling!" << endl;
				return sssg;//���peek != pop����������������޷�����
			}
			else if (slidingOrumbling->typ == Tumbling){
				push = slidingOrumbling->u.tumbling.tumbling_value->u.Const.value.i;
				cout << "pushֵ:" << push << endl;
			}
		}
		else if (windowName == inStreamName){
			if (slidingOrumbling->typ == Sliding){
				Node *t = (Node *)FirstItem(slidingOrumbling->u.sliding.sliding_value->u.comma.exprs);
				peek = t->u.Const.value.i;
				cout << "peekֵ��" << peek << endl;

				t = (Node *)LastItem(slidingOrumbling->u.sliding.sliding_value->u.comma.exprs);
				pop = t->u.Const.value.i;
				cout << "popֵ��" << pop << endl;
				if (peek != pop){
					cout << "����ʧ��:���ѽڵ��sliding���ڵ�peek��pop������ȣ�" << endl;
					return sssg;//���peek != pop����������������޷�����
				}
			}
			else if (slidingOrumbling->typ == Tumbling){
				peek = slidingOrumbling->u.tumbling.tumbling_value->u.Const.value.i;
				pop = peek;
				cout << "peekֵ:" << peek << endl;
				cout << "popֵ:" << pop << endl;
			}
		}
	}

	compositeNode *p = V[i]->composite;
	string compositeName = p->decl->u.decl.name;
	cout << "compositeName���֣�" << compositeName << endl;
	ChildList *ParameList = p->body->u.comBody.param->u.param.parameters;
	length = ListLength(ParameList);
	cout << "��" << length << "������" << endl;
	ListMarker m;
	Node *n;
	vector <string> Type;//<��������>
	string paramType;
	IterateList(&m, ParameList);
	while (NextOnList(&m, (GenericREF)&n)){
		cout << n->u.decl.name << " ";
		Node *c = n->u.decl.type;
		switch (c->u.prim.basic) {
		case Sint:
		case Uint:
			paramType = "int";
			cout << paramType << endl;
			break;
		case Slong:
		case Ulong:
			paramType = "long";
			cout << paramType << endl;
			break;
		case Ulonglong:
		case Slonglong:
			paramType = "long long";
			cout << paramType << endl;
			break;
		case Float:
			paramType = "float";
			cout << paramType << endl;
			break;
		case Double:
		case Longdouble:
			paramType = "double";
			cout << paramType << endl;
			break;
		case Ushort:
		case Sshort:
			paramType = "short";
			cout << paramType << endl;
			break;
		case Char:
		case Schar:
		case Uchar:
			paramType = "char";
			cout << paramType << endl;
			break;

		default:
			Fail(__FILE__, __LINE__, "");
			return 0;
		}
		Type.push_back(paramType);
	}


#ifdef WIN32
	int pos1 = InputFileName.find_last_of('/');
	int pos2 = InputFileName.find(".cos");
	/*��pos=-1˵������Դ���򲻺Ϸ�*/
	int len = InputFileName.length();
	assert(pos1 != -1 && pos2 != -1 && pos2 == (len - 4));
	string substring = InputFileName.substr(pos1 + 1, pos2 - pos1 - 1);
	string OutputFileName = InputFileName.substr(0, pos2) + "Fission.cos";
#else
	//Linuxƽ̨�� Դ�����ļ��� ���Ƽ��� 2015/12/11 
	int pos = InputFileName.find(".cos");
	int len = InputFileName.length();
	if ((len - 4) != pos);
	{
		printf("error: in the input file name!\n");
		exit(1);
	}//if 
	string substring = InputFileName.substr(0, pos);
	string ccfilename = InputFileName.substr(0, pos);
	string OutputFileName = substring + "Fission.cos";
#endif

	ifstream in(InputFileName);
	ofstream out(OutputFileName);

	string s;
	string splitCompositeName = operatorName + "_Fission";
	ostringstream s1;
	s1 << "(" << k;
	string splitCompositeParam = s1.str();

	string add_pattern = "(add[ |\t]+)(" + compositeName + ")" + "(\\()" + "(.*)" + "(\\))";//ƥ��add composite���ô�,����ж�����ô��������������ȷ��
	string fmt = "add " + splitCompositeName + splitCompositeParam;
	regex r(add_pattern);
	smatch results;

	string compositeCall_pattern = "(" + outStreamName + ")" + "([ |\t]+=[ |\t]+)" + "(" + compositeName + ")";//ƥ��composite���ô�,����ж�����ô��������������ȷ��
	compositeCall_pattern = compositeCall_pattern + "(\\(" + inStreamName + "\\))" + "(\\()" + "(.*)" + "(\\))";
	string fmt1 = outStreamName + " = " + splitCompositeName + "(" + inStreamName + ")" + splitCompositeParam;
	regex r1(compositeCall_pattern);
	smatch results1;

	//���ѽڵ����
	ostringstream param1;
	ostringstream param2;
	ostringstream fission;
	param1.clear();
	param2.clear();
	fission.clear();
	if (Type.size() != 0){
		param1 << Type[0] + " " << compositeName + "_param1";
		param2 << compositeName + "_param1";
	}
	for (int i = 1; i < Type.size(); i++){
		param1 << "," + Type[i] + " " << compositeName + "_param" << i + 1;
		param2 << "," + compositeName + "_param" << i + 1;
	}
	fission << "composite " << operatorName << "_Fission(output stream<int x>Out, input stream<int x>In)\n\
{\n\
param\n\
	int size," << param1.str() << ";\n\
		Out = splitjoin(In)\n\
		{\n\
			int i;\n\
			split roundrobin(" << pop << ");\n\
			for (i = 0; i < size; i++)\n\
			{\n\
				add " << compositeName << "(" << param2.str() << ")" << ";\n\
			}\n\
			join roundrobin(" << push << ");\n\
	};\n\
}\n";

	string Main_pattern("composite[ |\t]+Main");//ƥ��composite Mian���������ѽڵ��������composite Mian����
	regex r2(Main_pattern);
	smatch results2;

	while (getline(in, s)){
		if (regex_search(s, results1, r1)){
			cout << "����ƥ��ɹ���" << endl;
			cout << "����:" << results1[6].str() << endl;
			out << regex_replace(s, r1, fmt1 + "," + results1[6].str() + ")") << endl;
		}
		else if (regex_search(s, results, r)){
			cout << "add����ƥ��ɹ���" << endl;
			cout << "����:" << results[4].str() << endl;
			out << regex_replace(s, r, fmt + "," + results[4].str() + ")") << endl;
		}
		else if (regex_search(s, results2, r2)){
			cout << "Mainƥ��ɹ���" << endl;
			out << fission.str() << endl;
			out << s << endl;
		}
		else out << s << endl;
	}
	cout << endl << "���ѳɹ��� " << "���ļ�" << substring << "Fission.cos������" << endl;

	input_file = OutputFileName.c_str();
	//cout << input_file << endl;
	if (GenerateOutput == TRUE && input_file != NULL && output_file == NULL)
		output_file = with_suffix(input_file, INPUT_SUFFIX, OUTPUT_SUFFIX);
	InputFileName = input_file;

	yyin = get_preprocessed_input();
	//��1����ʼ�����������͡����ű���������
	InitTypes();
	init_symbol_tables(TRUE);
	InitOperatorTable();
	numOfMain = 0;//�׽ڵ��������Ϊ0
	//gDeclList = NULL;//��ȫ����������Ϊ��
	/*��2���ķ��������﷨�����ɣ�yyparse()�������ڲ������﷨������
	* ��parser.c 3371�� {Program = GrabPragmas((yyvsp[(1)-(1)].L))};
	* �õ�������Դ����ķ������-�﷨��������list *program��
	*/

	PhaseName = "Parsing";
	parsed_ok = yyparse();

	//if (Level != 0) {
	//	SyntaxError("unexpected end of file");
	//}

	////��3����ӡ���ű�
	//if (PrintSymTables) {
	//	PrintSymbolTable(stdout, Externals);
	//}

	// ��4��ɾ����ʱ�ļ� & �ʡ��﷨����������
	if (tmpname[0] != 0)
	{
		assert(0 == fclose(yyin));
		assert(0 == remove(tmpname));
	}

#ifndef NDEBUG
	if (Errors == 0) {
		PhaseName = "Verification";
		VerifyParse(Program);
	}
#endif

	// ��5�������� sem-check.c ����һ����ǰ���﷨�����������ļ�
	PhaseName = "Semantic Check";
	if (Errors == 0 && SemanticCheck)
		Program = SemanticCheckProgram(Program);

	// ��6����Ծ�������� analyze.c
	PhaseName = "Analyze";
	if (Errors == 0 && Analyze)
		AnalyzeProgram(Program);

	//��7����ӱ���������
	PhaseName = "VariableRename";
	if (Errors == 0 && VariableRename)
	{
		Program = VariableRenameProgram(Program);
	}
	ResetASTSymbolTable(VariableRenameTable);

	// ��8����������
	PhaseName = "Propagate";
	if (Errors == 0 && Propagate)
	{
		Program = PropagateProgram(Program);
		gIsAfterPropagate = TRUE;
	}

	//��9����ӡ�����﷨��
	FILE *fp = fopen("SyntaxTree.txt", "w");
	PrintList(fp, Program, -1);
	if (PrintAST)
	{
		PrintList(stdout, Program, -1);
		fprintf(stdout, "\n");
	}

	// ��10���﷨����ƽ��ͼ�� SSG �� StaticStreamGraph ����
	PhaseName = "AST2FlatSSG";
	if (Errors == 0 && AST2FlatSSG)
		SSG = AST2FlatStaticStreamGraph(gMainComposite);

	// ��11���ж��Ƿ��Ƕ�̬���������򣬲���uncertainty���ڣ�yangshengzhe

	PhaseName = "DSGSearch";
	if (Errors == 0 && hasUncertainty(SSG))
	{
		if (Errors == 0 && CheckSplitUncertainty(SSG))
		{
			/*����X86�Ķ�̬��������ˣ��޸ĺ�ˣ��ر�X86,������������������޳�ͻ*/
			DivideSSG = TRUE;
			DynamicX86Backend = TRUE;
			X86Backend = FALSE;

			dynamicX86Backend(substring);
		}
		else
		{
			SyntaxError("can not put uncertainty in roundrobin/duplicate");
		}
	}//if

	// ��12����ƽ��ͼ���ڵ���й��������ƺ�����GenerateWorkEst.cpp��ʵ��
	PhaseName = "WorkEstimate";
	if (Errors == 0 && WorkEstimate)
		GenerateWorkEst(SSG, WorkEstimateByDataFlow);
	//����ǰ��+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++end

	//�칹��Ⱥ���+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++bengin
	/*���Ȼ�������칹��Ⱥģ�� --- 20151206 yrr���*/
	if (0 == Errors && HeterogeneousClusterBackend)
	{
		/*substringΪԴ�����ļ�����ֱ�ӽ����칹��Ⱥ��ˣ�����������޳�ͻ*/
		hClusterBanckend(substring);
	}//if

	//����X86��X10��GPU���standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++begin

	// ��1����ƽ��ͼ���г�ʼ�����Ⱥ���̬���� SSG����ǰ�˲���(10)���﷨��ת��
	PhaseName = "schedulerSSG";
	if (Errors == 0 && SchedulingFlatSSG)
		SSSG = SchedulingSSG(SSG);

	// ��ƽ��ͼ���и��Ʒ��� 2017-0-30 wl���
	PhaseName = "duplicatFission";
	if (Errors == 0 && DuplicatFission)
		SSSG = duplicatFission(SSSG, InputFileName);
	return SSSG;
}

#endif