// pointer-test
struct s1
{
	int x;
	int y;
	int ***p;
};

typedef struct s1 SS1;

struct s2
{
	SS1 a;
	SS1 ***p;
	int b;
	
};

typedef struct s2 SS2;

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
			SS1 s;
			SS1 *ps = &s;
			unsigned long ul_ps = (unsigned long)ps;
			float t;   
			//C中变量定义必须在代码前面定义
			for(j = 0; j < 3; j++)
				pSrc[j].x = j;
			//result = test->myfun(1,0);  //workEstiamte.c 293行函数调用操作符类型部分出错
			
			{
				{
					float b = 1.0;
					b = 2.0;
				}
				{
					float b = 1.0;
					b = 2.0;
				}
				
			}
			
					
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
			//pDst[0].x = pSrc[0].x;	
			pDst[0].x = compressCU();
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