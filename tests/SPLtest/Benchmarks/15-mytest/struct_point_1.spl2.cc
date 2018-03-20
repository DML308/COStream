// pointer-test
//主要实现如下：
// 外部变量多级指针的声明extern SPLExtTyp1 ****test; extern int ****q;
// work中复杂类型多级指针变量SPLExtTyp1*** s;  SPLExtTyp1**** r;  SPLExtTyp1***** w;  //正确
// 

typedef struct m1
{
	int x;
	int y;
	int *p;
	
}SPLExtTyp1;

/*
typedef struct
{
    double z;
	SPLExtTyp1* a;
}SPLExtTyp2;
*/

/*
typedef struct
{
    double z;
	SPLExtTyp1 b;
	SPLExtTyp2 c;
}SPLExtTyp3;
*/
extern SPLExtTyp1 ****test;
//extern SPLExtTyp2 *test2;
//extern SPLExtTyp3 *test3;
extern int ****q;

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
			//SPLExtTyp1* p;    
			//SPLExtTyp1** q;  
			
			SPLExtTyp1*** s;   //正确
			SPLExtTyp1**** r;  //正确	
			SPLExtTyp1***** w;  //正确
			
			//SPLExtTyp2 m2;
			//SPLExtTyp3 m3;
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