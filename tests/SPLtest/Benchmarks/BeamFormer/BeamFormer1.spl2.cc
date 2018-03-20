int numChannels           = 12; 
int numSamples            = 256; 
int numBeams              = 4; 
int numCoarseFilterTaps   = 64; 
int numFineFilterTaps     = 64; 
		 
int coarseDecimationRatio = 1;
int fineDecimationRatio   = 2;
int numSegments           = 1;
int numPostDec1           = 256;
int numPostDec2           = 128;
int mfSize                = 128;
int pulseSize             = 64;
int predecPulseSize       = 128;
int targetBeam            = 1;
int targetSample          = 64;
	 
int targetSamplePostDec   = 32;
double dOverLambda         = 0.5;
double cfarThreshold       =182.4;
//double cfarThreshold       = 0.95 * dOverLambda*numChannels* (0.5*pulseSize);
composite InputGenerate(output stream<double x>Out, input stream<double x>In)
{
	param
		int myChannel,int numberOfSamples,int tarBeam,int targetSample,double thresh;
	
	Out=InputGenerate(In)
	{
		
		int curSample;
		int holdsTarget;
		init
		{
			curSample = 0;
			if(tarBeam == myChannel)
				holdsTarget = 1;
			else 
				holdsTarget = 0;
		}
		work
		{
			double tmp  = curSample*myChannel;
			if( holdsTarget == 1 && (curSample == targetSample) )
			{
				Out[0].x = sqrt(tmp);
				Out[1].x = sqrt(tmp)+1;
			} 
			else 
			{	     
				Out[0].x = -sqrt(tmp);
				Out[1].x = -(sqrt(tmp)+1);
			}

			curSample++;
			
			if (curSample >= numberOfSamples) 
			{
				curSample = 0;
			}
		}
		window {
			Out	tumbling(2);			
		}
	};
}

composite BeamFirFilter(output stream<double x>Out, input stream<double x>In)
{
	param
		int numTaps,int inputLength,int decimationRatio;
	
	Out=BeamFirFilter(In)
	{
		
		double real_weight[numTaps];//��ȷ�������鳤��
		double imag_weight[numTaps];
		int numTapsMinusOne;
		double realBuffer[numTaps];
		double imagBuffer[numTaps];
		 
		int Count;
		int pos;

		init
		{
			int j, idx;
			numTapsMinusOne = numTaps-1;
			pos = 0;
			for (j=0; j<numTaps; j++)
			{
				idx = j + 1;
				real_weight[j] = sin(idx) / ((double)idx);
				imag_weight[j] = cos(idx) / ((double)idx);
			}
		}
		work
		{
			double real_curr = 0;
			double imag_curr = 0;
			int i;
			int modPos;

			realBuffer[numTapsMinusOne - pos] = In[0].x;
			imagBuffer[numTapsMinusOne - pos] = In[1].x;
 
			modPos = numTapsMinusOne - pos;
			for (i = 0; i < numTaps; i++) 
			{
				real_curr += realBuffer[modPos] * real_weight[i] + imagBuffer[modPos] * imag_weight[i];
				imag_curr += imagBuffer[modPos] * real_weight[i] + realBuffer[modPos] * imag_weight[i];
		 
				modPos = (modPos + 1) & numTapsMinusOne;
			}
	 
			pos = (pos + 1) & numTapsMinusOne;
			Out[0].x=real_curr;
			Out[1].x=imag_curr;
			
			Count += decimationRatio;
 
			if (Count==inputLength)
			{
				Count = 0;
				pos = 0;
				for (i=0; i<numTaps; i++)
				{
					realBuffer[i] = 0;
					imagBuffer[i] = 0;
				}
			}
		}
		window {
			Out tumbling(2);
			In sliding(2*decimationRatio,2*decimationRatio);
		}
	};
}

composite BeamForm(output stream<double x>Out,input stream<double x>In)
{
	param
		int myBeamId,int numChannels;
	
	Out=BeamForm(In)
	{
		
		double real_weight[numChannels];
		double imag_weight[numChannels];//��ȷ�������鳤��
			
		init
		{
			int idx,j;
			for (j=0; j<numChannels; j++)
			{
				idx = j + 1;
	 
				real_weight[j] = sin(idx) / ((double)(myBeamId+idx));
				imag_weight[j] = cos(idx) / ((double)(myBeamId+idx));
			}
		}
		work
		{
			double real_curr = 0;
			double imag_curr = 0;
			int i;
			for (i=0; i<numChannels; i++) 
			{
				double real_pop = In[2*i].x;
				double imag_pop = In[2*i+1].x;
				 
				real_curr += real_weight[i] * real_pop - imag_weight[i] * imag_pop;
				imag_curr += real_weight[i] * imag_pop + imag_weight[i] * real_pop;
			}
			Out[0].x=real_curr;
			Out[1].x=imag_curr;
		}
			window {
				Out	tumbling(2);
				In sliding(2*numChannels,2*numChannels);
			}
	};
	
}

composite Magnitude(output stream<double x>Out,input stream<double x>In)
{
	
	Out=Magnitude(In)
	{	
		work
		{
			double sum;
			double f1 = In[0].x;
			double f2 = In[1].x;
			sum=(double)sqrt(f1*f1 + f2*f2);
			Out[0].x= sum;
		}
		window {
			Out	tumbling(1);
			In	sliding(2,2);
		}
	};
		
}

composite Detector(output stream<double x>Out,input stream<double x>In)
{
	param
		int _myBeam,int numSamples,int targetBeam,int targetSample,double cfarThreshold;
	
	Out=Detector(In)
	{
		
		int curSample;
		int myBeam;
		int holdsTarget;
		double thresh;
		init
		{
			curSample = 0;
			if(_myBeam == targetBeam)
				holdsTarget = 1;
			else
				holdsTarget = 0;
					
			myBeam = _myBeam+1;
			thresh = 0.1;
		}
		work
		{
			double inputVal = In[0].x;
			double outputVal;
			if(holdsTarget == 1 && targetSample == curSample) 
			{
				if( !(inputVal >= thresh) ) 
				{
					outputVal = 0;
				} 
				else 
				{
					outputVal = myBeam;
				}
			} 
			else 
			{
				if( !(inputVal >= thresh) ) 
				{
					outputVal = 0;
				} 
				else 
				{
					outputVal = -myBeam;
				}
			}

			outputVal = inputVal;


			//println (outputVal);
			Out[0].x=outputVal;

			curSample++;
			
			if( curSample >= numSamples )
				curSample = 0;
		}
		window {
			Out	tumbling(1);
			In	sliding(1,1);
		}
	};
		
}

composite Main_pipeline_1(output stream<double x>Out,input stream<double x>In)
{
	param
		int i,int numSamples,int targetBeam,int targetSample,double cfarThreshold,int numCoarseFilterTaps,int coarseDecimationRatio,int numFineFilterTaps,int numPostDec1,int fineDecimationRatio;
	
	Out=pipeline(In)
	{
		add InputGenerate(i, numSamples, targetBeam, targetSample, cfarThreshold);
		add BeamFirFilter(numCoarseFilterTaps, numSamples, coarseDecimationRatio);
		add BeamFirFilter(numFineFilterTaps, numPostDec1, fineDecimationRatio);
	};
}

composite Main_pipeline_2(output stream<double x>Out,input stream<double x>In)
{
	param
		int i,int mfSize,int numChannels,int numPostDec2,double cfarThreshold,int targetBeam,int targetSamplePostDec;
	
	Out=pipeline(In)
	{
		add BeamForm(i, numChannels);
		add BeamFirFilter(mfSize, numPostDec2, 1);
		add Magnitude();
		add Detector(i, numPostDec2, targetBeam, targetSamplePostDec, cfarThreshold);
	};
}

composite Main_splitjoin_1(output stream<double x>Out,input stream<double x>In)
{
	param
		int numSamples,int targetBeam,int targetSample,double cfarThreshold,int numCoarseFilterTaps,int coarseDecimationRatio,int numFineFilterTaps,int numPostDec1,int fineDecimationRatio;
	
	Out=splitjoin(In)
	{
		int i;
		split duplicate(1);
		for(i=0; i<numChannels; i++)
			add Main_pipeline_1(i, numSamples, targetBeam, targetSample, cfarThreshold,numCoarseFilterTaps,coarseDecimationRatio,numFineFilterTaps,numPostDec1, fineDecimationRatio);
			join roundrobin(2);
	};
}

composite Main_splitjoin_2(output stream<double x>Out,input stream<double x>In)
{
	param
		int mfSize,int numChannels,int numPostDec2,double cfarThreshold,int targetBeam,int targetSamplePostDec;
		
	Out=splitjoin(In)
	{
		int i;
		split duplicate();
		for (i=0; i<numBeams; i++)
			add Main_pipeline_2(i, numChannels,mfSize, numPostDec2, targetBeam, targetSamplePostDec, cfarThreshold);
		join roundrobin(1);	
	};
}

composite Main()
{
	
	stream<double x> Source,Out_1,Out_2;
	Source= FloatSource()
	{
		work
		{
			Source[0].x = 1;
		}
	};
	Out_1=Main_splitjoin_1(Source)( numSamples, targetBeam, targetSample, cfarThreshold,numCoarseFilterTaps,coarseDecimationRatio,numFineFilterTaps,numPostDec1, fineDecimationRatio);
	Out_2=Main_splitjoin_2(Out_1)( numChannels,mfSize, numPostDec2, targetBeam, targetSamplePostDec, cfarThreshold);
	FloatSink(Out_2)
	{
			work
			{
				println(Out_2[0].x);
			}
	};
}