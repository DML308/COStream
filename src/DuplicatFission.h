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

SchedulerSSG *duplicatFission(SchedulerSSG *sssg, string InputFileName){//复制分裂算法
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg所有顶点的vector
	int nvtxs = V.size();
	vector<int> vwgt(nvtxs, 0); //每个顶点的权重(总工作量)
	map<FlatNode *, int> stadyWork = sssg->GetSteadyWorkMap();//存放各个operator的稳态工作量估计
	map<FlatNode *, int> steadyCount = sssg->mapSteadyCount2FlatNode; // SDF图所有节点稳定状态调度序列<节点，执行次数>
	map<FlatNode *, int> initCount = sssg->mapInitCount2FlatNode;
	sssg->total_work = 0.0;//计算SDF总的节点工作量
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
	double we = sssg->total_work / partnum;//子图的平均工作量
	int vwmax = 0, choose = 0;
	for (int i = 1; i < vwgt.size() - 1; i++){//首节点和尾节点无法分裂
		if (vwgt[i] > vwmax){
			vwmax = vwgt[i];
			choose = i;
		}
	}

	SchedulerSSG *res = sssg;
	if (vwmax > we * BalanceFactor){
		int k = vwmax / we + 1;//理论上分裂k份
		int sum = initCount[V[choose]] + steadyCount[V[choose]];
		//总的输入数据量 = sum * pop,每次给一个节点pop个，要给sum次
		if (k >= sum)k = sum;
		else {//若sum = 9,k = 3,则k = 3；若sum = 8，k = 3,则k变为4 
			int t = sum / k;
			k = sum / t;
		}
		if (k > 1)res = makeDuplicatFission(sssg, choose, k, InputFileName);
		cout << endl;
	}
	return res;
}

SchedulerSSG *makeDuplicatFission(SchedulerSSG *sssg, int i, int k, string InputFileName){//将第i个节点复制分裂为k份

	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg所有顶点的vector
	if (V[i]->nIn != 1 && V[i]->nOut != 1)return sssg;//目前只实现单输入和单输出节点的分裂操作
	string operatorName = V[i]->name;
	operatorNode *q = V[i]->contents;
	//cout << "节点body类型：" << q->body->typ << endl;
	cout << i << "号节点" << operatorName << "将被分裂为" << k << "份" << endl;

	string inStreamName, outStreamName;
	//	cout << "AAAAAA" << q->decl->u.decl.type->typ << endl;
	Node *t1 = (Node *)FirstItem(q->decl->u.decl.type->u.operdcl.outputs);
	outStreamName = t1->u.id.text;
	cout << "输出边:" << outStreamName << endl;

	Node *t2 = (Node *)FirstItem(q->decl->u.decl.type->u.operdcl.inputs);
	inStreamName = t2->u.id.text;
	cout << "输入边:" << inStreamName << endl;

	ChildList *windowList = q->body->u.operBody.window;
	int length = ListLength(windowList);
	//cout << "窗口链表长度：" << length << endl;
	int peek, pop, push;
	ListMarker mm;
	Node *nn;
	IterateList(&mm, windowList);
	while (NextOnList(&mm, (GenericREF)&nn)){
		string windowName = nn->u.window.id->u.id.text;
		Node *slidingOrumbling = nn->u.window.wtype;
		if (windowName == outStreamName){
			if (slidingOrumbling->typ == Sliding){
				cout << "分裂失败:分裂节点的输出窗口必须为tumbling!" << endl;
				return sssg;//如果peek != pop则存在数据依赖，无法分裂
			}
			else if (slidingOrumbling->typ == Tumbling){
				push = slidingOrumbling->u.tumbling.tumbling_value->u.Const.value.i;
				cout << "push值:" << push << endl;
			}
		}
		else if (windowName == inStreamName){
			if (slidingOrumbling->typ == Sliding){
				Node *t = (Node *)FirstItem(slidingOrumbling->u.sliding.sliding_value->u.comma.exprs);
				peek = t->u.Const.value.i;
				cout << "peek值：" << peek << endl;

				t = (Node *)LastItem(slidingOrumbling->u.sliding.sliding_value->u.comma.exprs);
				pop = t->u.Const.value.i;
				cout << "pop值：" << pop << endl;
				if (peek != pop){
					cout << "分裂失败:分裂节点的sliding窗口的peek和pop必须相等！" << endl;
					return sssg;//如果peek != pop则存在数据依赖，无法分裂
				}
			}
			else if (slidingOrumbling->typ == Tumbling){
				peek = slidingOrumbling->u.tumbling.tumbling_value->u.Const.value.i;
				pop = peek;
				cout << "peek值:" << peek << endl;
				cout << "pop值:" << pop << endl;
			}
		}
	}

	compositeNode *p = V[i]->composite;
	string compositeName = p->decl->u.decl.name;
	cout << "compositeName名字：" << compositeName << endl;
	ChildList *ParameList = p->body->u.comBody.param->u.param.parameters;
	length = ListLength(ParameList);
	cout << "有" << length << "个参数" << endl;
	ListMarker m;
	Node *n;
	vector <string> Type;//<参数类型>
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
	/*若pos=-1说明输入源程序不合法*/
	int len = InputFileName.length();
	assert(pos1 != -1 && pos2 != -1 && pos2 == (len - 4));
	string substring = InputFileName.substr(pos1 + 1, pos2 - pos1 - 1);
	string OutputFileName = InputFileName.substr(0, pos2) + "Fission.cos";
#else
	//Linux平台下 源程序文件名 控制检验 2015/12/11 
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

	string add_pattern = "(add[ |\t]+)(" + compositeName + ")" + "(\\()" + "(.*)" + "(\\))";//匹配add composite调用处,如果有多个调用处，则用输入边来确定
	string fmt = "add " + splitCompositeName + splitCompositeParam;
	regex r(add_pattern);
	smatch results;

	string compositeCall_pattern = "(" + outStreamName + ")" + "([ |\t]+=[ |\t]+)" + "(" + compositeName + ")";//匹配composite调用处,如果有多个调用处，则用输入边来确定
	compositeCall_pattern = compositeCall_pattern + "(\\(" + inStreamName + "\\))" + "(\\()" + "(.*)" + "(\\))";
	string fmt1 = outStreamName + " = " + splitCompositeName + "(" + inStreamName + ")" + splitCompositeParam;
	regex r1(compositeCall_pattern);
	smatch results1;

	//分裂节点代码
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

	string Main_pattern("composite[ |\t]+Main");//匹配composite Mian处，将分裂节点代码置于composite Mian上面
	regex r2(Main_pattern);
	smatch results2;

	while (getline(in, s)){
		if (regex_search(s, results1, r1)){
			cout << "调用匹配成功！" << endl;
			cout << "参数:" << results1[6].str() << endl;
			out << regex_replace(s, r1, fmt1 + "," + results1[6].str() + ")") << endl;
		}
		else if (regex_search(s, results, r)){
			cout << "add调用匹配成功！" << endl;
			cout << "参数:" << results[4].str() << endl;
			out << regex_replace(s, r, fmt + "," + results[4].str() + ")") << endl;
		}
		else if (regex_search(s, results2, r2)){
			cout << "Main匹配成功！" << endl;
			out << fission.str() << endl;
			out << s << endl;
		}
		else out << s << endl;
	}
	cout << endl << "分裂成功！ " << "新文件" << substring << "Fission.cos已生成" << endl;

	input_file = OutputFileName.c_str();
	//cout << input_file << endl;
	if (GenerateOutput == TRUE && input_file != NULL && output_file == NULL)
		output_file = with_suffix(input_file, INPUT_SUFFIX, OUTPUT_SUFFIX);
	InputFileName = input_file;

	yyin = get_preprocessed_input();
	//（1）初始化环境（类型、符号表、操作符表）
	InitTypes();
	init_symbol_tables(TRUE);
	InitOperatorTable();
	numOfMain = 0;//首节点计数重置为0
	//gDeclList = NULL;//将全局声明重置为空
	/*（2）文法建立和语法树生成，yyparse()函数是内部调用语法分析器
	* 于parser.c 3371行 {Program = GrabPragmas((yyvsp[(1)-(1)].L))};
	* 得到对输入源程序的分析结果-语法树，存在list *program中
	*/

	PhaseName = "Parsing";
	parsed_ok = yyparse();

	//if (Level != 0) {
	//	SyntaxError("unexpected end of file");
	//}

	////（3）打印符号表
	//if (PrintSymTables) {
	//	PrintSymbolTable(stdout, Externals);
	//}

	// （4）删除临时文件 & 词、语法分析结果检查
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

	// （5）语义检查 sem-check.c 这是一个对前端语法树的语义检查文件
	PhaseName = "Semantic Check";
	if (Errors == 0 && SemanticCheck)
		Program = SemanticCheckProgram(Program);

	// （6）活跃变量分析 analyze.c
	PhaseName = "Analyze";
	if (Errors == 0 && Analyze)
		AnalyzeProgram(Program);

	//（7）添加变量重命名
	PhaseName = "VariableRename";
	if (Errors == 0 && VariableRename)
	{
		Program = VariableRenameProgram(Program);
	}
	ResetASTSymbolTable(VariableRenameTable);

	// （8）常量传播
	PhaseName = "Propagate";
	if (Errors == 0 && Propagate)
	{
		Program = PropagateProgram(Program);
		gIsAfterPropagate = TRUE;
	}

	//（9）打印抽象语法树
	FILE *fp = fopen("SyntaxTree.txt", "w");
	PrintList(fp, Program, -1);
	if (PrintAST)
	{
		PrintList(stdout, Program, -1);
		fprintf(stdout, "\n");
	}

	// （10）语法树到平面图， SSG 是 StaticStreamGraph 对象
	PhaseName = "AST2FlatSSG";
	if (Errors == 0 && AST2FlatSSG)
		SSG = AST2FlatStaticStreamGraph(gMainComposite);

	// （11）判断是否是动态数据流程序，查找uncertainty窗口，yangshengzhe

	PhaseName = "DSGSearch";
	if (Errors == 0 && hasUncertainty(SSG))
	{
		if (Errors == 0 && CheckSplitUncertainty(SSG))
		{
			/*进入X86的动态数据流后端，修改后端，关闭X86,单独处理与其他后端无冲突*/
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

	// （12）对平面图各节点进行工作量估计函数在GenerateWorkEst.cpp中实现
	PhaseName = "WorkEstimate";
	if (Errors == 0 && WorkEstimate)
		GenerateWorkEst(SSG, WorkEstimateByDataFlow);
	//编译前端+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++end

	//异构集群后端+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++bengin
	/*调度划分添加异构集群模块 --- 20151206 yrr添加*/
	if (0 == Errors && HeterogeneousClusterBackend)
	{
		/*substring为源程序文件名，直接进入异构集群后端，与其它后端无冲突*/
		hClusterBanckend(substring);
	}//if

	//单机X86、X10、GPU后端standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++begin

	// （1）对平面图进行初始化调度和稳态调度 SSG是由前端步骤(10)由语法树转化
	PhaseName = "schedulerSSG";
	if (Errors == 0 && SchedulingFlatSSG)
		SSSG = SchedulingSSG(SSG);

	// 对平面图进行复制分裂 2017-0-30 wl添加
	PhaseName = "duplicatFission";
	if (Errors == 0 && DuplicatFission)
		SSSG = duplicatFission(SSSG, InputFileName);
	return SSSG;
}

#endif