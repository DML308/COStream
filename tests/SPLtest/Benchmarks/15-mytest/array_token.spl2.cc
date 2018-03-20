//支持token为数组时取数组首地址，如下指针p; 支持prim类型元素取地址操作，如q = &p[j];
composite  Main()
{
	stream<int a[100],int b,int c>pSrc;
    stream<int x>pDst; 
	pSrc = Source()
	{
		work
		{
			int j;
			for(j = 0; j < 100; j++)
			{
				pSrc[0].a[j] = j;	
			}
			pSrc[0].b = 1;
			pSrc[0].c = 1;
		}
		window
		{
			pSrc tumbling(1);
		}
	};
	pDst = xx(pSrc)
	{
		work
		{
		    int j;
			//int a[100];  //正确
			//int tt[100] = {0};  //正确
			//int *q = &a[0];  //正确，支持一般数组的取地址
			int *p = pSrc[0].a;  //正确，获取token中数组的首地址；
			int *q;
			//int *p = &(pSrc[0].a[0]); //错误
			for(j = 0; j < 100; j++)
			{
			    //pDst[j].x = pSrc[0].a[j];
				pDst[j].x = p[j];
				q = &p[j];
			}
			//pDst[j++].x = pSrc[0].b;
			//pDst[j++].x = pSrc[0].c;
			pDst[j++].x = q[0];
			pDst[j++].x = q[0];
		}
		window
		{
			pSrc sliding(1,1);
			pDst tumbling(102);
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