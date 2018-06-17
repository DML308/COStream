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
#include "HorizontalFission.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#include "ClusterPartition.h"
#include "ClusterAmortizing.h"
#include "ActorEdgeInfo.h"
//#include "AmplifyFactorChoose.h"
#include "ClusterStageAssignment.h"
//#include "MPIBackendGenerator.h"
#include "ClusterHorizontalFission.h"
//#include "AutoProfiling.h"
#include "GPUClusterPartition.h"
#include "HAFLPartition.h"
#include "GPUSetMultiNum.h"
#include "TemplateClass.h"
#include "DNBPartition.h"

#ifndef WIN32
#include "FileGenerateHelper.h"
#else
#endif

#ifdef WIN32

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
PRIVATE const char *input_file    = NULL;
GLOBAL const char *output_file   = NULL;
PRIVATE const char *stdin_name    = NULL;

/*FileReader鍜孎ileWriter鐨勮矾寰