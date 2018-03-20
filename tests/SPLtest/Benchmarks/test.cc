#define PI 3.141592654

composite Combine(output stream<double x>Out, input stream<double x>In)
{
	param 
		int TN;

	Out = CombineDFTX(In)
	{
		double wn_r = cos(2 * PI / TN);
		double wn_i = sin(2 * PI / TN);
		work
		{
			int i;
			double w_r = 1, w_i = 0;
			double results[2 * TN];
			double y0_r, y0_i, y1_r, y1_i;
			double y1w_r, y1w_i, w_r_next, w_i_next;

			for (i = 0; i < TN; i += 2)
			{
				y0_r = In[i].x;
				y0_i = In[i+1].x;

				y1_r = In[TN+i].x;
				y1_i = In[TN+i+1].x;

				y1w_r = y1_r * w_r - y1_i * w_i;
				y1w_i = y1_r * w_i + y1_i * w_r;

				results[i] = y0_r + y1w_r;
				results[i + 1] = y0_i + y1w_i;

				results[TN + i] = y0_r - y1w_r;
				results[TN + i + 1] = y0_i - y1w_i;

				w_r_next = w_r * wn_r - w_i * wn_i;
				w_i_next = w_r * wn_i + w_i * wn_r;
				w_r = w_r_next;
				w_i = w_i_next;
			}

			for (i = 0; i < 2 * TN; i++)
			{
				Out[i].x = results[i];
			}
		}
		window{
			In  sliding(2 * TN,3);
			Out tumbling(1);
		} 
	};
}

composite FFTReorderSimple(output stream<double x>Out, input stream<double x>In)
{
	param
		int TN;

	Out = FFTReorderSimpleX(In)
	{
		int totalData = 2*TN;
		work
		{
			int i = 0, j = 0;
			for (i = 0; i < totalData; i+=4)
			{
				Out[j++].x = In[i].x;
				Out[j++].x = In[i+1].x;
			}

			for (i = 2; i < totalData; i+=4)
			{
				Out[j++].x = In[i].x;
				Out[j++].x = In[i+1].x;
			}
		}
		window {
			In  sliding(1,1);
			Out tumbling(1);
		}
	};
}

composite FFTReorder(output stream<double x>Out, input stream<double x>In)
{
	param 
		int n;
	Out = pipeline(In)
	{
		int i;
		for(i=1; i<(n/2); i*= 2)
			add FFTReorderSimple(n/i);
	};
}

composite CombineDFT(output stream<double x>Out, input stream<double x>In)
{
	param 
		int n;
	Out = pipeline(In)
	{
		int j;
		for(j=2; j<=n; j*=2)
			add Combine(j);
	};
}

composite Main()
{
	int N = 256;

	stream<double x> Source,CDFT;
	
	Source = FloatSource()
	{
		double max = 1000.0;	
		double current = 0.0;
		work
		{	
			int i;
			if (current > max)
				current = 0.0;
			for ( i=0; i<2*N; i++) 
			{
				Source[i].x = current;
				current += 1;
			}
		}
		window {
			Source tumbling(1);
		}
	};
	CDFT = pipeline(Source){
		add FFTReorder(N);
		add CombineDFT(N);
	};
	FloatSink(CDFT)
	{
		work
		{
			println(CDFT[0].x);
		}
	};
}

