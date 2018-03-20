#ifndef _WORK_ESTIMATE_H_
#define _WORK_ESTIMATE_H_

#include "ast.h"
#include <stdlib.h>

#define PRINT  3
#define PEEK  3
#define POP  3
#define PUSH  3
#define INT_ARITH_OP  1
#define FLOAT_ARITH_OP  2
#define LOOP_COUNT  5
#define SWITCH  1
#define IF  1
#define CONTINUE  1
#define BREAK  1
#define MEMORY_OP  2
#define METHOD_CALL_OVERHEAD  10
#define UNKNOWN_METHOD_CALL  60
#define INIT 0
#define STEADY 1
#define STREAM_OP 20
#define PRINTLN_OP 60 // modify by wangliang
#define FRTA_OP 60 // modify by wangliang
int totalWork = 0;
Bool isSTREAM = 0;
Bool state;
void WEST_listwalk(List *);
void WEST_astwalk(Node *);
void rWorkCompute(Node *);

#endif /* ifndef _WORK_ESTIMATE_H_ */