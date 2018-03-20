//#include "myMath.h"

//#define myfun(j) j*j

composite  Main()
{
	stream<int x>pSrc;
    stream<int x>pDst; 
	//int result;
	//result = myfun(2);
	pSrc = Source()
	{
		work
		{
			int j;
			int result;
			for(j = 0; j < 9; j++)
				pSrc[j].x = j;
			result = myfun(2);
			//result = (int)(floor(8+0.5));
			pSrc[9].x = result;
					
		}
		window
		{
			pSrc tumbling(10);
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