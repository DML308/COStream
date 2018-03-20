/*************************************************************************
 *
 *  C-to-C Translator
 *
 *  Adapted from Clean ANSI C Parser
 *  Eric A. Brewer, Michael D. Noakes
 *  
 *  main.c,v
 * Revision 1.22  1995/05/05  19:18:28  randall
 * Added #include reconstruction.
 *
 * Revision 1.21  1995/04/21  05:44:28  rcm
 * Cleaned up data-flow analysis, and separated into two files, dataflow.c
 * and analyze.c.  Fixed void pointer arithmetic bug (void *p; p+=5).
 * Moved CVS Id after comment header of each file.
 *
 * Revision 1.20  1995/04/09  21:30:48  rcm
 * Added Analysis phase to perform all analysis at one place in pipeline.
 * Also added checking for functions without return values and unreachable
 * code.  Added tests of live-variable analysis.
 *
 * Revision 1.19  1995/03/23  15:31:12  rcm
 * Dataflow analysis; removed IsCompatible; replaced SUN4 compile-time symbol
 * with more specific symbols; minor bug fixes.
 *
 * Revision 1.18  1995/02/13  02:00:13  rcm
 * Added ASTWALK macro; fixed some small bugs.
 *
 * Revision 1.17  1995/02/10  22:11:59  rcm
 * -nosem, -notrans, etc. options no longer toggle, so they can appear more than
 * once on the command line with same meaning.  Added -- option to accept
 * unknown options quietly.
 *
 * Revision 1.16  1995/02/01  07:33:18  rcm
 * Reorganized help message and renamed some compiler options
 *
 * Revision 1.15  1995/02/01  04:34:50  rcm
 * Added cc compatibility flags.
 *
 * Revision 1.14  1995/01/25  21:38:17  rcm
 * Added TypeModifiers to make type modifiers extensible
 *
 * Revision 1.13  1995/01/20  03:38:07  rcm
 * Added some GNU extensions (long long, zero-length arrays, cast to union).
 * Moved all scope manipulation out of lexer.
 *
 * Revision 1.12  1995/01/11  17:19:16  rcm
 * Added -nopre option.
 *
 * Revision 1.11  1995/01/06  16:48:51  rcm
 * added copyright message
 *
 * Revision 1.10  1994/12/23  09:18:31  rcm
 * Added struct packing rules from wchsieh.  Fixed some initializer problems.
 *
 * Revision 1.9  1994/12/20  09:24:05  rcm
 * Added ASTSWITCH, made other changes to simplify extensions
 *
 * Revision 1.8  1994/11/22  01:54:30  rcm
 * No longer folds constant expressions.
 *
 * Revision 1.7  1994/11/10  03:15:41  rcm
 * Added -nosem option.
 *
 * Revision 1.6  1994/11/03  07:38:41  rcm
 * Added code to output C from the parse tree.
 *
 * Revision 1.5  1994/10/28  18:58:53  rcm
 * Fixed up file headers.
 *
 * Revision 1.4  1994/10/28  18:52:29  rcm
 * Removed ALEWIFE-isms.
 *
 * Revision 1.3  1994/10/25  20:51:24  rcm
 * Added single makefile
 *
 * Revision 1.2  1994/10/25  15:52:13  bradley
 * Added cvs Log and pragma ident to file.
 *
 *
 *  May 27, 1993  MDN Added support to call genir
 *
 *  Created: Tue Apr 27 13:17:36 EDT 1993
 *
 *
 * 
 * Copyright (c) 1994 MIT Laboratory for Computer Science
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE MIT LABORATORY FOR COMPUTER SCIENCE BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the MIT Laboratory for
 * Computer Science shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without prior
 * written authorization from the MIT Laboratory for Computer Science.
 *
 *************************************************************************/
#pragma ident "main.c,v 1.22 1995/05/05 19:18:28 randall Exp Copyright 1994 Massachusetts Institute of Technology"

#include <iostream>
#include <cstdio>

//#include "CollapseDataParallelism.h"
#include "HorizontalFusionSSG.h"
#include "PartitionSet.h"
#include "CodeGeneration.h"
#include "DynamicX86Backend/DSGCodeGeneration.h"
#include "HorizontalFission.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#include "X86Cluster/ClusterPartition.h"
#include "X86Cluster/ClusterAmortizing.h"
#include "ActorEdgeInfo.h"
//#include "AmplifyFactorChoose.h"
#include "X86Cluster/ClusterStageAssignment.h"
//#include "MPIBackendGenerator.h"
#include "X86Cluster/ClusterHorizontalFission.h"
//#include "AutoProfiling.h"
#include "GPUClusterPartition.h"
#include "MAFLPartition.h"
#include "GPUSetMultiNum.h"

#include "DynamicX86Backend/Utils.h"
#include "DynamicX86Backend/DynamicX86Backend.h"
/*包含异构集群后端所需头文件 2016/04/28 yrr*/
#include "HeterogeneousCluster/HClusterBackend.h"

#include "GreedyPartition.h"
#include "DuplicatFission.h"

#ifdef WIN32
/*解决vs2015与之前版本不一致问题,非2015版本注释掉此行*/
//extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }


extern "C"{
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef NO_POPEN
#ifdef NO_PROTOTYPES
  extern FILE *popen(const char *, const char *);
  extern int  pclose(FILE *pipe);
#endif
#endif

/* Set to 1 to enable parser debugging.  Also requires YACC flags */
  extern int yydebug;

GLOBAL Bool CallModelEmbed = FALSE;//默认为独立执行
GLOBAL Bool EmbedModeUseThreadPool = TRUE; // 嵌入/混合编程方式下是否使用线程池 add by mobinsheng

GLOBAL const char * Executable;
GLOBAL const float VersionNumber = 0.6;
GLOBAL const char * const VersionDate = __DATE__;
GLOBAL const char *PhaseName = "???";

#define CPP_FLAG_LIMIT 2048

GLOBAL int WarningLevel = WARNING_LEVEL; /* values 1-5 are legal; 5 == all */

GLOBAL List *Program;

GLOBAL Node *gMainComposite = NULL; //New COStream
GLOBAL extern StaticStreamGraph *SSG = NULL; // New COStream

GLOBAL SchedulerSSG *SSSG = NULL; // New CoStream
GLOBAL extern FILE *yyin;  /* file pointer used by the lexer */
GLOBAL const char *input_file = NULL; //modify by wangliang
GLOBAL const char *output_file   = NULL;
PRIVATE const char *stdin_name    = NULL;

/*FileReader和FileWriter的路径*/
GLOBAL const char *infileName   = NULL;
GLOBAL const char *outfileName   = NULL;
GLOBAL int posbuf = -2; 

PRIVATE const char *default_preproc = DEFAULT_PREPROC;
PRIVATE const char *ansi_preproc = ANSI_PREPROC;
PRIVATE const char *preproc; /* = default_preproc, initialized in main */

PRIVATE char cpp_flags[CPP_FLAG_LIMIT];
PRIVATE int cpp_flags_index = 0;
PRIVATE Bool piped_input = FALSE;

#ifdef NO_POPEN
PRIVATE char tmpname[L_tmpnam];  /* temporary filename */
#endif

#ifndef WIN32
#define MAXPathSize 100
GLOBAL char COStreamPath[MAXPathSize];
#endif

/* global flags */
GLOBAL Bool IsInCompositeParam     = FALSE;/* SPL */
GLOBAL Bool gIsAfterPropagate = FALSE; // 是否进行了常量传播

GLOBAL Bool QuietlyIgnore     = FALSE;
GLOBAL Bool DebugLex          = FALSE;
GLOBAL Bool PrintSymTables    = FALSE;
GLOBAL Bool PrintPreproc      = FALSE;
GLOBAL Bool TrackInsertSymbol = FALSE;
GLOBAL Bool PrintAST          = FALSE;
GLOBAL Bool PrintLineOffset   = FALSE;
GLOBAL Bool IgnoreLineDirectives = FALSE;
GLOBAL Bool VariableRename    = FALSE; // ly-20121009-变量重命名开关
GLOBAL Bool ANSIOnly          = FALSE;
GLOBAL Bool PrintLiveVars     = FALSE;

GLOBAL Bool Preprocess        = TRUE;
GLOBAL Bool SemanticCheck     = TRUE;
GLOBAL Bool Analyze           = TRUE;
GLOBAL Bool Propagate        = TRUE; // 常量传播开关
GLOBAL Bool AST2FlatSSG      = TRUE;  // AST转平面图开关

GLOBAL Bool SSG2Graph		  = TRUE; // 打印SSG的dot图开关 
GLOBAL Bool Transform         = FALSE;
//正对不同的后端使用的不同开关
GLOBAL Bool SchedulingFlatSSG  = TRUE; // 对平面图进行初始化调度 和稳态调度开关*/
GLOBAL Bool WorkEstimate	  = TRUE; // 工作量估计——静态工作量估计
GLOBAL Bool MakeProfile       = FALSE; // 生成actor工作量的profile文件（通过运行代码做工作量估计） 
GLOBAL Bool WorkEstimateByDataFlow = FALSE;	//使用数据流估计稳态工作量
GLOBAL Bool Speedup			  = TRUE;/*开关：打印加速比情况*/

/************Scheduling policy *******************/
GLOBAL Bool PPartition		  = TRUE;/*开关：划分（划分方法有：循环分发调度RRS，束调度BS（metis））多核*/
GLOBAL Bool PartitionGraph	  = TRUE;/*开关：打印Partition的结果图*/
GLOBAL Bool RRS				  = FALSE;/*开关：循环分发调度*/
GLOBAL Bool BS				  =TRUE ;/*开关: 束调度*/
GLOBAL Bool TDOB			  = FALSE;/*开关: 自顶向下的负载均衡调度*/
GLOBAL Bool	RHFission = FALSE;/*开关：Replicate horizontal fission*/ //zww -20120316
GLOBAL double BalanceFactor = 1.1;//add by wangliang 平衡因子初始化为1.1
GLOBAL Bool GAPartition = TRUE;// add by wangliang 开关：使用GAP任务划分算法
GLOBAL Bool DuplicatFission = FALSE;/*add by wangliang 开关：任务水平分裂。
									功能暂时关闭，测试用例VocoderTopLevel无法编译通过，原因是前面的节点无法识别后面声明的节点。
									后来的学弟学妹们可以尝试解决*/
GLOBAL Bool realnCpucore = FALSE;//add by wangliang 设置实际运行的核数
/******************Partition Strategy********************************/   //zww -20120316
/*Cluster partition */
GLOBAL Bool CollapseDataParallelismSSG = TRUE;//SSG数据并行消除的开关
GLOBAL Bool HorizontalFusingSSG = TRUE;//SSG图水平融合的开关
GLOBAL Bool CPPartiotion = TRUE;//集群间的二级划分
GLOBAL Bool CAmortization = TRUE;/*开关:对二级划分的结果进行摊销*/

/*Backend-----codeGeneration define for SPL*/
GLOBAL Bool GenerateDestCode  = TRUE; // 将SDF图转换成目标代码（X10, X86...）
GLOBAL Bool X10Backend = FALSE;/*代码生成选择x10的后端*/
GLOBAL Bool X86Backend = FALSE;/*代码生成选择x86的后端*/
GLOBAL Bool GPUBackend = TRUE;/*代码生成选择GPU的后端*/
GLOBAL Bool MPIBackend = FALSE;/*生成MPI后端代码*/ //zww-20120525
GLOBAL Bool X10DistributedBackEnd = FALSE;
GLOBAL Bool X10ParallelBackEnd = FALSE;

/*添加后端*/
GLOBAL Bool HeterogeneousClusterBackend = FALSE;/*代码生成选择异构集群后端*/ //yrr-20151125
GLOBAL Bool DynamicX86Backend = FALSE; /*代码生成选择X86动态数据流后端，由运行程序决定*/

/*选择代码类型，分为windows和linux***********************************/
GLOBAL Bool Win = FALSE;//y由于setcpu等用的是linux内核的库函数，win下面编译不了，目前lib中没有添加windows版本
GLOBAL Bool Linux = TRUE;
/**********************************/
GLOBAL Bool FileRW = FALSE;//filereader和filewriter的开关
/************************************************************************/
GLOBAL Bool CALRATIO = FALSE;//测试每个线程计算同步比
GLOBAL Bool TRACE = FALSE;//时间测试开关
GLOBAL Bool CHECKBARRIERTIME = FALSE; //只测试各种不同barrier的时间
GLOBAL Bool CHECKEACHACTORTIME =FALSE;//测试稳态后每个actor的steadywork所需要的时间
GLOBAL Bool PrintResult = TRUE;//关闭最后程序的输出(print)
/**********************************/
GLOBAL Bool GenerateOutput    = FALSE; // 将语法树转换成目标C语言开关
GLOBAL Bool FormatReadably    = FALSE;
//———————————————————20121127
/*为X86代码生成额外添加**********************************/
GLOBAL Bool NoCheckBuffer = TRUE;//非边界检查缓冲区，默认为true
GLOBAL Bool AmplifySchedule = FALSE;
GLOBAL int AmplifyFactor = 1;
/********************************/

/*设置线程和进程数目******************************/
GLOBAL int ClusterNum = 4; // *默认初始化集群中机器数目为1
GLOBAL int CpuCoreNum = 8;/*默认初始化为1一台机器中核的数目*/
GLOBAL int GpuNum = 3;//默认的GPU个数
GLOBAL int NThreads = 2;/*默认初始化的threads*/
GLOBAL int NCpuThreads = 1;//cwb cputhread个数初始值

/*为GPU代码生成添加*****************************************/
GLOBAL int MultiNum = 1;     //默认扩大倍数
GLOBAL int LocalSize = 1;    //为数据传输分配空间 用于传输
GLOBAL int LocalSizeofWork = 32;    
GLOBAL Bool IsMod = TRUE;//缓冲区取数据时是否使用取余操作
GLOBAL int PrintFlag = TRUE;//是否打印结果，默认不输出
GLOBAL int Comm_num = 1;  //cwb CPU-GPU通信传输数据的批次数
GLOBAL Bool PrintTime = FALSE;//cwb 打印各部分的运行时间
GLOBAL Bool SetMultiNum = FALSE; //cwb
/**************************************/

/*GPU后端几种划分算法开关*/
GLOBAL Bool MetisFlag = FALSE;/*采用Metis划分*/
GLOBAL Bool GCPFlag = FALSE;   /*采用GCP划分算法*/
GLOBAL Bool MAFLPFlag = TRUE; //混合架构一级划分，CPU、GPU端任务的划分，Mixed Architecture First Level Partition

/******************************设置X86动态数据流后端配置 yangshengzhe*******************/
GLOBAL Bool DivideSSG = FALSE; //划分静态子图开关
GLOBAL Bool Compress = TRUE;	//压缩算法开关
GLOBAL DividedStaticGraph* DSG = NULL;//动静态子图

/******************************设置异构集群配置 yrr*******************/
GLOBAL int hClusterNum = 1;
/*《集群编号，《CPU核数，GPU个数》》*/
GLOBAL std::map<int, std::pair<int, int>> hCluster2CpuAndGpu =
{ { 0,{ 4, 3 } }/*,{ 1,{ 4, 0 } },{ 2,{ 4, 3 }}*/ };

/*当集群为异构时，控制目标代码版本（1：支持OpenCL，在GPU服务器；2：不支持OpenCL，在CPU服务器）*/
GLOBAL Bool CurGpuFlag = TRUE;

/*异构集群后端划分方式*/
GLOBAL Bool PMTMPartition = FALSE; //进程级划分采用Metis，线程级划分采用Metis
GLOBAL Bool PGTMPartition = FALSE;  //进程级划分采用Group，线程级划分采用Metis
GLOBAL Bool PGTDPartition = TRUE;
//GLOBAL Bool MemoryOptimiZation = TRUE;

/*结果统计信息开关*/
GLOBAL Bool RUNTIME = FALSE; //测执行时间-------------用于收集集群后端代码在运行时的信息主要是整个程序的执行时间，使用该开关是MPI_COLLECTRUNTIME_INFO要关闭
};

string InputFileName;

PRIVATE void print_version_info(FILE *out, const char *pre)
{
  fprintf(out, "%sVersion %.02f (%s)\n",
   pre, VersionNumber, VersionDate);
  exit(0);
}


PRIVATE void print_copyright(FILE *out, const char *pre)
{
  static const char *lines[] = {
    "Copyright (c) 1994 MIT Laboratory for Computer Science\n",
    NULL };
    int i;

    for (i=0; lines[i] != NULL; i++) {
     fputs(pre, out);
     fputs(lines[i], out);
   }
   exit(0);
 }


 PRIVATE void add_cpp_flag(const char *flag)
 {
  /* Quote flag with single quotes, escaping any single quotes that
     appear in flag.  This code only works if system() and popen() 
     use sh as the command shell. */

  const char *src = flag;
  char *dest = &cpp_flags[cpp_flags_index];

  strcpy(dest, " '"); /* starting quote */
  dest+=2;

  for (; *src; ++src) {
    if (*src == '\'') {
      strcpy(dest, "'\\''");
      dest+=4;
    }
    else
      *dest++ = *src;
  }
  strcpy(dest, "'"); /* ending quote */
  dest++;

  cpp_flags_index += dest - &cpp_flags[cpp_flags_index];
}


PRIVATE void usage(Bool print_all_options, int exitcode)
{
  fprintf(stderr, "Usage: %s [options] [file]\n", Executable);

  fprintf(stderr, 
    "\n"
    "Parses <file> as a C program, reporting syntax and type errors, and writes\n"
    "processed C program out to <file>%s.  If <file> is omitted, uses \n"
    "standard input and standard output.\n"
    "\n",
    OUTPUT_SUFFIX);

  fprintf(stderr, "General Options:\n");
  fprintf(stderr,
   "\t-help              Print this description\n");
  fprintf(stderr,
   "\t-options           Print all options\n");
  fprintf(stderr,
   "\t-copy              Print the copyright information\n");
  fprintf(stderr,
   "\t-v                 Print version information\n");

  if (print_all_options) {
    fprintf(stderr, "Phase Options:\n");
    fprintf(stderr,
     "\t-nopre             Don't preprocess\n");
    fprintf(stderr,
     "\t-nosem             Don't semantic-check\n");
    fprintf(stderr,
     "\t-noanalyze         Don't perform dataflow analysis\n");
    fprintf(stderr,
     "\t-notrans           Don't transform syntax tree\n");
    fprintf(stderr,
     "\t-noprint           Don't write C output\n");

    fprintf(stderr, "Output Options:\n");
    fprintf(stderr,
     "\t-o <name>          Write C output to <name>\n");
    fprintf(stderr,
     "\t-N                 Don't emit line directives\n");

    fprintf(stderr, "Warning Options:\n");
    fprintf(stderr,
     "\t-ansi              Disable GCC extensions and undefine __GNUC__\n");
    fprintf(stderr,
     "\t-W<n>              Set warning level; <n> in 1-5. Default=%d\n",
     WARNING_LEVEL);
    fprintf(stderr,
     "\t-Wall              Same as -W5\n");
    fprintf(stderr,
     "\t-il                Ignore line directives (use actual line numbers)\n");
    fprintf(stderr,
     "\t-offset            Print offset within the line in warnings/errors\n");
    fprintf(stderr,
     "\t-name <x>          Use stdin with <x> as filename in messages\n");
    
    fprintf(stderr, "Preprocessing Options:\n");
    fprintf(stderr,
     "\t-P<str>            Set the preprocessor command to <str>\n");
    fprintf(stderr,
     "\t-pre               Print the preprocessor command and flags\n");
    fprintf(stderr,
     "\t-I<path>           Specify path to search for include files\n");
    fprintf(stderr,
     "\t-Dmacro[=value]    Define macro (with optional value)\n");
    fprintf(stderr,
     "\t-Umacro            Undefine macro\n");
    fprintf(stderr,
     "\t-H                 Print the name of each header file used\n");
    fprintf(stderr,
     "\t-undef             Do not predefine nonstandard macros\n");
    fprintf(stderr,
     "\t-nostdinc          Do not scan standard include files\n");
    fprintf(stderr, "Debugging Options:\n");
    fprintf(stderr,
     "\t-lex               Show lexical tokens\n");
    fprintf(stderr,
     "\t-yydebug           Track parser stack and actions\n");
    fprintf(stderr,
     "\t-insert            Track symbol creation\n");
    fprintf(stderr,
     "\t-sym               Print out symbol tables after parse\n");
    fprintf(stderr,
     "\t-ast               Print out syntax tree (after last phase)\n");
    fprintf(stderr,
     "\t-live              Print live variables as cmts in C output\n");
    fprintf(stderr, "CC Compatibility Options:\n");
    fprintf(stderr,
     "\t--                 Toggles ignoring of unknown options\n"
     "\t                   (for makefile compatibility with cc)\n"); 
    
    fprintf(stderr, "\n");
  }

  exit(exitcode);
}

PRIVATE void unknown_option(char *option)
{
  if (!QuietlyIgnore) {
    fprintf(stderr, "Unknown option: `%s'\n\n", option);
    usage(FALSE, 1);
  }
}

/* Generate a filename with a new suffix.
   If <filename> ends with <old_suffix>, replace the suffix with <new_suffix>;
   otherwise just append <new_suffix>. */
PRIVATE const char *with_suffix(const char *filename, 
  const char *old_suffix, 
  const char *new_suffix)
{ int root_len, old_len, len;
  char *newfilename;

  /* Look for old_suffix at end of filename */
  root_len = strlen(filename);
  old_len = strlen(old_suffix);
  if (root_len >= old_len && 
    !strcmp(filename + root_len - old_len, old_suffix))
    root_len -= old_len;
  
  /* Compute the length of the create filename */
  len = root_len + strlen(new_suffix) + 1;

  /* allocate the create name */
  if ((newfilename = HeapNewArray(char, len)) == NULL) {
    printf("INTERNAL ERROR: Unable to allocate %d bytes for a filename\n", 
      len);
    exit(-1);
  }

  strncpy(newfilename, filename, root_len);
  strcat(newfilename, new_suffix);

  return newfilename;
}


/***********************************************************************\
 * Handle command-line arguments
\***********************************************************************/

PRIVATE void handle_options(int argc, char *argv[])
{
  int i;
  #ifdef WIN32
  
  char *files[256] = { "./tests/SPLtest/Benchmarks/00-test/wang.cos",//此用例用于测试任务水平分裂 modify by wangliang
  "./tests/SPLtest/Benchmarks/01-FilterBank/FilterBank6.cos",
  "./tests/SPLtest/Benchmarks/02-Serpent_full/Serpent_full.cos",
  "./tests/SPLtest/Benchmarks/03-FMRadio/FMRadio.cos",
  "./tests/SPLtest/Benchmarks/04-BeamFormer1/BeamFormer1.cos",
  "./tests/SPLtest/Benchmarks/05-DES2/DES2.cos",
  "./tests/SPLtest/Benchmarks/06-FFT5/FFT5.cos",
  "./tests/SPLtest/Benchmarks/07-VocoderTopLevel/VocoderTopLevel.cos",
	  "./tests/SPLtest/Benchmarks/08-BitonicSort2/BitonicSort2.cos", //multwo.spl2.cc  BitonicSort2.spl2.cc
	  "./tests/SPLtest/Benchmarks/09-DCT2/DCT2.cos",
	  "./tests/SPLtest/Benchmarks/10-ChannelVocoder7/ChannelVocoder7.cos",
	  "./tests/SPLtest/Benchmarks/11-Tde_pp/tde_pp.cos",
	  "./tests/SPLtest/Benchmarks/12-MPEGdecoder_nomessage_5_3/MPEGdecoder_nomessage_5_3.cos",
	  "./tests/SPLtest/Benchmarks/13-opensurf4/openSURF4.cos",
	  "./tests/SPLtest/Benchmarks/14-FFT-Dynamic/FFT5kuoda.cos",
	  "./tests/SPLtest/Benchmarks/15-mytest/struct_test.cos",
	  "./tests/SPLtest/Benchmarks/16-SAO/SAO_propagator_check.cos",
	  "./tests/SPLtest/Benchmarks/17-Compress/CU_P_final.cos",
	  "./tests/SPLtest/Benchmarks/18-DeblockAndSAO/DeblockAndSao_4.cos",
	  "./tests/SPLtest/Benchmarks/19-intra/intra.cos",
	  "./tests/SPLtest/Benchmarks/20-inter/inter.cos" ,
	  "./tests/SPLtest/Benchmarks/21-hevc/hevc_entropy.cos",
	  "./tests/SPLtest/Benchmarks/22-frwtest/Rect.cos" //测试文件输入输出 内置composite FileWriter modify by 陈名韬

  };

  char *hClusterFiles[9] = {
   "./tests/SPLtest/hClusterBenchmarks/01-FilterBank/FilterBank6_8.cos",
   "./tests/SPLtest/hClusterBenchmarks/02-Serpent_full/Serpent_full-2_2.cos",
   "./tests/SPLtest/hClusterBenchmarks/03-FMRadio/FMRadio.cos",
   "./tests/SPLtest/hClusterBenchmarks/04-BeamFormer1/BeamFormer1.cos",
   "./tests/SPLtest/hClusterBenchmarks/05-DES2/DES2.cos",
   "./tests/SPLtest/hClusterBenchmarks/06-FFT5/FFT5.cos", 
   "./tests/SPLtest/hClusterBenchmarks/08-BitonicSort2/BitonicSort2.cos", 
   "./tests/SPLtest/hClusterBenchmarks/09-DCT2/DCT2.cos"
 };

 if (X86Backend)
 {
   argc = 5;
   argv[1] = "-x86";  
   argv[2] = "-nCpucore";
   argv[3] = "4";
   argv[4] = files[06];
 }
 else if (GPUBackend)
 {
   argc = 9;
   argv[1] = "-gpu";  
   argv[2] = "-nGpu";
   argv[3] = "3";
   argv[4] = files[05];
   argv[5] = "-o";
   argv[6] = "test";
   argv[7] = "-multi";
   argv[8] = "3";
 }
  /*异构集群后端 2016/01/14 yrr*/
 else if (HeterogeneousClusterBackend)
 {
   argc = 9;
   argv[1] = "-hCluster";
   argv[2] = "-nCluster";
   argv[3] = "4";
   argv[4] = hClusterFiles[0];
   argv[5] = "-o";
   argv[6] = "test";
   argv[7] = "-multi";
   argv[8] = "3";
 }
 
#else
 if(getcwd(COStreamPath,MAXPathSize)==NULL)
   exit(1);
 string curPath = COStreamPath;
 string tempPath =  argv[0];
 cout<<curPath<<endl<<tempPath<<endl;  

 int pos1 = tempPath.find_first_of('/'),pos2=-1;
 while(pos1!=-1)
 {
  string sub = tempPath.substr(pos2+1,pos1-pos2);
  strcpy(COStreamPath,sub.c_str());
  if(chdir(COStreamPath)==-1){
    cout<<"error"<<endl;
  }
  pos2 = pos1;
  pos1 = tempPath.find_first_of('/',pos1+1);
  
}
if(getcwd(COStreamPath,MAXPathSize)==NULL)
 exit(1);
if(chdir(curPath.c_str())==-1)
 cout<<"error";
tempPath = COStreamPath;
tempPath = tempPath.substr(0,tempPath.size()-4);
cout<<tempPath<<endl;
strcpy(COStreamPath,tempPath.c_str());
#endif
for (i=1; i<argc; i++) {
  if (argv[i][0] == '-') {
    switch (argv[i][1]) {
      case '-':
      QuietlyIgnore = !QuietlyIgnore;
      break;
	//Increase the mixed mode command option donghui
      case 'd':
      CallModelEmbed = TRUE;
      break;
      case 'a':
      if (strcmp(argv[i], "-ansi") == 0) {
       ANSIOnly = TRUE;
	  /* change the preprocessor command, if the user hasn't
	     already changed it with -P */
       if (preproc == default_preproc)
         preproc = ansi_preproc;
     }
     else if (strcmp(argv[i], "-ast") == 0) 
       PrintAST = TRUE;
     else
       unknown_option(argv[i]);
     break;

     case 'D':
     case 'U':
     case 'I':
     add_cpp_flag(argv[i]);
     break;
     case 'H':
     if (strcmp(argv[i], "-H") == 0)
       add_cpp_flag(argv[i]);
     else
       unknown_option(argv[i]);
     break;
     case 'P':
     preproc = &argv[i][2];
#if 0
/* didn't seem necessary -- rcm */
	fprintf(stderr, "Preprocessor set to `%s'\n", preproc);
#endif
     break;
     case 'N':
     FormatReadably = TRUE;
     break;
     case 'W':
     if (strcmp(argv[i], "-Wall")==0) {
       WarningLevel = 5;
     } 
     else {
       int c = atoi(&argv[i][2]);
       if (c < 1 || c > 5) {
         unknown_option(argv[i]);
       } else {
         WarningLevel = c;
       }
     }
     break;
     case 'c':
     if (strcmp(argv[i], "-copy") == 0)
       print_copyright(stderr, "");
     else if(strcmp(argv[i],"-cluster") == 0)
     {
      MPIBackend = TRUE;
		ClusterNum = atoi(argv[++i]);//zww 20120903 添加参数解析
	}
	else
		unknown_option(argv[i]);
	break;
  case 'i':
  if (strcmp(argv[i], "-insert")==0)
   TrackInsertSymbol = TRUE;
 else if (strcmp(argv[i], "-imacros") == 0 ||
   strcmp(argv[i], "-include") == 0) {
   add_cpp_flag(argv[i++]);
 add_cpp_flag(argv[i]);
} else if (strcmp(argv[i], "-il")==0) {
 IgnoreLineDirectives = TRUE;
} else unknown_option(argv[i]);
break;
case 'l':
if (strcmp(argv[i], "-lex")==0)
 DebugLex = TRUE;
else if (strcmp(argv[i], "-live")==0)
 PrintLiveVars = TRUE;
else unknown_option(argv[i]);
break;
case 'm':
if (strcmp(argv[i], "-multi") == 0)
{
 MultiNum = atoi(argv[i + 1]);
 i++;
}
else unknown_option(argv[i]);
break;
case 'e':
if (strcmp(argv[i], "-embed") == 0)
 CallModelEmbed = TRUE;
else unknown_option(argv[i]);
break;
case 'n':
if (strcmp(argv[i], "-name")==0) {
 i++;
 if (input_file != NULL) {
   fprintf(stderr,
    "Multiple input files defined, using `%s'\n",
    input_file);
 } else {
   stdin_name = argv[i];
 }
} else if (strcmp(argv[i], "-nostdinc")==0) {
 add_cpp_flag(argv[i]);
} else if (strcmp(argv[i], "-nosem")==0) {
 SemanticCheck = FALSE;
} else if (strcmp(argv[i], "-notrans")==0) {
 Transform = FALSE;
} else if (strcmp(argv[i], "-noprint")==0) {
 GenerateOutput = FALSE;
} else if (strcmp(argv[i], "-nopre")==0) {
 Preprocess = FALSE;
} else if (strcmp(argv[i], "-noanalyze") == 0) {
 Analyze = FALSE;
} else if (strcmp(argv[i], "-nCpucore") == 0){
  CpuCoreNum = atoi(argv[++i]);
}else if (strcmp(argv[i], "-nGpu") == 0){
  GpuNum = atoi(argv[++i]);
}
else if (strcmp(argv[i], "-nCluster") == 0)
{
  ClusterNum = atoi(argv[++i]);
}
else unknown_option(argv[i]);
break;
case 'o':
if (strcmp(argv[i], "-o")==0) {
 i++;
 output_file = argv[i];
}
else if (strcmp(argv[i], "-offset")==0)
 PrintLineOffset = TRUE;
else if (strcmp(argv[i], "-options")==0) {
 usage(TRUE, 0);
 system("pause");
 exit(0);
}
else unknown_option(argv[i]);
break;
case 'p':
if (strcmp(argv[i], "-pre") == 0)
 PrintPreproc  = TRUE;
else
 unknown_option(argv[i]);
break;
case 'r':
	if(strcmp(argv[i], "-ratio")==0)//同步加速比，lihe 2013-01-14
		CALRATIO = TRUE;
	else if(strcmp(argv[i], "-result")==0)//是否在终端打印程序输出结果，默认为打印，lihe 2013-01-14
	{
		i++;
		if(strcmp(argv[i], "no")==0)
			PrintResult = FALSE;
		else if(strcmp(argv[i], "yes")==0)
			PrintResult = TRUE;
		else
			unknown_option(argv[i]);
	}
	else unknown_option(argv[i]);
	break;
  case 's':
  if (strcmp(argv[i], "-sym")==0)
   PrintSymTables = TRUE;
 else unknown_option(argv[i]);
 break;
 
	case 't'://记录程序执行时间轨迹，lihe 2013-01-14
	if (strcmp(argv[i], "-trace")==0)
   {TRACE = TRUE;
   }
   else unknown_option(argv[i]);
   break;
   case 'u':
   if (strcmp(argv[i], "-undef")==0)
     add_cpp_flag(argv[i]);
   else unknown_option(argv[i]);
   break;
   case 'v':
   if (strcmp(argv[i], "-v")==0)
     print_version_info(stderr, "");
   else unknown_option(argv[i]);
   break;
   case 'y':
   if (strcmp(argv[i], "-yydebug") == 0)
     yydebug = 1;
   else
     unknown_option(argv[i]);
   break;
   case 'g':
   if(strcmp(argv[i],"-gpu") == 0)
     GPUBackend = TRUE;
   break;
	  /*异构集群后端*/
   case 'h':
   if (strcmp(argv[i], "-hCluster") == 0)
     HeterogeneousClusterBackend = TRUE;
   break;
   case 'x':
   if(strcmp(argv[i],"-x10") == 0)
     X10Backend = TRUE;
   else if(strcmp(argv[i],"-x86") == 0)
     X86Backend = TRUE;
   else
     unknown_option(argv[i]);
   break;
   default:
   unknown_option(argv[i]);
 }
} else {
  if (input_file != NULL) {
   fprintf(stderr, "Multiple input files defined, using `%s'\n",
    argv[i]);
 }
 input_file = argv[i];
}
}

if (GenerateOutput == TRUE && input_file != NULL && output_file == NULL)
  output_file = with_suffix(input_file, INPUT_SUFFIX, OUTPUT_SUFFIX);
   InputFileName = input_file; //zww 20120903 修改
 }


/***********************************************************************\
 * ANSI C symbol tables
\***********************************************************************/

 GLOBAL SymbolTable *Identifiers, *Labels, *Tags, *Externals;
/***********************--------------Define For SPL----------****************************/
 GLOBAL SymbolTable *CompositeIds;
GLOBAL ASTSymbolTable *ToTransformDecl;/*zww：动态匹配使用的符号表,展开时使用*/
GLOBAL ASTSymbolTable *ParameterPassTable;/*zww:该符号表存放参数和输入、输出流的对应信息*/
GLOBAL ASTSymbolTable *VariableRenameTable;/*ly:该符号表用于变量重命名*/

/***********************--------------Define For SPL----------****************************/

 PRIVATE void shadow_var(Node *create, Node *shadowed)
 {
    /* the two are equal only for redundant function/extern declarations */
  if (create != shadowed  && WarningLevel == 5) {
   WarningCoord(5, create->coord,
     "`%s' shadows previous declaration", VAR_NAME(create));
   fprintf(stderr, "\tPrevious declaration: ");
   PRINT_COORD(stderr, shadowed->coord);
   fputc('\n', stderr);
 }
}


PRIVATE void init_symbol_tables(Bool shadow_warnings)
{
  ShadowProc shadow_proc;

  if (shadow_warnings)
    shadow_proc = (ShadowProc) shadow_var;
  else
    shadow_proc = NULL;

  Identifiers = NewSymbolTable("Identifiers", Nested,
   shadow_proc, (ExitscopeProc) OutOfScope);

  Labels = NewSymbolTable("Labels", Flat,
   NULL, (ExitscopeProc) EndOfLabelScope);

  Tags = NewSymbolTable("Tags", Nested,
    shadow_warnings ? (ShadowProc)ShadowTag : (ShadowProc)NULL,
    NULL);

  Externals = NewSymbolTable("Externals", Flat,
    NULL, (ExitscopeProc) OutOfScope);

/***********************--------------Define For SPL----------****************************/
	/*StreamIds = NewSymbolTable("StreamIds", Nested,
		shadow_warnings ? (ShadowProc)ShadowTag : (ShadowProc)NULL,
		NULL);*/

  CompositeIds = NewSymbolTable("CompositeIds", Flat,
    NULL, (ExitscopeProc) OutOfScope);

	ToTransformDecl = NewASTSymbolTable("ToTransformDecl",Flat);//zww

	ParameterPassTable = NewASTSymbolTable("ParameterPassTable",Flat);//zww

	VariableRenameTable = NewASTSymbolTable("VariableRenameTable",Flat);//ly

	/*OperatorIds = NewSymbolTable("OperatorIds", Nested,
		shadow_warnings ? (ShadowProc)ShadowTag : (ShadowProc)NULL,
		NULL);*/
}


/***********************************************************************\
 * Determine input file, preprocess if needed
\***********************************************************************/

PRIVATE FILE *get_preprocessed_input()
{
  FILE *in_file;
  
  if (Preprocess && input_file != NULL) {
    char command[2048];
    
    if (PrintPreproc)
      fprintf(stderr, "Preprocessing: %s %s %s\n", preproc,
       cpp_flags, input_file);
#ifdef NO_POPEN
    tmpname[0] = 0;
    tmpnam(tmpname);  /* get a temporary filename */
    sprintf(command, "%s %s %s > %s", preproc, cpp_flags,
     input_file, tmpname);
    /* the following assumes that "system" returns nonzero if
       the command fails, which is not required by ANSI C */
    if (system(command)) {
      fprintf(stderr, "Preprocessing failed.\n");
      remove(tmpname);
      exit(10);
    }
    input_file = tmpname;
    in_file = fopen(input_file, "r");
    if (in_file == NULL) {
      fprintf(stderr,
       "Unable to read input file \"%s\".\n", input_file);
      if (tmpname[0] != 0) remove(tmpname);
      exit(1);
    }
#else
    sprintf(command, "%s %s %s", preproc, cpp_flags, input_file);
    in_file = popen(command, "r");
    if (in_file == NULL) {
      fprintf(stderr, "Unable to preprocess input file \"%s\".\n",
       input_file);
      exit(1);
    }
    piped_input = TRUE;
#endif
    SetFile(input_file, 0);
  } else {
    fprintf(stderr, "(Assuming input already preprocessed)\n");

    if (input_file != NULL) {
      in_file = fopen(input_file, "r");
      if (in_file == NULL) {
       fprintf(stderr,
        "Unable to read input file \"%s\".\n", input_file);
       exit(1);
     }
     SetFile(input_file, 0);
   }
   else {
    if (stdin_name == NULL) stdin_name = "stdin";
    in_file = stdin;
    SetFile(stdin_name, 0);
  }
}
return(in_file);
}


/***********************************************************************\
 * Main
\***********************************************************************/
Bool parsed_ok;
GLOBAL int main(int argc, char *argv[])
{
	Partition *pp = NULL; // TANHONG
	HorizontalFission *hfp = NULL;//zhangweiwei
	Executable = argv[0];
	StageAssignment *pSA;//lihao
	vector<StageAssignment*>pSAList;
	Partition *mp = NULL;
	vector<MetisPartiton*>mplist;
	GPUClusterPartition *gcp=NULL;//chenwenbin
	MAFLPartition *maflp = NULL;

//编译前端+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++begin
	preproc = default_preproc;
  handle_options(argc, argv);


#ifdef WIN32
  int pos1 = InputFileName.find_last_of('/');
  int pos2 = InputFileName.find(".cos");
	/*若pos=-1说明输入源程序不合法*/
  int len = InputFileName.length();
  assert(pos1 != -1 && pos2 != -1 && pos2 == (len - 4));

  string substring = InputFileName.substr(pos1 + 1, pos2 - pos1 - 1);
  string ccfilename = InputFileName.substr(pos1 + 1, pos2 - pos1 - 1);
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
#endif
	//对输入预处理
	yyin = get_preprocessed_input();
	//（1）初始化环境（类型、符号表、操作符表）
	InitTypes();
	init_symbol_tables(TRUE);
	InitOperatorTable();

	/*（2）文法建立和语法树生成，yyparse()函数是内部调用语法分析器
	* 于parser.c 3371行 {Program = GrabPragmas((yyvsp[(1)-(1)].L))};
	* 得到对输入源程序的分析结果-语法树，存在list *program中
	*/
	PhaseName = "Parsing";
	parsed_ok = yyparse();

	if (Level != 0) {
		SyntaxError("unexpected end of file");
	}

	//（3）打印符号表
	if (PrintSymTables) {
		PrintSymbolTable(stdout, Externals);
	}

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
	if (PrintAST)
	{
		PrintList(stdout, Program, -1);
		fprintf(stdout, "\n");
	}

	// （10）语法树到平面图 SSG 是 StaticStreamGraph 对象
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
		goto End;
	}//if

	 //单机X86、X10、GPU后端standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++begin

	PhaseName = "schedulerSSG";
	if (Errors == 0 && SchedulingFlatSSG)
		SSSG = SchedulingSSG(SSG);

	// 对平面图进行复制分裂 add by wangliang 测试用例VocoderTopLevel无法编译通过，原因是前面的节点无法识别后面声明的节点。
	PhaseName = "duplicatFission";
	if (Errors == 0 && DuplicatFission)
		SSSG = duplicatFission(SSSG, InputFileName);

	// （2）用XML文本的形式描述SDF图
	PhaseName = "SSG2Graph";
	if (Errors == 0 && SSG2Graph)
		DumpStreamGraph(SSSG, NULL, "flatgraph.dot", NULL);

	// （3）对节点进行调度划分
	PhaseName = "Partition";
	if (Errors == 0 && PPartition && (X86Backend || X10Backend || GPUBackend))
	{
		if (RRS)
			pp = new RRSPartiton();
		else if (BS)
		{
			if (X10Backend)
			{
				mp = new MetisPartiton(0, 0);//**仅为测试之用，下面的代码为最终可用的代码
			}
			else if (X86Backend)
			{
				if (GAPartition)mp = new GreedyPartition(SSSG);
				else mp = new MetisPartiton(0, 0);//如果是X86backend，则划分的子图不需要是连通的
				
			}
			else if (GPUBackend)
			{
				//选择GPU划分算法
				if (MAFLPFlag)
					maflp = new MAFLPartition();
			}
			pp = mp;
		}//else
		else if (TDOB)
			pp = new TDOBPartition();


		if (X10ParallelBackEnd == TRUE)
		{
			pp->setCpuCoreNum(CpuCoreNum, SSSG);
			pp->SssgPartition(SSSG, 1); // 一级划分
		}
		else if (X10DistributedBackEnd == TRUE)
		{
			pp->setCpuCoreNum(CpuCoreNum, SSSG);
			pp->SssgPartition(SSSG, 1); // 一级划分
			if (BS == TRUE)//TDOB划分算法不会产生死锁
			{
				//束调度可能会产生死锁，Adjust函数调整划分结果以避免死锁
				pp->Adjust(SSSG, 0);
			}
			pp->SssgPartition(SSSG, 2); // 二级划分

		}
		else if (X10Backend || X86Backend)
		{
			//根据SSSG图中的节点个数以及CPU核数设置划分partition份数
			cout << "正在进行任务划分..." << endl;
			pp->setCpuCoreNum(CpuCoreNum, SSSG);
			pp->SssgPartition(SSSG, 1);
			cout << "任务划分完成！" << endl;
		}
		else if (GPUBackend && MAFLPFlag)
		{
			maflp->SetGpuNum(SSSG);
			maflp->SssgPartition(SSSG, 1);
		}
	}

	PhaseName = "SetMultiNum";
	if (Errors == 0 && GPUBackend)
	{
		if (SetMultiNum)
		{
			GPUSetMultiNum * gsm = new GPUSetMultiNum(SSSG);
			gsm->SetMultiNum(SSSG, maflp);
		}
		maflp->SetMultiNum2FlatNode();
	}

	PhaseName = "PartitionGraph";
	if (Errors == 0 && PPartition && PartitionGraph && (X86Backend || X10Backend || GPUBackend))
	{
		if (GPUBackend)
			DumpStreamGraph(SSSG, maflp, "GPUPartitionGraph.dot", NULL);//cwb
		else
			DumpStreamGraph(SSSG, mp, "PartitionGraph.dot", NULL);//zww_20120605添加第四个参数
	}

	/*（4）水平分裂*/

	//if(Errors == 0 && RHFission && (X86Backend || X10Backend) )
	//{
	//	assert(mp);
	//	//mp是上一步骤中完成并保存的初始划分
	//	hfp = new HorizontalFission(mp,1.5);
	//	hfp->HorizontalFissionPRPartitions(SSSG,1.5);
	//	SSSG->ResetFlatNodeNames();
	//	SSSG->InitScheduling();		
	//	DumpStreamGraph(SSSG, mp, "HorizontalPartitionGraph.dot",NULL);//zww_20120605添加第四个参数
	//} 

	/*（5）打印理论加速比*/
	PhaseName = "Speedup";
	if (Errors == 0 && Speedup && (X86Backend || X10Backend))
		if (GAPartition)ComputeSpeedup(SSSG, pp, ccfilename, "workEstimate.txt", "GAPartition");
  else ComputeSpeedup(SSSG, pp, ccfilename, "workEstimate.txt", "Metis");

	/*（6）阶段赋值*/
	if (Errors == 0 && X86Backend)  //zww---20120522为了调试二级划分暂时注释
	{
		//存储阶段赋值的结果
		pSA = new StageAssignment();
		//第一步首先根据SDF图的输入边得到拓扑序列，并打印输出
		pSA->actorTopologicalorder(SSSG->GetFlatNodes());
		//第二步根据以上步骤的节点划分结果，得到阶段赋值结果
		pSA->actorStageMap(mp->GetFlatNode2PartitionNum());
	}
	if (Errors == 0 && GPUBackend && MAFLPFlag)
	{
		pSA = new StageAssignment();
		pSA->actorTopologicalorder(SSSG->GetFlatNodes());
		pSA->actorStageMapForGPU(maflp->GetFlatNode2PartitionNum());
	}
	/*根据L1 cache大小选择扩大执行因子*/

	// （7）输入为SDF图，输出为目标代码
	PhaseName = "CodeGeneration";
	if (Errors == 0 && GenerateDestCode && (X86Backend || X10Backend || GPUBackend))
	{
#ifdef WIN32
		char *tmp = new char[1000];
		getcwd(tmp, 1000);
		//printf("%s\n", tmp);
		ActorEdgeInfo actorEdgeInfo(SSSG);
		CodeGeneration(tmp, SSSG, substring, pSA, mp, maflp);
		delete tmp;
		delete pSA;
		tmp = NULL;
#else
		DIR *dir = NULL;
		DIR *dir1 = NULL;
		char *tmp;

		if (output_file == NULL)
			tmp = "./CppFiles/"; //若未选择输出目录，则默认为当前目录下的CppFiles
		else
		{
			char *buf;

			if (output_file[strlen(output_file) - 1] != '/')
			{
				buf = new char[strlen(output_file) + 2];
				strcpy(buf, output_file);
				buf[strlen(output_file)] = '/';
			}
			else
			{
				buf = new char[strlen(output_file) + 1];
				strcpy(buf, output_file);
			}
			string tmpbuf = buf;
			cout << "tmpbuf =" << tmpbuf << endl;
			posbuf = tmpbuf.find_last_of('/');
			cout << "posbuf =" << posbuf << endl;
			if (posbuf == -1)//只是指定了目标文件的名称
			{
				tmp = "./";
			}
			else
			{
				mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				dir1 = opendir(buf);
				tmp = buf;

			}
		}
		cout << "调试信息1" << endl;
		CodeGeneration(tmp, SSSG, substring, pSA, mp, &actorEdgeInfo);
		//delete tmp;
		delete pSA;
		//tmp = NULL;
#endif	
	}

  End:
	//防止控制台结果一闪消失
  system("pause");
	//单机X86、X10、GPU后端standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++end	

  if (Errors > 0) {
    fprintf(stderr, "\nCompilation Failed: %d error%s, %d warning%s\n",
     Errors, PLURAL(Errors),
     Warnings, PLURAL(Warnings));
  }
  else if (Warnings > 0) {
    fprintf(stderr, "\nCompilation Successful (%d warning%s)\n",
     Warnings,
     PLURAL(Warnings));
  }



	/* cleanup */
#ifdef NO_POPEN
  if (tmpname[0] != 0) remove(tmpname);
#else
  if (piped_input) pclose(yyin);
#endif

  return(Errors);
}
