/***************************************************************
This code was written by Franz Franchetti
See www.ece.cmu.edu/~franzf and www.spiral.net for details
Copyright (c) 2005, Carnegie Mellon University
All rights reserved.

This code is distributed for evaluation only and may not be 
used for any other purposes. Redistribution without prior consent
by the authors is prohibited.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, reference to Spiral, this list of conditions and the
  following disclaimer.
* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.
* Neither the name of Carnegie Mellon University nor the name of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*AS IS* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************/

#define COUNTER_LO(a) ((a).int32.lo)
#define COUNTER_HI(a) ((a).int32.hi)
#define COUNTER_VAL(a) ((a).int64)

#define COUNTER(a,b) \
	(((double)COUNTER_VAL(a))/(b))

#define COUNTER_DIFF(a,b,c) \
	(COUNTER(a,c)-COUNTER(b,c))

#define CYCLES		1
#define OPERATIONS	1
#define SEC		PROCESSOR_FREQ
#define MILI_SEC	(SEC/1E3)
#define MICRO_SEC	(SEC/1E6)
#define NANO_SEC	(SEC/1E9)

/* ==================== GNU C and possibly other UNIX compilers ===================== */
#if !defined(_WIN32) && !defined(_WIN64)

#if defined(__GNUC__) || defined(__linux__)
#define VOLATILE __volatile__
#define ASM __asm__
#else
/* we can at least hope the following works, it probably won't */
#define ASM asm
#define VOLATILE 
#endif

#define myInt64 unsigned long long
#define INT32 unsigned int

typedef union
{       myInt64 int64;
        struct {INT32 lo, hi;} int32;
} tsc_counter;

#if defined(__ia64__)
	#if defined(__INTEL_COMPILER)
		#define RDTSC(tsc) (tsc).int64=__getReg(3116)
	#else
		#define RDTSC(tsc) ASM VOLATILE ("mov %0=ar.itc" : "=r" ((tsc).int64) )
	#endif

	#define CPUID() do{/*No need for serialization on Itanium*/}while(0)
#else
	#define RDTSC(cpu_c) \
		ASM VOLATILE ("rdtsc" : "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))
	#define CPUID() \
		ASM VOLATILE ("cpuid" : : "a" (0) : "bx", "cx", "dx" )
#endif

	#define rdtsc_works() {\
		tsc_counter t0,t1;\
		RDTSC(t0);\
		RDTSC(t1);\
		return COUNTER_DIFF(t1,t0,1) > 0;\
	}

/* ======================== WIN32 ======================= */
#else

	#define myInt64 signed __int64
	#define INT32 unsigned __int32

	typedef union
	{       myInt64 int64;
			struct {INT32 lo, hi;} int32;
	} tsc_counter;

	#define RDTSC(cpu_c)   \
	{       __asm rdtsc    \
			__asm mov (cpu_c).int32.lo,eax  \
			__asm mov (cpu_c).int32.hi,edx  \
	}

	#define CPUID() \
	{ \
		__asm mov eax, 0 \
		__asm cpuid \
	}

	#define rdtsc_works()\
	{\
		__try {\
			__asm rdtsc\
		} __except ( 1) {\
		return 0;\
		}\
		return 1;\
	}
#endif

/*
#define RDTSC(cpu_c) \
{	asm("rdtsc"); 	\
	asm("mov %%eax, %0" : "=m" ((cpu_c).int32.lo) ); \
	asm("mov %%edx, %0" : "=m" ((cpu_c).int32.hi) ); \
}
*/

/*	Read Time Stamp Counter
	Read PMC 
#define RDPMC0(cpu_c) \
{		     	\
        __asm xor ecx,ecx	\
	__asm rdpmc	\
	__asm mov (cpu_c).int32.lo,eax	\
	__asm mov (cpu_c).int32.hi,edx	\
}
*/

