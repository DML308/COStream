int BITS_PER_WORD  = 32;

    // length of plain text, and cipher text
int NBITS          = 128;

    // used for key schedule (golden ration)
int PHI            = 0x7e3779b9;

    // algorithm has 32 total rounds
int MAXROUNDS      = 1;

    // used for printing descriptor in output
int PRINTINFO  = 0;
int PLAINTEXT      = 0;
int USERKEY        = 1;
int CIPHERTEXT     = 2;

    // sample user keys
int USERKEYS[5][8]  = {// 0000000000000000000000000000000000000000000000000000000000000000
						{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                          // 0000000000000000000000000000000000000000000000000000000000000000 (repeated purposefully for testing)
                        {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
                          // 92efa3ca9477794d31f4df7bce23e60a6038d2d2710373f04fd30aaecea8aa43
                        {0x92efa3ca, 0x9477794d, 0x31f4df7b, 0xce23e60a, 0x6038d2d2, 0x710373f0, 0x4fd30aae, 0xcea8aa43},
                          // d3fc99e32d09420f00a041f7e32914747731be4d4e5b5da518c2abe0a1239fa8
                        {0xd3fc99e3, 0x2d09420f, 0x00a041f7, 0xe3291474, 0x7731be4d, 0x4e5b5da5, 0x18c2abe0, 0xa1239fa8},
                          // bd14742460c6addfc71eef1328e2ddb6ba5b8798bb66c3c4d380acb055cac569
                        {0xbd147424, 0x60c6addf, 0xc71eef13, 0x28e2ddb6, 0xba5b8798, 0xbb66c3c4, 0xd380acb0, 0x55cac569}};
    
int USERKEY_LENGTH = 256;

    // initial permutation
int IP[128]  = { 0, 32, 64, 96,   1, 33, 65, 97,   2, 34, 66, 98,   3, 35, 67, 99,
                4, 36, 68, 100,  5, 37, 69, 101,  6, 38, 70, 102,  7, 39, 71, 103,
                8, 40, 72, 104,  9, 41, 73, 105, 10, 42, 74, 106, 11, 43, 75, 107,
                12, 44, 76, 108, 13, 45, 77, 109, 14, 46, 78, 110, 15, 47, 79, 111,
                16, 48, 80, 112, 17, 49, 81, 113, 18, 50, 82, 114, 19, 51, 83, 115,
                20, 52, 84, 116, 21, 53, 85, 117, 22, 54, 86, 118, 23, 55, 87, 119,
                24, 56, 88, 120, 25, 57, 89, 121, 26, 58, 90, 122, 27, 59, 91, 123,
               28, 60, 92, 124, 29, 61, 93, 125, 30, 62, 94, 126, 31, 63, 95, 127};

    // final permutation
int FP[128]  = { 0,  4,  8, 12, 16, 20, 24, 28, 32,  36,  40,  44,  48,  52,  56,  60,
                64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
                1,  5,  9, 13, 17, 21, 25, 29, 33,  37,  41,  45,  49,  53,  57,  61,
                65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125,
                2,  6, 10, 14, 18, 22, 26, 30, 34,  38,  42,  46,  50,  54,  58,  62,
                66, 70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 110, 114, 118, 122, 126,
                3,  7, 11, 15, 19, 23, 27, 31, 35,  39,  43,  47,  51,  55,  59,  63,
                67, 71, 75, 79, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127};

    // substitution boxes
int SBOXES[8][16]  = {{ 3,  8, 15,  1, 10,  6,  5, 11, 14, 13,  4,  2,  7,  0,  9, 12 }, /* S0: */
                     {15, 12,  2,  7,  9,  0,  5, 10,  1, 11, 14,  8,  6, 13,  3,  4 }, /* S1: */
                     { 8,  6,  7,  9,  3, 12, 10, 15, 13,  1, 14,  4,  0, 11,  5,  2 }, /* S2: */
                     { 0, 15, 11,  8, 12,  9,  6,  3, 13,  1,  2,  4, 10,  7,  5, 14 }, /* S3: */
                     { 1, 15,  8,  3, 12,  0, 11,  6,  2,  5,  4, 10,  9, 14,  7, 13 }, /* S4: */
                     {15,  5,  2, 11,  4, 10,  9, 12,  0,  3, 14,  8, 13,  6,  7,  1 }, /* S5: */
                     { 7,  2, 12,  5,  8,  4,  6, 11, 14,  9,  1, 15, 13,  3, 10,  0 }, /* S6: */
                     { 1, 13, 15,  0, 14,  8,  2, 11,  7,  4, 12, 10,  9,  3,  5,  6 }};/* S7: */
composite IntoBits(output stream<int x>Out,input stream<int x>In)
{
	Out = IntoBits(In)
	{
		work
		{
			int v = Out[0].x;
			int m = 1;
			int i;

			for (i = 0; i < 32; i++)
			{
				if (((v & m) >> i) != 0)
					Out[i].x = 1;
				else
					Out[i].x = 0;
				m = m << 1;
			}
		}
		window {
			Out	tumbling(32);
		}
	
	};
}


composite Permute(output stream<int x>Out,input stream<int x>In)
{
	param
		int N, int permutation[N];
	Out = Permute(In)
	{
		work
		{
			int i;
			for (i = 0; i < N; i++)
			{
				Out[i].x = In[permutation[i]].x;
			}			
		}
		window {
			Out  tumbling(N);
			In sliding(N,N);
		}
	};
}

composite Identity(output stream<int x>Out, input stream<int x>In)
{
	Out = Identity(In)
	{ 
		work
		{
			Out[0].x = In[0].x;
		}
	};
}

composite BitSlice(output stream<int x>Out, input stream<int x>In)
{
	param
		int w, int b;
	int i;
	Out = splitjoin(In)
	{
		split roundrobin(1);
		for(i = 0 ; i < b; i++)
			add Identity();
		join roundrobin(w);	
	};
		

}

composite Sbox(output stream<int x>Out,input stream<int x>In)
{
	param
		int round;
	Out = Sbox(In)
	{
		work
		{
			int val = In[0].x;
			int out;
			val = (In[1].x << 1)|val;
			val = (In[2].x << 2)|val;
			val = (In[3].x << 3)|val;
			out = SBOXES[round][val];
			Out[0].x = ((int)((out & 1) >> 0));
			Out[1].x = ((int)((out & 2) >> 1));
			Out[2].x = ((int)((out & 4) >> 2));
			Out[3].x = ((int)((out & 8) >> 3));
		}
		window {
			Out	tumbling(4);
			In	sliding(4,4);
		}
	};
}

composite slowKeySchedule(output stream<int x>Out,input stream<int x>In)
{
	param 
		int vector, int round;
	stream<int x>OutA,OutITB,OutBS,OutSbox,OutBS2;
	OutA = Anonfilter(In)
	{
		int w[140];
		init
		{
			int key[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // initialize to 0
			int i,j;
			int words = USERKEY_LENGTH / BITS_PER_WORD;
			int wx[32];
			for (i = words - 1; i >= 0; i--)
				key[words - 1 - i] = USERKEYS[vector][i];
		
			// add 1 to MSB of user key
			if (USERKEY_LENGTH < 256) {
				int msb = key[USERKEY_LENGTH / BITS_PER_WORD];
				key[USERKEY_LENGTH / BITS_PER_WORD] = msb | 1 << (USERKEY_LENGTH % BITS_PER_WORD);
			}
		
			// make prekeys w_-8 ... w_-1
			for (i = 0; i < 8; i++)
				w[i] = key[i];
		
			// calculate intermediate keys w_0 ... w_131
			for (i = 8; i < 140; i++) {
				int v[32];
				int x,m,r;
				w[i] = w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ PHI ^ (i - 8);
				//w[i] = LRotate(w[i], 11);
				x = w[i];
				m = 1;
				for ( j = 0; j < 32; j++)
				{
					if (((x & m) >> j) != 0)
						v[j] = 1;
					m = m << 1;
				}

					
				for ( j = 0; j < 32; j++)
				{
					wx[j] = v[(j + 32 - 11) % 32];
				}

				r = 0;
				for ( j = 0; j < 32; j++) 
				{
					r = r | (wx[j] << j);
				}
					
				w[i] = r;
			}
		
		}
		work
		{
			int i = (4 * round) + 8;
			OutA[0].x = (w[i + 0]);
			OutA[1].x = (w[i + 1]);
			OutA[2].x = (w[i + 2]);
			OutA[3].x = (w[i + 3]);			
		}
		window {
			OutA  tumbling(4);
			In  sliding(0,0);
		}
				
	};
	OutITB = IntoBits(OutA)();
	OutBS = BitSlice(OutITB)(4, 32);
	OutSbox = Sbox(OutBS)((32 + 3 - round) % 8);

	// reverse the bit slicing
	OutBS2 = BitSlice(OutSbox)(32, 4);

	Out = Permute(OutBS2)(NBITS, IP);		

}

composite KeySchedule(output stream<int x>Out,input stream<int x>In)
{
	param
		int vector, int round;
	Out = KeySchedule(In)
	{
		int keys[9][NBITS];
		init
		{
			int userkey[8]= {0, 0, 0, 0, 0, 0, 0, 0}; // initialize to 0
			int w[140];
			int i,b,k;
			int key[NBITS];
			int words = USERKEY_LENGTH / BITS_PER_WORD;
			for (i = words - 1; i >= 0; i--)
				userkey[words - 1 - i] = USERKEYS[vector][i];
			
			// add 1 to MSB of user key
			if (USERKEY_LENGTH < 256) {
				int msb = userkey[USERKEY_LENGTH / BITS_PER_WORD];
				userkey[USERKEY_LENGTH / BITS_PER_WORD] = msb | 1 << (USERKEY_LENGTH % BITS_PER_WORD);
			}
		
			// make prekeys w_-8 ... w_-1
			for (i = 0; i < 8; i++)
				w[i] = userkey[i];
		
			// calculate intermediate keys w_0 ... w_131
			for (i = 8; i < 140; i++) {
				int j, m=1, y, n, rx, wx[32], v[32];
					
				w[i] = w[i - 8] ^ w[i - 5] ^ w[i - 3] ^ w[i - 1] ^ PHI ^ (i - 8);
					//w[i] = LRotate(w[i], 11);
					
				y = w[i]; n = 11;
				for ( j = 0; j < 32; j++) 
				{
					if (((y & m) >> j) != 0)
						v[j] = 1;
					m = m << 1;
				}

					
				for ( j = 0; j < 32; j++) 
				{
					wx[j] = v[(j + 32 - 11) % 32];
				}

				rx = 0;
				for ( j = 0; j < 32; j++)
				{
					rx = rx | (wx[j] << j);
				}
					
				w[i] = rx;
			}
			for (i = 0; i <= MAXROUNDS; i++)
			{
				int sbox[BITS_PER_WORD] ;
				int b;
				for (b = 0; b < BITS_PER_WORD; b++)
				{
				// int to bits in slices
					int r  = (4 * i) + 8;
					int b0 = (w[r + 0] & (1 << b)) >> b;
					int b1 = (w[r + 1] & (1 << b)) >> b;
					int b2 = (w[r + 2] & (1 << b)) >> b;
					int b3 = (w[r + 3] & (1 << b)) >> b;

					int val = 0;
					if (b0 != 0) val = 1;
					if (b1 != 0) val = val | (1 << 1);
					if (b2 != 0) val = val | (1 << 2);
					if (b3 != 0) val = val | (1 << 3);
				
					// round  0: use sbox 3
					// round  1: use sbox 2
					// round  2: use sbox 1
					// round  3: use sbox 0
					// round  4: use sbox 7
					// ...
					// round 31: use sbox 4
					// round 32: use sbox 3
					sbox[b] = SBOXES[(32 + 3 - i) % 8][val];
				}
			
				for (k = 0; k < NBITS / BITS_PER_WORD; k++)
				{
					for (b = 0; b < BITS_PER_WORD; b++)
					{
						int x ;
						x= (sbox[b] & (1 << k)) >> k;
						if (x != 0) 
							key[(k * BITS_PER_WORD) + b] = 1;
						else
							key[(k * BITS_PER_WORD) + b] = 0;
					}
				}
				for (b = 0; b < NBITS; b++)
				{
					keys[i][b] = key[IP[b]];
				}			
			}
		
		}
		work
		{
			int i;
			for (i = 0; i < NBITS; i++)
			{
				Out[i].x = keys[round][i];			
			}		
		}
		window {
			Out	tumbling(NBITS);	
			In sliding(0,0);
		}

	};
}

composite Xor(output stream<int x>Out,input stream<int x>In)
{
	param
		int n;
	Out = Xor(In)
	{
		work
		{
			int x = In[0].x;
			int i;
			for (i = 1; i < n; i++)
			{
				int y = In[i].x;
				x = x ^ y;
			}				
			Out[0].x = x;
		}	
		window{
			Out	tumbling(1);
			In	sliding(n,n);	
		}		
	};
}

composite rawL(output stream<int x>Out,input stream<int x>In)
{
	Out = rawL(In)
	{
		work
		{
			Out[0].x = (In[16].x ^ In[52].x ^ In[56].x ^ In[70].x ^ In[83].x ^ In[94].x ^ In[105].x);
			Out[1].x = (In[72].x ^ In[114].x ^ In[125].x);
			Out[2].x = (In[2].x ^ In[9].x ^ In[15].x ^ In[30].x ^ In[76].x ^ In[84].x ^ In[126].x);
			Out[3].x = (In[36].x ^ In[90].x ^ In[103].x);
			Out[4].x = (In[20].x ^ In[56].x ^ In[60].x ^ In[74].x ^ In[87].x ^ In[98].x ^ In[109].x);
			Out[5].x = (In[1].x ^ In[76].x ^ In[118].x);
			Out[6].x = (In[2].x ^ In[6].x ^ In[13].x ^ In[19].x ^ In[34].x ^ In[80].x ^ In[88].x);
			Out[7].x = (In[40].x ^ In[94].x ^ In[107].x);
			Out[8].x = (In[24].x ^ In[60].x ^ In[64].x ^ In[78].x ^ In[91].x ^ In[102].x ^ In[113].x);
			Out[9].x = (In[5].x ^ In[80].x ^ In[122].x);
			Out[10].x = (In[6].x ^ In[10].x ^ In[17].x ^ In[23].x ^ In[38].x ^ In[84].x ^ In[92].x);
			Out[11].x = (In[44].x ^ In[98].x ^ In[111].x);
			Out[12].x = (In[28].x ^ In[64].x ^ In[68].x ^ In[82].x ^ In[95].x ^ In[106].x ^ In[117].x);
			Out[13].x = (In[9].x ^ In[84].x ^ In[126].x);
			Out[14].x = (In[10].x ^ In[14].x ^ In[21].x ^ In[27].x ^ In[42].x ^ In[88].x ^ In[96].x);
			Out[15].x = (In[48].x ^ In[102].x ^ In[115].x);
			Out[16].x = (In[32].x ^ In[68].x ^ In[72].x ^ In[86].x ^ In[99].x ^ In[110].x ^ In[121].x);
			Out[17].x = (In[2].x ^ In[13].x ^ In[88].x);
			Out[18].x = (In[14].x ^ In[18].x ^ In[25].x ^ In[31].x ^ In[46].x ^ In[92].x ^ In[100].x);
			Out[19].x = (In[52].x ^ In[106].x ^ In[119].x);
			Out[20].x = (In[36].x ^ In[72].x ^ In[76].x ^ In[90].x ^ In[103].x ^ In[114].x ^ In[125].x);
			Out[21].x = (In[6].x ^ In[17].x ^ In[92].x);
			Out[22].x = (In[18].x ^ In[22].x ^ In[29].x ^ In[35].x ^ In[50].x ^ In[96].x ^ In[104].x);
			Out[23].x = (In[56].x ^ In[110].x ^ In[123].x);
			Out[24].x = (In[1].x ^ In[40].x ^ In[76].x ^ In[80].x ^ In[94].x ^ In[107].x ^ In[118].x);
			Out[25].x = (In[10].x ^ In[21].x ^ In[96].x);
			Out[26].x = (In[22].x ^ In[26].x ^ In[33].x ^ In[39].x ^ In[54].x ^ In[100].x ^ In[108].x);
			Out[27].x = (In[60].x ^ In[114].x ^ In[127].x);
			Out[28].x = (In[5].x ^ In[44].x ^ In[80].x ^ In[84].x ^ In[98].x ^ In[111].x ^ In[122].x);
			Out[29].x = (In[14].x ^ In[25].x ^ In[100].x);
			Out[30].x = (In[26].x ^ In[30].x ^ In[37].x ^ In[43].x ^ In[58].x ^ In[104].x ^ In[112].x);
			Out[31].x = (In[3].x ^ In[118].x);
			Out[32].x = (In[9].x ^ In[48].x ^ In[84].x ^ In[88].x ^ In[102].x ^ In[115].x ^ In[126].x);
			Out[33].x = (In[18].x ^ In[29].x ^ In[104].x);
			Out[34].x = (In[30].x ^ In[34].x ^ In[41].x ^ In[47].x ^ In[62].x ^ In[108].x ^ In[116].x);
			Out[35].x = (In[7].x ^ In[122].x);
			Out[36].x = (In[2].x ^ In[13].x ^ In[52].x ^ In[88].x ^ In[92].x ^ In[106].x ^ In[119].x);
			Out[37].x = (In[22].x ^ In[33].x ^ In[108].x);
			Out[38].x = (In[34].x ^ In[38].x ^ In[45].x ^ In[51].x ^ In[66].x ^ In[112].x ^ In[120].x);
			Out[39].x = (In[11].x ^ In[126].x);
			Out[40].x = (In[6].x ^ In[17].x ^ In[56].x ^ In[92].x ^ In[96].x ^ In[110].x ^ In[123].x);
			Out[41].x = (In[26].x ^ In[37].x ^ In[112].x);
			Out[42].x = (In[38].x ^ In[42].x ^ In[49].x ^ In[55].x ^ In[70].x ^ In[116].x ^ In[124].x);
			Out[43].x = (In[2].x ^ In[15].x ^ In[76].x);
			Out[44].x = (In[10].x ^ In[21].x ^ In[60].x ^ In[96].x ^ In[100].x ^ In[114].x ^ In[127].x);
			Out[45].x = (In[30].x ^ In[41].x ^ In[116].x);
			Out[46].x = (In[0].x ^ In[42].x ^ In[46].x ^ In[53].x ^ In[59].x ^ In[74].x ^ In[120].x);
			Out[47].x = (In[6].x ^ In[19].x ^ In[80].x);
			Out[48].x = (In[3].x ^ In[14].x ^ In[25].x ^ In[100].x ^ In[104].x ^ In[118].x);
			Out[49].x = (In[34].x ^ In[45].x ^ In[120].x);
			Out[50].x = (In[4].x ^ In[46].x ^ In[50].x ^ In[57].x ^ In[63].x ^ In[78].x ^ In[124].x);
			Out[51].x = (In[10].x ^ In[23].x ^ In[84].x);
			Out[52].x = (In[7].x ^ In[18].x ^ In[29].x ^ In[104].x ^ In[108].x ^ In[122].x);
			Out[53].x = (In[38].x ^ In[49].x ^ In[124].x);
			Out[54].x = (In[0].x ^ In[8].x ^ In[50].x ^ In[54].x ^ In[61].x ^ In[67].x ^ In[82].x);
			Out[55].x = (In[14].x ^ In[27].x ^ In[88].x);
			Out[56].x = (In[11].x ^ In[22].x ^ In[33].x ^ In[108].x ^ In[112].x ^ In[126].x);
			Out[57].x = (In[0].x ^ In[42].x ^ In[53].x);
			Out[58].x = (In[4].x ^ In[12].x ^ In[54].x ^ In[58].x ^ In[65].x ^ In[71].x ^ In[86].x);
			Out[59].x = (In[18].x ^ In[31].x ^ In[92].x);
			Out[60].x = (In[2].x ^ In[15].x ^ In[26].x ^ In[37].x ^ In[76].x ^ In[112].x ^ In[116].x);
			Out[61].x = (In[4].x ^ In[46].x ^ In[57].x);
			Out[62].x = (In[8].x ^ In[16].x ^ In[58].x ^ In[62].x ^ In[69].x ^ In[75].x ^ In[90].x);
			Out[63].x = (In[22].x ^ In[35].x ^ In[96].x);
			Out[64].x = (In[6].x ^ In[19].x ^ In[30].x ^ In[41].x ^ In[80].x ^ In[116].x ^ In[120].x);
			Out[65].x = (In[8].x ^ In[50].x ^ In[61].x);
			Out[66].x = (In[12].x ^ In[20].x ^ In[62].x ^ In[66].x ^ In[73].x ^ In[79].x ^ In[94].x);
			Out[67].x = (In[26].x ^ In[39].x ^ In[100].x);
			Out[68].x = (In[10].x ^ In[23].x ^ In[34].x ^ In[45].x ^ In[84].x ^ In[120].x ^ In[124].x);
			Out[69].x = (In[12].x ^ In[54].x ^ In[65].x);
			Out[70].x = (In[16].x ^ In[24].x ^ In[66].x ^ In[70].x ^ In[77].x ^ In[83].x ^ In[98].x);
			Out[71].x = (In[30].x ^ In[43].x ^ In[104].x);
			Out[72].x = (In[0].x ^ In[14].x ^ In[27].x ^ In[38].x ^ In[49].x ^ In[88].x ^ In[124].x);
			Out[73].x = (In[16].x ^ In[58].x ^ In[69].x);
			Out[74].x = (In[20].x ^ In[28].x ^ In[70].x ^ In[74].x ^ In[81].x ^ In[87].x ^ In[102].x);
			Out[75].x = (In[34].x ^ In[47].x ^ In[108].x);
			Out[76].x = (In[0].x ^ In[4].x ^ In[18].x ^ In[31].x ^ In[42].x ^ In[53].x ^ In[92].x);
			Out[77].x = (In[20].x ^ In[62].x ^ In[73].x);
			Out[78].x = (In[24].x ^ In[32].x ^ In[74].x ^ In[78].x ^ In[85].x ^ In[91].x ^ In[106].x);
			Out[79].x = (In[38].x ^ In[51].x ^ In[112].x);
			Out[80].x = (In[4].x ^ In[8].x ^ In[22].x ^ In[35].x ^ In[46].x ^ In[57].x ^ In[96].x);
			Out[81].x = (In[24].x ^ In[66].x ^ In[77].x);
			Out[82].x = (In[28].x ^ In[36].x ^ In[78].x ^ In[82].x ^ In[89].x ^ In[95].x ^ In[110].x);
			Out[83].x = (In[42].x ^ In[55].x ^ In[116].x);
			Out[84].x = (In[8].x ^ In[12].x ^ In[26].x ^ In[39].x ^ In[50].x ^ In[61].x ^ In[100].x);
			Out[85].x = (In[28].x ^ In[70].x ^ In[81].x);
			Out[86].x = (In[32].x ^ In[40].x ^ In[82].x ^ In[86].x ^ In[93].x ^ In[99].x ^ In[114].x);
			Out[87].x = (In[46].x ^ In[59].x ^ In[120].x);
			Out[88].x = (In[12].x ^ In[16].x ^ In[30].x ^ In[43].x ^ In[54].x ^ In[65].x ^ In[104].x);
			Out[89].x = (In[32].x ^ In[74].x ^ In[85].x);
			Out[90].x = (In[36].x ^ In[90].x ^ In[103].x ^ In[118].x);
			Out[91].x = (In[50].x ^ In[63].x ^ In[124].x);
			Out[92].x = (In[16].x ^ In[20].x ^ In[34].x ^ In[47].x ^ In[58].x ^ In[69].x ^ In[108].x);
			Out[93].x = (In[36].x ^ In[78].x ^ In[89].x);
			Out[94].x = (In[40].x ^ In[94].x ^ In[107].x ^ In[122].x);
			Out[95].x = (In[0].x ^ In[54].x ^ In[67].x);
			Out[96].x = (In[20].x ^ In[24].x ^ In[38].x ^ In[51].x ^ In[62].x ^ In[73].x ^ In[112].x);
			Out[97].x = (In[40].x ^ In[82].x ^ In[93].x);
			Out[98].x = (In[44].x ^ In[98].x ^ In[111].x ^ In[126].x);
			Out[99].x = (In[4].x ^ In[58].x ^ In[71].x);
			Out[100].x = (In[24].x ^ In[28].x ^ In[42].x ^ In[55].x ^ In[66].x ^ In[77].x ^ In[116].x);
			Out[101].x = (In[44].x ^ In[86].x ^ In[97].x);
			Out[102].x = (In[2].x ^ In[48].x ^ In[102].x ^ In[115].x);
			Out[103].x = (In[8].x ^ In[62].x ^ In[75].x);
			Out[104].x = (In[28].x ^ In[32].x ^ In[46].x ^ In[59].x ^ In[70].x ^ In[81].x ^ In[120].x);
			Out[105].x = (In[48].x ^ In[90].x ^ In[101].x);
			Out[106].x = (In[6].x ^ In[52].x ^ In[106].x ^ In[119].x);
			Out[107].x = (In[12].x ^ In[66].x ^ In[79].x);
			Out[108].x = (In[32].x ^ In[36].x ^ In[50].x ^ In[63].x ^ In[74].x ^ In[85].x ^ In[124].x);
			Out[109].x = (In[52].x ^ In[94].x ^ In[105].x);
			Out[110].x = (In[10].x ^ In[56].x ^ In[110].x ^ In[123].x);
			Out[111].x = (In[16].x ^ In[70].x ^ In[83].x);
			Out[112].x = (In[0].x ^ In[36].x ^ In[40].x ^ In[54].x ^ In[67].x ^ In[78].x ^ In[89].x);
			Out[113].x = (In[56].x ^ In[98].x ^ In[109].x);
			Out[114].x = (In[14].x ^ In[60].x ^ In[114].x ^ In[127].x);
			Out[115].x = (In[20].x ^ In[74].x ^ In[87].x);
			Out[116].x = (In[4].x ^ In[40].x ^ In[44].x ^ In[58].x ^ In[71].x ^ In[82].x ^ In[93].x);
			Out[117].x = (In[60].x ^ In[102].x ^ In[113].x);
			Out[118].x = (In[3].x ^ In[18].x ^ In[72].x ^ In[114].x ^ In[118].x ^ In[125].x);
			Out[119].x = (In[24].x ^ In[78].x ^ In[91].x);
			Out[120].x = (In[8].x ^ In[44].x ^ In[48].x ^ In[62].x ^ In[75].x ^ In[86].x ^ In[97].x);
			Out[121].x = (In[64].x ^ In[106].x ^ In[117].x);
			Out[122].x = (In[1].x ^ In[7].x ^ In[22].x ^ In[76].x ^ In[118].x ^ In[122].x);
			Out[123].x = (In[28].x ^ In[82].x ^ In[95].x);
			Out[124].x = (In[12].x ^ In[48].x ^ In[52].x ^ In[66].x ^ In[79].x ^ In[90].x ^ In[101].x);
			Out[125].x = (In[68].x ^ In[110].x ^ In[121].x);
			Out[126].x = (In[5].x ^ In[11].x ^ In[26].x ^ In[80].x ^ In[122].x ^ In[126].x);
			Out[127].x = (In[32].x ^ In[86].x ^ In[99].x);			
		}
		window {
			Out	tumbling(128);
			In	sliding(128,128);	
		}	
	};
}
composite SP_Xor_Sbox_rawL(output stream<int x>Out,input stream<int x>In)
{
	param 
		int vector, int round;
	stream<int x> OutSP,OutXor,OutXb;
	int i;
	OutSP = splitjoin(In)
	{
		split roundrobin(NBITS, 0);
		//Identify_slowKeySchedule()(vector, round);
		for(i=0;i<2;i++)
			if(i==0) 
				add Identity();
			else 
				add slowKeySchedule(vector, round);
		join roundrobin();		
	};
	OutXor = Xor(OutSP)(2);
	OutXb = Sbox(OutXor)(round % 8);
	Out = rawL(OutXb)();		
}
composite SP_Xor_Sbox_SP_KeySchedule(output stream<int x>Out,input stream<int x>In)
{
	param 
		int vector, int round;
	stream<int x> OutSP,OutXor,OutXb,OutSPKS;
	int i;
	OutSP = splitjoin(In)
	{
		split roundrobin(NBITS, 0);
		for(i=0;i<2;i++)
			if(i==0) 
				add Identity();
			else 
				add slowKeySchedule(vector, round);
		join roundrobin();		
	};
	OutXor = Xor(OutSP)(2);
	OutXb = Sbox(OutXor)(round % 8);
	OutSPKS = splitjoin(OutXb)
	{
		split roundrobin(NBITS, 0);
		//Identify_KeySchedule()(vector, MAXROUNDS);
		for(i=0;i<2;i++)
			if(i==0) 
				add Identity();
			else 
				add KeySchedule(vector, MAXROUNDS);
		join roundrobin();		
	};
	Out = Xor(OutSPKS)(2);		
}
composite SerpentEncoder(output stream<int x>Out,input stream<int x>In)
{
	param
		int vector;
	stream<int x> Outpipeline,OutP;
	int i;
	OutP = Permute(In)(NBITS , IP);
	Outpipeline = pipeline(OutP)
	{
		for(i= 0; i < MAXROUNDS ; i++)
		{
			//R()(vector,i);
			if(i<MAXROUNDS - 1) 
				add SP_Xor_Sbox_rawL(vector,i);
			else 
				add SP_Xor_Sbox_SP_KeySchedule(vector,i);
		}		
	};
	Out = Permute(Outpipeline)(NBITS , FP);
}
composite BitstoInts(output stream<int x>Out,input stream<int x>In)
{
	param
		int n;
	Out = BitstoInts(In)
	{
		work
		{
			int v = 0;
			int i;
			for (i = 0; i < n; i++)
			{
				v = v | (In[i].x << i);
			}
			Out[0].x = v;
		
		}
	};
		

}


composite Main()
{
	int testvector = 2;
	stream<int x>Source,OutITB,SE,OutBTI;
	Source = PlainTextSource()//FileReader
	{
		int TEXT[5][4] = {{0x00000000, 0x00000000, 0x00000000, 0x00000000},  // 00000000000000000000000000000000
						{0x00000003, 0x00000002, 0x00000001, 0x00000000},  // 00000003000000020000000100000000
						{0x92efa3ca, 0x9477794d, 0x31f4df7b, 0xce23e60a},  // 92efa3ca9477794d31f4df7bce23e60a
						{0x41133a29, 0xb97e3b42, 0x31549e8c, 0x2d0af27e},  // 41133a29b97e3b4231549e8c2d0af27e
						{0x6ee8edc7, 0x4dcfefd0, 0xc7beaee4, 0xcbcbc9c2}}; // 6ee8edc74dcfefd0c7beaee4cbcbc9c2
		int vector= testvector;
		work
		{
			Source[0].x = TEXT[vector][3];
			Source[1].x = TEXT[vector][2];
			Source[2].x = TEXT[vector][1];
			Source[3].x = TEXT[vector][0];
		}
		window {
			Source	tumbling(4);
		}
	
	};
	OutITB = IntoBits(Source)();
	SE = SerpentEncoder(OutITB)(testvector);
	OutBTI = BitstoInts(SE)(4);
	HexPrinter(OutBTI) //FileWriter
	{
		int bits ;
		int bytes ;
		init
		{
			bits = NBITS;
			bytes = bits/4;		
		}
		work
		{
			int i;
			for (i = bytes - 1; i >= 0; i--) 
			{
				int v = OutBTI[i].x;
				if (v < 10) print(v);
				else if (v == 10) print("A");
				else if (v == 11) print("B");
				else if (v == 12) print("C");
				else if (v == 13) print("D");
				else if (v == 14) print("E");
				else if (v == 15) print("F");
				else {
					print("ERROR: "); 
					println(v);
				}
			}
			println("");
		
		}	
		window {
			OutBTI	sliding(bytes,bytes);	
		}		
	};		
	//HexPrinter(SE)(CIPHERTEXT, NBITS); 

}

