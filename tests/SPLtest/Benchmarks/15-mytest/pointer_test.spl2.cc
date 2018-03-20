// pointer-test
typedef struct
{
	int x;
	int y;
	//int (*myfun)(int x,int y);
	int *p;
}SPLExtTyp1;

typedef struct
{
    double z;
	SPLExtTyp1 my;
}SPLExtTyp2;



extern SPLExtTyp1 *test;
extern SPLExtTyp2 *test2;

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
			SPLExtTyp1 m;                      //有误，后端代码直接生成为 m ,没有类型;只支持外部定义的变量，
			SPLExtTyp2 m2;
			float t;   
			//C中变量定义必须在代码前面定义
			for(j = 0; j < 3; j++)
				pSrc[j].x = j;
			//result = test->myfun(1,0);  //workEstiamte.c 293行函数调用操作符类型部分出错
			//result = test->x;
			//z = test2->z;
			//m = test2->my;
			//result = (int)(floor(8+0.5));
			//pSrc[2].x = result;
					
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