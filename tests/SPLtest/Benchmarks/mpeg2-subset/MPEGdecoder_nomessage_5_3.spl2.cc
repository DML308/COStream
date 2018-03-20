double pi = 3.141592;
int blocks_per_macroblock[4] = {0, 6, 8, 12};
int source[403]={72,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-66,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-68,-5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-67,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-9,-111,-111,0,-101,0,28,0,0,28,0,25,0,23,0,-15,0,0,0,0,-16,0,-13,0,-7,0,-12,0,10,0,0,0,0,0,0,13,11,0,4,0,4,0,8,0,0,0,0,0,0,-2,0,-2,0,-2,0,0,0,0,1,0,1,0,0,0,0,26,26,0,24,0,-6,0,0,-6,0,-6,0,-5,0,3,0,0,0,0,3,0,3,0,1,0,3,0,-2,0,0,0,0,0,0,-2,-2,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0};

composite MotionVectorDecode(output stream<int x>Out, input stream<int x>In)
{
	
	Out = MotionVectorDecode(In)
	{
		
		int PMV[8] ;
		int mv_format; // HACKED TODO - MESSAGING
		int picture_structure; // HACKED TODO - MESSAGING
		init 
		{
			mv_format = 1; // HACKD TODO MESSAGING
			picture_structure = 1; // HACKED TODO - MESSAGING
		}
		work
		{				
			int r,s,t;
			int pop = 0;
			int push = 0;
			int motion_code[8];
			int motion_residual[8];
			int vectorp[8] ;
			for (r = 0; r < 2; r++)
				for (s = 0; s < 2; s++) 
					for (t = 0; t < 2; t++) {
						motion_code[r*4 + s*2 + t] = In[pop].x;
						pop++;
					}
	
			for (r = 0; r < 2; r++)
				for (s = 0; s < 2; s++) 
					for (t = 0; t < 2; t++) {
						motion_residual[r*4  + s*2 + t] = In[pop].x;
						pop++;
					}
	
			for (r = 0; r < 1; r++) {
				// NOTE TODO - Hacked right now, don't know when we need the second motion vector.
				for (s = 0; s < 2; s++) {
					for (t = 0; t < 2; t++) {
						int r_size = 14;
						int f = 1 << r_size;
						int high = (16*f)-1;
						int low = ((-16)*f);
						int range = (32*f);
						int delta;
						int prediction;
						if ((f == 1) || (motion_code[r*4 + s*2 + t] == 0)) {
							delta = motion_code[r*4  + s*2 + t];
						} else {
							delta = ((int) (abs(motion_code[r*4  + s*2 + t])-1)*f) + 
							motion_residual[r*4 + s*2 + t]+1;
							if (motion_code[r*4 + s*2 + t]<0)
							delta = -delta;
						}
						prediction = PMV[r*4 + s*2 + t];
						if ((mv_format == 0) && (t == 1) && (picture_structure == 3))
							println("Error - Program Limitation: May not be correct in decoding motion vectors");
						vectorp[r*4 + s*2 + t] = prediction + delta;
						if (vectorp[r*4 + s*2 + t] < low)
							vectorp[r*4 + s*2 + t] = vectorp[r*4 + s*2 + t] + range;
						if (vectorp[r*4 + s*2 + t] > high)
							vectorp[r*4 + s*2 + t] = vectorp[r*4 + s*2 + t] - range;
						if ((mv_format == 0) && (t == 1) && (picture_structure == 3))
							println("Error - Program Limitation: May not be correct in decoding motion vectors");
						else 
							PMV[r*4 + s*2 + t] = vectorp[r*4 + s*2 + t];
						// TODO handle updating missed motion_vectors
						// section 7.6.3.3 
					}
				}
			} 
			for (r = 0; r < 2; r++)
				for (s = 0; s < 2; s++) 
					for (t = 0; t < 2; t++) {
						Out[push].x = vectorp[r*4 + s*2 + t];
						push++;
					}			
		}
		window {
			In	sliding(16,16);
			Out	tumbling(8);
		}
	};
}

composite Repeat(output stream<int x>Out, input stream<int x>In)
{
	param
		int numitems, int numtimes;
	
	Out = Repeat(In)
	{
		work
		{
			int dataArray[numitems];
			int i,j,push = 0;
			for (i = 0; i < numitems; i++) {
				dataArray[i] = In[i].x;
			}
			for (j = 0; j < numtimes; j++) {
				for (i = 0; i < numitems; i++) {
					Out[push++].x = dataArray[i];// lxx
				}
			}			
		}
		window {
			In	sliding(numitems, numitems);
			Out	tumbling(numitems * numtimes);
		}
	
	};

}

composite PipelineMVDR(output stream<int x>Out, input stream<int x>In)
{
	param 
		int numitems, int numtimes;
	
	Out = pipeline(In)
	{
		add MotionVectorDecode();
		add Repeat(numitems, numtimes);		
	};
}

composite ZigZagUnordering(output stream<int x>Out, input stream<int x>In)
{
	
	Out = ZigZagUnordering(In)
	{
		 
		int Ordering[64] = {0, 1, 5, 6, 14, 15, 27, 28,
				2, 4, 7, 13, 16, 26, 29, 42,
				3, 8, 12, 17, 25, 30, 41, 43,
				9, 11, 18, 24, 31, 40, 44, 53,
				10, 19, 23, 32, 39, 45, 52, 54,
				20, 22, 33, 38, 46, 51, 55, 60,
				21, 34, 37, 47, 50, 56, 59, 61,
				35, 36, 48, 49, 57, 58, 62, 63};
		work
		{
			int peekSubstitute[64];
			int i;
			for (i = 0; i < 64; i++) {
				peekSubstitute[i] = In[i].x;
			}
			for (i = 0; i < 64; i++) {
				Out[i].x = peekSubstitute[Ordering[i]];
			}		
		}
		window {
			In	sliding(64,64);
			Out	tumbling(64);
		}
	};
}

composite InverseQuantization_DC_Intra_Coeff(output stream<int x>Out, input stream<int x>In)
{
	
	Out = InverseQuantization_DC_Intra_Coeff(In)
	{
		
		int intra_dc_mult[4] ;
		int intra_dc_precision;
		init 
		{
			intra_dc_mult[0] = 8;
			intra_dc_mult[1] = 4;
			intra_dc_mult[2] = 2;
			intra_dc_mult[3] = 1;
			intra_dc_precision = 0; 
		}
		work
		{
			Out[0].x = intra_dc_mult[intra_dc_precision] * In[0].x;			
		}
		window  {
			In	sliding(1, 1);
			Out	tumbling(1);
		}
	};
}

composite InverseQuantization_AC_Coeff(output stream<int x>Out, input stream<int x>In)
{
	param 
		int macroblock_intra;
	
	Out = InverseQuantization_AC_Coeff(In)
	{
		
		int quantiser_scale_code;
		int q_scale_type;
		int intra_quantiser_matrix[64]  = { 8, 16, 19, 22, 26, 27, 29, 34,
							   16, 16, 22, 24, 27, 29, 34, 37,
							   19, 22, 26, 27, 29, 34, 34, 38,
							   22, 22, 26, 27, 29, 34, 37, 40,
							   22, 26, 27, 29, 32, 35, 40, 48,
							   26, 27, 29, 32, 35, 40, 48, 58,
							   26, 27, 29, 34, 38, 46, 56, 69,
							   27, 29, 35, 38, 46, 56, 69, 83};
		int non_intra_quantiser_matrix[64]  = {16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16,
								  16, 16, 16, 16, 16, 16, 16, 16};

		// (cite 1, P.70 Table 7-6)
		int quantiser_scale[2][32]  =
	// Note that quantiser_scale[x][0] is a Forbidden Value
								{{ 0,  2,  4,  6,  8, 10, 12, 14,
								16, 18, 20, 22, 24, 26, 28, 30,
								32, 34, 36, 38, 40, 42, 44, 46,
								48, 50, 52, 54, 56, 58, 60, 62},
								{ 0,  1,  2,  3,  4,  5,  6,  7,
								8, 10, 12, 14, 16, 18, 20, 22,
								24, 28, 32, 36, 40, 44, 48, 52, 
								56, 64, 72, 80, 88, 96, 104, 112}};
		init 
		{
			quantiser_scale_code = 1; // Guarantees that this throws an error
			// if it doesn't get a quantiser message
			// before getting some data.
			q_scale_type = 0; // Another nice error if no message received in time.
		}
		work
		{
			int i, push = 0, pop = 0;
			if (quantiser_scale_code == 0)
				println("Error - quantiser_scale_code not allowed to be 0 " + macroblock_intra);
			
			for (i = macroblock_intra; i < 64; i++) 
			{
				
				int W = 0, F;
				int QF = In[pop++].x; //lxx
				// (cite 1, P.71)
				int k = 0;
				if (macroblock_intra == 1) {
				k = 0;
				} 
				else 
				{
					// TODO - I think I'm interpreting this part of the spec correctly, check though.
					if (QF > 0) {
						k = 1;
					} else if (QF < 0) {
						k = -1;
					} else {
						k = 0;
					}          
				}
			
				if (macroblock_intra == 1) {
					W = intra_quantiser_matrix[i];
				} else {
					W = non_intra_quantiser_matrix[i];
				}
				F = (2 * QF + k) * W * 
					quantiser_scale[q_scale_type][quantiser_scale_code] / 32;
				Out[push++].x = F; // lxx
			}
		}
		window {
			In	sliding(64-macroblock_intra, 64-macroblock_intra);
			Out	tumbling(64-macroblock_intra);
		}
	};
}

composite InverseQuantizationJoinerSubstitute(output stream<int x>Out, input stream<int x>In)
{
	
	Out = InverseQuantizationJoinerSubstitute(In)
	{
		
		int macroblock_intra;
		init 
		{
			macroblock_intra = 1;
		}
		work
		{
			int i;
			if (macroblock_intra == -1) 
			{
				println("  Error: macroblock_intra should not be -1, should have recieved update message");
			} 
			else if (macroblock_intra == 1) 
			{
				// It was Intra Coded
				for (i = 0; i < 64; i++) 
				{
					Out[i].x = In[i].x;//push(pop());
				}
			} 
			else 
			{
				// It was Non Intra Coded
				for (i = 0; i < 64; i++) 
				{
					Out[i].x = In[i+64].x;//push(pop());
				}
			}
		
		}
		window  {
			In	sliding(128, 128);
			Out	tumbling(64);
		}
	
	};
}

composite SplitjoinIQ_DCIC_AC_Coeff(output stream<int x>Out, input stream<int x>In)
{
	param 
		int n;
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(1, 63);
		for(i = 0; i<2;i++)
			if(0 == i) 
				add InverseQuantization_DC_Intra_Coeff();
			else 
				add InverseQuantization_AC_Coeff(1); 
		join roundrobin(1, 63);		
	};
}

composite InverseQuantization(output stream<int x>Out, input stream<int x>In)
{
	
	stream<int x>OutSplit;
	int i;
	OutSplit = splitjoin(In)
	{
		split duplicate();
		for(i=0;i<2;i++)
			if(0 == i) 
				add SplitjoinIQ_DCIC_AC_Coeff(1);
			else 
				add InverseQuantization_AC_Coeff(0);			
		join roundrobin(64, 64); 	
	};
	Out = InverseQuantizationJoinerSubstitute(OutSplit)();
}

composite BoundedSaturation(output stream<int x>Out, input stream<int x>In)
{
	param 
		int min, int max, int worst_input_min, int worst_input_max;
	
	Out = BoundedSaturation(In)
	{		
		
		int range;
		int n = 2*(worst_input_max - worst_input_min + 1);
		int saturate[n] ; // TODO - fix static variables propogating in library
		init 
		{
			int i;
			range = worst_input_max - worst_input_min + 1; // Should come earlier but range not set correctly
			for (i = 0; i < range*2; i++) 
			{
				int valx = i + worst_input_min;
				if (valx < min) {
					saturate[i] = min;
				} else if (valx > max) {
					saturate[i] = max;
				} else {
					saturate[i] = valx;
				}
			}   
		}
		work
		{
			Out[0].x = saturate[In[0].x-worst_input_min];
		}		
		window {
			In	sliding(1,1);
			Out	tumbling(1);
		}
	};
}

composite Saturation(output stream<int x>Out, input stream<int x>In)
{
	param 
		int min, int max;
	
	Out = Saturation(In)
	{
		work
		{
			int valx = In[0].x;
			if (valx > max) {
				Out[0].x = max; 
			} else if (valx < min) {
				Out[0].x = min;
			} else {
				Out[0].x = valx;
			}
		
		}	
	};
}

composite MismatchControl(output stream<int x>Out, input stream<int x>In)
{
	
	Out = MismatchControl(In)
	{
		work
		{	
			int sum, valx,i;
			sum = 0;
			for (i = 0; i < 63; i++) 
			{
				valx = In[i].x;
				sum += valx;
				Out[i].x=valx;
			}
			valx = In[i].x;
			sum += valx;
			if ((sum & 0x1) == 0x1) 
			{
				Out[63].x = valx;
			}
			else 
			{
				if ((valx * 0x1) == 0x1) 
				{
					Out[63].x = valx-1;
				} 
				else 
				{
					Out[63].x = valx+1;
				}
			}
						
		}
		window {
			In	sliding(64, 64);
			Out	tumbling(64);		
		}
	
	};

}

composite BestSaturation(output stream<int x>Out, input stream<int x>In)
{
	param
		int min, int max, int worst_input_min, int worst_input_max;
		// we know:
		//  - range of <= 521  does better with bounded saturation
		//  - range of >= 1024 does better with plain saturation
	int range = worst_input_max - worst_input_min + 1;
	int i;
	Out = pipeline(In)
	{
		for(i=0;i<1;i++)
			if(range<600) 
				add BoundedSaturation(min, max, worst_input_min, worst_input_max);
			else 
				add Saturation(min, max);	
	};
}

composite iDCT_2D_reference_coarse(output stream<int x>Out, input stream<int x>In)
{
	param
		int size;
	
	Out = iDCT_2D_reference_coarse(In)
	{	
		double coeff[size][size] ;
		init
		{
			int time,freq;
			double scale;
			for (freq = 0; freq < size; freq++) 
			{
				scale = (freq == 0) ? sqrt(0.125) : 0.5;
				for (time = 0; time < size; time++)
					coeff[freq][time] = scale * cos((pi/(double)size) * freq * (time + 0.5));
			}			
		}
		work
		{
			double block_x[size][size] ;
			int i, j, k;
			int push=0;
			for (i = 0; i < size; i++)
				for (j = 0; j < size; j++) 
				{
					block_x[i][j] = 0;
					for (k = 0; k < size; k++) 
					{
						block_x[i][j] += coeff[k][j] * In[size*i + k].x ;
					}
				}

			for (i = 0; i < size; i++) {
				for (j = 0; j < size; j++) {
					double block_y = 0.0;
					for (k = 0; k < size; k++) {
						block_y += coeff[k][i] * block_x[k][j];
					}
					block_y = floor(block_y + 0.5);
					Out[push++].x = (int) block_y;
				}
			}
		}
		window {
			In	sliding(size*size, size*size);
			Out	tumbling(size*size);
		}
	};
}

composite iDCT_1D_reference_fine(output stream<double x>Out, input stream<double x>In)
{
	param 
		int size;
	
	Out = iDCT_1D_reference_fine(In)
	{
		double coeff[size][size]; 
		init
		{
			int x,u;
			for (x = 0; x < size; x++) {
				for (u = 0; u < size; u++) {
					double Cu = 1;
					if (u == 0) Cu = 1/sqrt(2);
						coeff[x][u] = 0.5 * Cu * cos(u * pi * (2.0 * x+1) / (2.0 * size));
				}
			}
		
		}
		work
		{
			int x,u;
			for (x = 0; x < size; x++) {
				double tempsum = 0;
				for (u = 0; u < size; u++) {
					tempsum += coeff[x][u] * In[u].x;
				}
				Out[x].x = (int)tempsum;
			}
		
		}
		window {
			In	sliding(size,size);
			Out	tumbling(size);
		}
	
	};


}

composite iDCT_1D_Y_reference_fine(output stream<double x>Out, input stream<double x>In)
{
	param 
		int size;
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(1);
		for (i = 0; i < size; i++) 
		{
			add iDCT_1D_reference_fine(size); 
		}
		 join roundrobin(1);		
	};
}

composite pipeline_iDCT_1D_reference_fine_AnonyFilter(output stream<int x>Out, input stream<double x>In)
{
	param
		int size;
	stream<double x>Out_fine;
	Out_fine = iDCT_1D_reference_fine(In)(size);
	Out = Anony_floor(Out_fine)
	{
		work
		{
			Out[0].x = Out_fine[0].x;
		}		
	};
}

composite iDCT_1D_X_reference_fine(output stream<int x>Out, input stream<double x>In)
{
	param
		int size;
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(size);
		for(i = 0; i < size; i++)
		{
			add pipeline_iDCT_1D_reference_fine_AnonyFilter(size);			
		}
		join roundrobin(size);		
	};
}

composite iDCT_2D_reference_fine(output stream<int x>Out, input stream<int x>In)
{
	param
		int size;
	stream<double x> OutT,iDCT_1D_Y_Out;
	OutT = Anonoy_fine(In)
	{
		work
		{
			Out[0].x = (double)(In[0].x);			
		}		
	};
	iDCT_1D_Y_Out = iDCT_1D_Y_reference_fine(OutT)(size);
	Out = iDCT_1D_X_reference_fine(iDCT_1D_Y_Out)(size);
}

composite iDCT8x8_1D_row_fast (output stream<int x>Out, input stream<int x>In)
{
	
	Out = iDCT8x8_1D_row_fast(In)
	{
		int size = 8;

		int W1 = 2841; /* 2048*sqrt(2)*cos(1*pi/16) */
		int W2 = 2676; /* 2048*sqrt(2)*cos(2*pi/16) */
		int W3 = 2408; /* 2048*sqrt(2)*cos(3*pi/16) */
		int W5 = 1609; /* 2048*sqrt(2)*cos(5*pi/16) */
		int W6 = 1108; /* 2048*sqrt(2)*cos(6*pi/16) */
		int W7 = 565;  /* 2048*sqrt(2)*cos(7*pi/16) */
		work
		{
			int x0 = In[0].x;
			int x1 = In[4].x << 11;
			int x2 = In[6].x;
			int x3 = In[2].x;
			int x4 = In[1].x;
			int x5 = In[7].x;
			int x6 = In[5].x;
			int x7 = In[3].x;
			int x8;
			int i;

				/* shortcut */
			if ((x1 == 0) && (x2 == 0) && (x3 == 0) &&(x4 == 0) && (x5 == 0) && (x6 == 0) && (x7 == 0))
			{
					x0 = x0 << 3;
					for (i = 0; i < size; i++) {
						Out[i].x=x0;
					}
			}
			else {
				/* for proper rounding in the fourth stage */
				x0 = (x0 << 11) + 128; 

				/* first stage */
				x8 = W7 * (x4 + x5);
				x4 = x8 + (W1 - W7) * x4;
				x5 = x8 - (W1 + W7) * x5;
				x8 = W3 * (x6 + x7);
				x6 = x8 - (W3 - W5) * x6;
				x7 = x8 - (W3 + W5) * x7;

				/* second stage */
				x8 = x0 + x1;
				x0 = x0 - x1;
				x1 = W6 * (x3 + x2);
				x2 = x1 - (W2 + W6) * x2;
				x3 = x1 + (W2 - W6) * x3;
				x1 = x4 + x6;
				x4 = x4 - x6;
				x6 = x5 + x7;
				x5 = x5 - x7;

				/* third stage */
				x7 = x8 + x3;
				x8 = x8 - x3;
				x3 = x0 + x2;
				x0 = x0 - x2;
				x2 = (181 * (x4 + x5) + 128) >> 8;
				x4 = (181 * (x4 - x5) + 128) >> 8;

					/* fourth stage */
				Out[0].x = (x7 + x1) >> 8;
				Out[1].x = (x3 + x2) >> 8;
				Out[2].x = (x0 + x4) >> 8;
				Out[3].x = (x8 + x6) >> 8;
				Out[4].x = (x8 - x6) >> 8;
				Out[5].x = (x0 - x4) >> 8;
				Out[6].x = (x3 - x2) >> 8;
				Out[7].x = (x7 - x1) >> 8;
			}	
		}
		window {
			In	sliding(8,8);
			Out	tumbling(8);
		}
	};
}

composite iDCT8x8_1D_col_fast(output stream<int x>Out, input stream<int x>In)
{
	
	int size = 8;
	
	Out = iDCT8x8_1D_col_fast(In)
	{
			
		int n = 8*8;
		int buffer[n] ;

		int W1 = 2841; /* 2048*sqrt(2)*cos(1*pi/16) */
		int W2 = 2676; /* 2048*sqrt(2)*cos(2*pi/16) */
		int W3 = 2408; /* 2048*sqrt(2)*cos(3*pi/16) */
		int W5 = 1609; /* 2048*sqrt(2)*cos(5*pi/16) */
		int W6 = 1108; /* 2048*sqrt(2)*cos(6*pi/16) */
		int W7 = 565;  /* 2048*sqrt(2)*cos(7*pi/16) */
		int c;
		work
		{
			int i;
			for (c = 0; c < size; c++) {
				int x0 = In[c + size * 0].x;
				int x1 = In[c + size * 4].x << 8;
				int x2 = In[c + size * 6].x;
				int x3 = In[c + size * 2].x;
				int x4 = In[c + size * 1].x;
				int x5 = In[c + size * 7].x;
				int x6 = In[c + size * 5].x;
				int x7 = In[c + size * 3].x;
				int x8;
				

				/* shortcut */
				if ((x1 == 0) && (x2 == 0) && (x3 == 0) && 
					(x4 == 0) && (x5 == 0) && (x6 == 0) && (x7 == 0)) {
						x0 = (x0 + 32) >> 6;
						for (i = 0; i < size; i++) {
						buffer[c + size * i] = x0;
						}
				}
				else {
					/* for proper rounding in the fourth stage */
					x0 = (x0 << 8) + 8192; 
			
					/* first stage */
					x8 = W7 * (x4 + x5) + 4;
					x4 = (x8 + (W1 - W7) * x4) >> 3;
					x5 = (x8 - (W1 + W7) * x5) >> 3;
					x8 = W3 * (x6 + x7) + 4;
					x6 = (x8 - (W3 - W5) * x6) >> 3;
					x7 = (x8 - (W3 + W5) * x7) >> 3;
			
					/* second stage */
					x8 = x0 + x1;
					x0 = x0 - x1;
					x1 = W6 * (x3 + x2) + 4;
					x2 = (x1 - (W2 + W6) * x2) >> 3;
					x3 = (x1 + (W2 - W6) * x3) >> 3;
					x1 = x4 + x6;
					x4 = x4 - x6;
					x6 = x5 + x7;
					x5 = x5 - x7;
			
					/* third stage */
					x7 = x8 + x3;
					x8 = x8 - x3;
					x3 = x0 + x2;
					x0 = x0 - x2;
					x2 = (181 * (x4 + x5) + 128) >> 8;
					x4 = (181 * (x4 - x5) + 128) >> 8;
			
					/* fourth stage */
					buffer[c + size * 0] = ((x7 + x1) >> 14);
					buffer[c + size * 1] = ((x3 + x2) >> 14);
					buffer[c + size * 2] = ((x0 + x4) >> 14);
					buffer[c + size * 3] = ((x8 + x6) >> 14);
					buffer[c + size * 4] = ((x8 - x6) >> 14);
					buffer[c + size * 5] = ((x0 - x4) >> 14);
					buffer[c + size * 6] = ((x3 - x2) >> 14);
					buffer[c + size * 7] = ((x7 - x1) >> 14);
				}
			}
			for (i = 0; i < size*size; i++) {
				Out[i].x = buffer[i];
			}
		
		}
		window {
			In	sliding(size*size,size*size);
			Out	tumbling(size*size);
		}
			
	};

}

composite iDCT8x8_2D_fast_coarse(output stream<int x>Out, input stream<int x>In)
{
	Out = pipeline(In)
	{
		add iDCT8x8_1D_row_fast();
		add iDCT8x8_1D_col_fast();		
	};

}

composite iDCT8x8_1D_X_fast_fine(output stream<int x>Out, input stream<int x>In)
{
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(8);
		for(i = 0;i<8;i++)
			add iDCT8x8_1D_row_fast();
		join roundrobin(8);		
	};
}

composite iDCT8x8_1D_col_fast_fine(output stream<int x>Out, input stream<int x>In)
{
	int size = 8;
	Out = iDCT8x8_1D_col_fast_fine(In)
	{
			
		int W1 = 2841; /* 2048*sqrt(2)*cos(1*pi/16) */
		int W2 = 2676; /* 2048*sqrt(2)*cos(2*pi/16) */
		int W3 = 2408; /* 2048*sqrt(2)*cos(3*pi/16) */
		int W5 = 1609; /* 2048*sqrt(2)*cos(5*pi/16) */
		int W6 = 1108; /* 2048*sqrt(2)*cos(6*pi/16) */
		int W7 = 565;  /* 2048*sqrt(2)*cos(7*pi/16) */
		work
		{
			int x0 = In[0].x;
			int x1 = In[4].x << 8;
			int x2 = In[6].x;
			int x3 = In[2].x;
			int x4 = In[1].x;
			int x5 = In[7].x;
			int x6 = In[5].x;
			int x7 = In[3].x;
			int x8;
			int i;
			/* shortcut */
			if ((x1 == 0) && (x2 == 0) && (x3 == 0) && 
				(x4 == 0) && (x5 == 0) && (x6 == 0) && (x7 == 0)) {
					x0 = (x0 + 32) >> 6;
					for (i = 0; i < size; i++) {
						Out[i].x = x0;
					}
			}
			else {
				/* for proper rounding in the fourth stage */
				x0 = (x0 << 8) + 8192; 

				/* first stage */
				x8 = W7 * (x4 + x5) + 4;
				x4 = (x8 + (W1 - W7) * x4) >> 3;
				x5 = (x8 - (W1 + W7) * x5) >> 3;
				x8 = W3 * (x6 + x7) + 4;
				x6 = (x8 - (W3 - W5) * x6) >> 3;
				x7 = (x8 - (W3 + W5) * x7) >> 3;

				/* second stage */
				x8 = x0 + x1;
				x0 = x0 - x1;
				x1 = W6 * (x3 + x2) + 4;
				x2 = (x1 - (W2 + W6) * x2) >> 3;
				x3 = (x1 + (W2 - W6) * x3) >> 3;
				x1 = x4 + x6;
				x4 = x4 - x6;
				x6 = x5 + x7;
				x5 = x5 - x7;

				/* third stage */
				x7 = x8 + x3;
				x8 = x8 - x3;
				x3 = x0 + x2;
				x0 = x0 - x2;
				x2 = (181 * (x4 + x5) + 128) >> 8;
				x4 = (181 * (x4 - x5) + 128) >> 8;

				/* fourth stage */
				Out[0].x = (x7 + x1) >> 14;
				Out[1].x = (x3 + x2) >> 14;
				Out[2].x = (x0 + x4) >> 14;
				Out[3].x = (x8 + x6) >> 14;
				Out[4].x = (x8 - x6) >> 14;
				Out[5].x = (x0 - x4) >> 14;
				Out[6].x = (x3 - x2) >> 14;
				Out[7].x = (x7 - x1) >> 14;
			}
		
		}
		window {
			In	sliding(size, size);
			Out	tumbling(size);
		}
		
	};

}

composite iDCT8x8_1D_Y_fast_fine(output stream<int x>Out, input stream<int x>In)
{
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(1);
		for(i = 0;i<8;i++)
			add iDCT8x8_1D_col_fast_fine();
		join roundrobin(1);		
	};
}

composite iDCT8x8_2D_fast_fine(output stream<int x>Out, input stream<int x>In)
{
	stream<int x> Out_Xfast;
	Out_Xfast = iDCT8x8_1D_X_fast_fine(In)();
	Out = iDCT8x8_1D_Y_fast_fine(Out_Xfast)();
}

composite iDCT8x8_ieee(output stream<int x>Out, input stream<int x>In)
{
	param
		int mode;
	int i;
	Out = pipeline(In)
	{
		for(i=0;i<1;i++)
			if(mode == 0) 
				add iDCT_2D_reference_coarse(8);
			else if(mode == 1) 
				add iDCT_2D_reference_fine(8);
			else if(mode == 2) 
				add iDCT8x8_2D_fast_coarse();
			else   
				add iDCT8x8_2D_fast_fine();
	
	};


}

composite BlockDecode(output stream<int x>Out, input stream<int x>In)
{
	Out = pipeline(In)
	{
		add ZigZagUnordering();
		// Assumes no alternate_scan TODO
		// Output of this corresponds to QF[v][u], (cite 1, P. 67)
		add InverseQuantization();
		add BestSaturation(-2048, 2047, -2050, 2050);
		add MismatchControl();
		add iDCT8x8_ieee(3); //0 = reference coarse, 1 = reference fine, 2 = fast, fine,  
		add BestSaturation(-256, 255, -260, 260);		
	};

}

composite Main()
{
	int parse_or_process = 5;
	int width=352;
	int height=240;
	int the_chroma_format = 1; 
	int i;
	stream<int x> SourceOut,SBP;
	SourceOut = Source()
	{//ÓÐ´ý²¹³ä
		int num = 0;
		work
		{
			SourceOut[0].x = source[num];
			num++;
			num = num%403; // lxx
		}
	
	};
	SBP = splitjoin(SourceOut)
	{
		split roundrobin(64*blocks_per_macroblock[the_chroma_format], 16, 3);
		for(i=0;i<3;i++)
			if(0==i) 
				add BlockDecode();
			else if(1== i) 
				add PipelineMVDR(8, blocks_per_macroblock[the_chroma_format]);
			else if(2== i) 
				add Repeat(3, blocks_per_macroblock[the_chroma_format]);
		join roundrobin(64, 8, 3);		
	};
	Sink(SBP)
	{
		work
		{
			println(SBP[0].x);
		}
	};
} 
