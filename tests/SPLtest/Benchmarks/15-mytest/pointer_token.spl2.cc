composite  Main()
{
	stream<int x>pSrc;
    stream<int x>pDst; 
	stream<int *x>ptr;
	pSrc = Source()
	{
		work
		{
			int j;		
			for(j = 0; j < 3; j++)
				pSrc[j].x = j;
			
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

