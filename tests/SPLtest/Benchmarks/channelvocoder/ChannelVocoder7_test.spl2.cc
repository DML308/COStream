double pi = 3.141592654;

composite LowPassFilter(output stream<double x>Out, input stream<double x>In)
{
	param 
		double g, double cutoffFreq, int N;
		  
	
	Out = LowPassFilter(In)
	{
		
		double h[N];
		init
		{
			int OFFSET = N/2;
			int i, idx;
			for(i=0; i<N; i++)
			{
				idx = i + 1;
				if (idx == OFFSET) 
					h[i] = g * cutoffFreq / pi; 
				else 
					h[i] = g * sin(cutoffFreq * (idx-OFFSET)) / (pi*(idx-OFFSET));
			}
		}
		work
		{
			double sum = 0;
			int i;
			for (i=0; i<N; i++) 
			{ 
				sum += h[i]*In[i].x;
			}
			Out[0].x = sum;
		}
		
		window {
			In sliding(N,1);
		}
	};
}

composite HighPassFilter(output stream<double x>Out, input stream<double x>In)
{
	param 
		double g, double ws, int N; 
  
	
	Out = HighPassFilter(In)
	{
		
		double h[N];
		init
		{
			int OFFSET = N/2;
			double cutoffFreq = pi - ws;
			int i;
			
			for(i=0; i<N; i++)
			{
				int idx = i + 1;
				int sign = ((i%2) == 0) ? 1 : -1;
				if (idx == OFFSET) 
					h[i] = sign * g * cutoffFreq / pi; 
				else 
					h[i] = sign * g * sin(cutoffFreq * (idx-OFFSET)) / (pi*(idx-OFFSET));
			}
		}
		work
		{
			double sum = 0;
			int i;
			for ( i=0; i<N; i++) 
			{ 
				sum += h[i]*In[i].x;
			}
			Out[0].x = sum;
		}
		
		window {
			In sliding(N,1);
		}
	};
}

composite PitchDetector(output stream<double x>Out, input stream<double x>In)
{
	param 
		int winsize, int decimation;
		
	stream<double x> CC;
	CC = CenterClip(In)
	{
		
		double MIN = 0-0.75;
		double MAX =  0.75;
		work
		{
			double t = In[0].x;
			if (t<MIN)
			{
			  CC[0].x = MIN; 
			} 
			else if (t>MAX)
			{
			  CC[0].x = MAX;
			} 
			else
			{
			  CC[0].x = t;
			}
		}
	};
	
	Out = CorrPeak(CC)
	{
		double THRESHOLD = 0.07;
		work
		{
			double autocorr[winsize];
			double sum, maxpeak;
			int i, j;
			for (i=0; i<winsize; i++)
			{
				sum = 0;
				for (j=i; j<winsize; j++)
				{
					sum += CC[i].x * CC[j].x;
				}
				autocorr[i] = sum/winsize;
			}

			maxpeak = 0;
			for (i=0; i<winsize; i++)
			{
				if (autocorr[i]>maxpeak)
					maxpeak = autocorr[i];
			}

			if (maxpeak > THRESHOLD)
			{
				Out[0].x = maxpeak;
			}
			else 
			{
			  Out[0].x = 0;
			}
		}
		
		window {
			Out	tumbling(1);
			CC	sliding(winsize,decimation);	
		}
	};
}

composite BandPassFilter(output stream<double x>Out, input stream<double x>In)
{
	param 
		double gain, double ws, double wp, int numSamples;
		
	Out = pipeline(In)
	{
		add LowPassFilter(1, wp, numSamples);
		add HighPassFilter(gain, ws, numSamples);	
	};
}

composite Compressor(output stream<double x>Out, input stream<double x>In)
{
	param 
		int M;
			
	Out = Compressor(In)
	{
		work
		{
			Out[0].x = In[0].x;
		}
		
		window {
		In	sliding(M,M);	
		}
	};
}

composite FilterDecimate(output stream<double x>Out, input stream<double x>In)
{
	param 
		int i, double decimation;
			
	Out = pipeline(In)
	{
		add BandPassFilter(2, 400*i, 400*(i+1), 64); 
		add Compressor(decimation);	
	};
}

composite VocoderFilterBank(output stream<double x>Out, input stream<double x>In)
{
	param 
		int N, int decimation;
	int i;		
	Out = splitjoin(In)
	{
		split duplicate();
		for (i=0; i<N; i++)
		{
			add FilterDecimate(i, decimation);
		}
		join roundrobin();
	};
}



composite MainSplitJoin(output stream<double x>Out, input stream<double x>In)
{	
	int PITCH_WINDOW = 100; // the number of samples to base the pitch detection on
	int DECIMATION   = 50; // decimation factor
	int NUM_FILTERS  = 16; //18;
	int i;	
	
	Out = splitjoin(In)
	{
		split duplicate();
		
		for(i=0;i<2;i++)
		{
			if(0 == i)
				add PitchDetector(PITCH_WINDOW, DECIMATION);
			else
				add VocoderFilterBank(NUM_FILTERS, DECIMATION);
		}
		join roundrobin(1, 16);
	};
		
}

composite Main()
{
	stream<double x> Source,LPF,MSJ;
	Source = DataSource()
	{
		
		int index;
		int SIZE = 11;
		double x[SIZE]; 
		init
		{
			index = 0;
			x[0] = 0-0.70867825;
			x[1] = 0.9750938;
			x[2] = 0-0.009129746;
			x[3] = 0.28532153;
			x[4] = 0-0.42127264;
			x[5] = 0-0.95795095;
			x[6] = 0.68976873;
			x[7] = 0.99901736;
			x[8] = 0-0.8581795;
			x[9] = 0.9863592;
			x[10] = 0.909825;
		}
		work
		{
			Source[0].x = x[index];
			index = (index+1)%SIZE;
		}
	};
	
	LPF = LowPassFilter(Source)(1, (2*pi*5000)/8000, 64); 
			
	MSJ = MainSplitJoin(LPF)();
			
	FloatSink(MSJ)
	{
		work
		{
			println(MSJ[0].x);
		}
	};
}

