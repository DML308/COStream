// pointer-test
// work中含结构体变量的声明； struct test m;  正确
// 结构体中含指向自身类型的一级指针,使用typedef 和 不使用typedef后端代码中生成均不成功。指针指向的类型struct test在ptr的相关字段中找不到。


/*
struct test
{
	int x;
	int y;
	int *p;
	struct test* ptr;  
};
*/  //不支持

/*
typedef struct test
{
	int x;
	int y;
	int *p;
	struct test* ptr;  
}SPLExtTyptest1;   //不支持
*/

//说明在结构体中暂时不支持指向自身类型的指针。但是前面我们的实验我们是支持其他类型的指针的，可以定义一个和本结构体test2中除了指针类型外的其他成员相同的结构体test1，然后在新结构体中定义一个test1的指针，但是这样只能支持两个节点，不能有struct的类似链表的功能。
//SPLExtTyptest1

typedef struct
{
  int x;
  int y;
}SPLExtTyp1;

typedef struct
{
    int x;
	int y;
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
//extern SPLExtTyp1 *test;
//extern SPLExtTyp2 *test2;
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
			struct test m;
			struct test* my1;
			//SPLExtTyp1 m;                      
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