//指针减法操作测试；
// p--; p-=2;

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
			int *p = pSrc[0].a;  //
			p+=98;
			for(j = 0; j < 100; j+=2)
			{
			    //pDst[j].x = pSrc[0].a[j];
				pDst[j].x = p[0];  //获取指针当前所指对象的值
				if(j!=98)  p-=2;  //支持指针的减法操作；
			}
			pDst[j++].x = pSrc[0].b;
			pDst[j++].x = pSrc[0].c;
		
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