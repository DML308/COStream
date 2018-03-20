// pointer-test
// 结构体中含复杂类型的一级指针

typedef struct
{
	int x;
	int y;
	int **p;
	int ***q;
}SPLExtTyp1;

typedef struct
{
    double z;
	SPLExtTyp1* a;
}SPLExtTyp2;

/*
typedef struct
{
    double z;
	SPLExtTyp1 b;
	SPLExtTyp2 c;
}SPLExtTyp3;
*/
extern SPLExtTyp1 *test;
extern SPLExtTyp2 *test2;
//extern SPLExtTyp3 *test3;

composite  Main()
{
	stream<int x>pSrc;
    stream<int x>pDst; 
	pSrc = Source()
	{
		work
		{
			int j;
			int result;
			float t;   
			
			SPLExtTyp1 m;                     
			SPLExtTyp2 m2;
			//SPLExtTyp3 m3;
			//result = test->myfun(1,0);  //workEstiamte.c 293行函数调用操作符类型部分出错
			//result = test->x;
			//z = test2->z;
			//m = test2->my;
			//result = (int)(floor(8+0.5));
			//pSrc[2].x = result;
					
			for(j = 0; j < 3; j++)
				pSrc[j].x = j;
			......
			
		}
		window
		{
			pSrc tumbling(3);
		}
	};
	pDst = xx(pSrc)
	{
		work
		{
			pDst[0].x = pSrc[0].x;	
		}
		window
		{
			pSrc sliding(1,1);
			pDst tumbling(1);
		}
	};
	Sink(pDst)
	{
		work
		{
				println(pDst[0].x);
		}
	};
}

