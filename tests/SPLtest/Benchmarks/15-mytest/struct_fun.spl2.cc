
// pointer-test
//主要实现如下：
// 外部变量多级指针的声明extern SPLExtTyp1 ****test; extern int ****q;
// work中复杂类型多级指针变量SPLExtTyp1*** s;  SPLExtTyp1**** r;  SPLExtTyp1***** w;  //正确
// SPLExtTyp1 中 多级指针int ***p；

typedef struct m1
{
	int x;
	int y;
}SPLExtTyp1;

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
			SPLExtTyp1 pp;
		
			float t;   
			//C中变量定义必须在代码前面定义
			for(j = 0; j < 2; j++)
				pSrc[j].x = j;
			result = myfun(test.x, test.y);
		    pSrc[2].x = result;
			//result = test->myfun(test.x,test.y);  //workEstiamte.c 293行函数调用操作符类型部分出错
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