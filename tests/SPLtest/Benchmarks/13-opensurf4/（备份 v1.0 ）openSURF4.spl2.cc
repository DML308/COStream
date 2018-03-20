#define WIDTH 40
#define HEIGHT 32
#define WIDTHSTEP WIDTH*4
#define GS(x,y,s) ((1.0f/(2.0f*pi*s*s)) * exp( -((x)*(x)+(y)*(y))/(2.0f*s*s)))
#define GW (int)(WIDTH / 2)
#define GH (int)(HEIGHT / 2)
#define R1 (int)(GW*GH)
#define R5 (int)((int)(GW/2) * (int)(GH/2))
#define R7 (int)((int)(GW/4) * (int)(GH/4))
#define R9 (int)((int)(GW/8) * (int)(GH/8))
#define R11 (int)((int)(GW/16) * (int)(GH/16))

// Get image attributes	
const int R_w[12] = {GW,GW,GW,GW,GW/2,GW/2,GW/4,GW/4,GW/8,GW/8,GW/16,GW/16};
const int R_filter[12] = {9,15,21,27,39,51,75,99,147,195,291,387};
const  int init_sample = 2;
const  double thres = 0.0004;
const int R_w[12] = {GW,GW,GW,GW,GW/2,GW/2,GW/4,GW/4,GW/8,GW/8,GW/16,GW/16};
const int R_h[12] = {GH,GH,GH,GH,GH/2,GH/2,GH/4,GH/4,GH/8,GH/8,GH/16,GH/16};
const int R_step[12] = {2,2,2,2,4,4,8,8,16,16,32,32};

double resX[109] = {0}, resY[109] = {0}, Ang[109] = {0};

double int_img[WIDTH*HEIGHT] = {
0.105882,
0.211765,
0.317647,
0.423529,
0.529412,
0.635294,
0.741176,
0.847059,
0.952941,
1.05882,
1.16471,
1.27059,
1.37647,
1.48235,
1.58824,
1.69412,
1.8,
1.90588,
2.01177,
2.11765,
2.21961,
2.32157,
2.42353,
2.52549,
2.62353,
2.72157,
2.81961,
2.91765,
3.01569,
3.11373,
3.21177,
3.3098,
3.41177,
3.51373,
3.61569,
3.72157,
3.82745,
3.93333,
4.04314,
4.15294,
4.26275,
4.37255,
4.48628,
4.60392,
4.72157,
4.84314,
4.96863,
5.09412,
5.21961,
5.34118,
5.46275,
5.58432,
5.70981,
5.83922,
5.97255,
6.10588,
6.25098,
6.4,
6.55294,
6.70981,
6.87451,
7.04314,
7.21961,
7.39608,
7.56863,
7.7451,
7.92549,
8.10589,
8.28236,
8.45883,
8.64314,
8.83138,
9.01961,
9.21177,
9.41177,
9.61961,
9.8353,
10.0588,
10.2824,
10.5098,
10.7333,
10.9569,
11.1882,
11.4235,
11.6588,
11.8941,
12.1255,
12.3529,
12.5843,
12.8157,
13.0431,
13.2628,
13.4588,
13.6314,
13.7765,
13.9059,
14.0549,
14.2,
14.3451,
14.4863,
14.6235,
14.7569,
14.8902,
15.0235,
15.1608,
15.2941,
15.4235,
15.549,
15.6745,
15.7961,
15.9216,
16.0471,
16.1647,
16.2824,
16.4,
16.5177,
16.6353,
16.7569,
16.8784,
17,
17.1216,
17.2431,
17.3647,
17.4863,
17.6078,
17.7294,
17.851,
17.9726,
18.0941,
18.2157,
18.3333,
18.451,
18.5686,
18.6824,
18.7961,
18.9098,
19.0235,
19.1373,
19.251,
19.3608,
19.4706,
19.5804,
19.6902,
19.7961,
19.902,
20.0079,
20.1137,
20.2196,
20.3255,
20.4314,
20.5373,
20.6431,
20.749,
20.8549,
20.9608,
21.0667,
21.1726,
21.2784,
21.3882,
21.4981,
21.6039,
21.7098,
21.8157,
21.9216,
22.0275,
22.1373,
22.2471,
22.3569,
22.4628,
22.5686,
22.6745,
22.7804,
22.8863,
22.9922,
23.0981,
23.2039,
23.3137,
23.4235,
23.5334,
23.6432,
23.753,
23.8628,
23.9726,
24.0824,
24.1883,
24.2941,
24.4,
24.5059,
24.6118,
24.7177,
24.8236,
24.9294,
25.0353,
25.1412,
25.2471,
25.353,
25.4588,
25.5647,
25.6706,
25.7765,
25.8824,
25.9883,
26.0941,
26.2,
26.3059,
26.4118,
26.5177,
26.6236,
26.7294,
26.8353,
26.9412,
27.0471,
27.153,
27.2589,
27.3647,
27.4706,
27.5765,
27.6824,
27.7883,
27.8941,
28,
28.1059,
28.2118,
28.3177,
28.4275,
28.5334,
28.6392,
28.7412,
28.8432,
28.9491,
29.0549,
29.1647,
29.2706,
29.3765,
29.4824,
29.5883,
29.6942,
29.8,
29.9059,
30.0118,
30.1177,
30.2236,
30.3294,
30.4353,
30.5412,
30.6471,
30.753,
30.8589,
30.9647,
31.0706,
31.1765,
31.2824,
31.3883,
31.4942,
31.6,
31.7059,
31.8157,
31.9255,
32.0353,
32.1451,
32.2549,
32.3647,
32.4745,
32.5843,
32.6902,
32.7961,
32.9059,
33.0157,
33.1255,
33.2392,
33.353,
33.4667,
33.5726,
33.6784,
33.7843,
33.8902,
33.9961,
34.102,
34.2079,
34.3137,
34.4196,
34.5255,
34.6314,
34.7373,
34.8431,
34.949,
35.0549,
35.1608,
35.2667,
35.3725,
35.4784,
35.5843,
35.6902,
35.7961,
35.9019,
36.0078,
36.1137,
36.2196,
36.3255,
36.4313,
36.5372,
36.6431,
36.749,
36.8549,
36.9608,
37.0666,
37.1725,
37.2784,
37.3843,
37.4902,
37.596,
37.7019,
37.8078,
37.9137,
38.0196,
38.1254,
38.2313,
38.3372,
38.4431,
38.549,
38.6627,
38.7764,
38.8901,
39.0039,
39.1176,
39.2313,
39.345,
39.4588,
39.5646,
39.6705,
39.7764,
39.8823,
39.9882,
40.094,
40.1999,
40.3058,
40.4117,
40.5136,
40.6156,
40.7176,
40.8195,
40.9254,
41.0313,
41.1411,
41.247,
41.3568,
41.4666,
41.5803,
41.694,
41.8038,
41.9058,
42.0038,
42.1058,
42.2077,
42.3136,
42.4195,
42.5254,
42.6313,
42.7332,
42.8352,
42.9371,
43.0391,
43.141,
43.243,
43.3489,
43.4548,
43.5606,
43.6665,
43.7724,
43.8783,
43.9842,
44.0901,
44.1959,
44.3018,
44.4077,
44.5136,
44.6234,
44.7332,
44.843,
44.9528,
45.0626,
45.1724,
45.2822,
45.392,
45.5057,
45.6194,
45.7332,
45.843,
45.9488,
46.0508,
46.1528,
46.2547,
46.3645,
46.4743,
46.5841,
46.6939,
46.8037,
46.9135,
47.0233,
47.1331,
47.239,
47.341,
47.4429,
47.5449,
47.6469,
47.7449,
47.8429,
47.941,
48.039,
48.1371,
48.2351,
48.3331,
48.4312,
48.5292,
48.6272,
48.7253,
48.8233,
48.9214,
49.0194,
49.1174,
49.2155,
49.3135,
49.4115,
49.5096,
49.5998,
49.69,
49.7802,
49.8704,
49.9606,
50.0507,
50.1409,
50.2311,
50.3213,
50.4115,
50.5017,
50.5919,
50.6821,
50.7723,
50.8625,
50.9527,
51.0468,
51.1409,
51.235,
51.3292,
51.4233,
51.5174,
51.6115,
51.7056,
51.7997,
51.8939,
51.988,
52.0821,
52.1762,
52.2703,
52.3644,
52.4586,
52.5487,
52.6389,
52.7291,
52.8193,
52.9095,
53.0036,
53.0978,
53.1919,
53.286,
53.3801,
53.4742,
53.5683,
53.6585,
53.7487,
53.8389,
53.9291,
54.0193,
54.1095,
54.1997,
54.2899,
54.3762,
54.4624,
54.5487,
54.635,
54.7213,
54.8075,
54.8938,
54.984,
55.0781,
55.1722,
55.2703,
55.3683,
55.4585,
55.5487,
55.6389,
55.7291,
55.8193,
55.9095,
55.9997,
56.0899,
56.1722,
56.2546,
56.3369,
56.4193,
56.5016,
56.584,
56.6663,
56.7487,
56.835,
56.9212,
57.0075,
57.0938,
57.18,
57.2663,
57.3526,
57.4389,
57.5251,
57.6153,
57.7055,
57.7957,
57.8898,
57.984,
58.082,
58.18,
58.2702,
58.3604,
58.4506,
58.5408,
58.631,
58.7212,
58.8114,
58.9016,
58.9996,
59.0977,
59.1957,
59.2937,
59.3918,
59.4898,
59.5879,
59.6859,
59.78,
59.8741,
59.9682,
60.0624,
60.1565,
60.2467,
60.3369,
60.4271,
60.5212,
60.6153,
60.7094,
60.7996,
60.8898,
60.98,
61.0702,
61.1604,
61.2506,
61.3447,
61.4427,
61.5408,
61.627,
61.7055,
61.78,
61.8623,
61.9721,
62.0702,
62.1564,
62.2388,
62.329,
62.4192,
62.5055,
62.5878,
62.678,
62.7643,
62.8427,
62.9211,
63.0035,
63.0898,
63.18,
63.2702,
63.3564,
63.4427,
63.529,
63.6152,
63.7015,
63.7878,
63.8741,
63.9603,
64.0466,
64.1329,
64.2192,
64.3054,
64.3917,
64.478,
64.5643,
64.6505,
64.7368,
64.8231,
64.9093,
64.9956,
65.0819,
65.1682,
65.2544,
65.3407,
65.427,
65.5133,
65.5995,
65.6858,
65.7721,
65.8584,
65.9446,
66.0309,
66.1133,
66.1956,
66.278,
66.3603,
66.4427,
66.525,
66.6074,
66.6897,
66.7721,
66.8544,
66.9368,
67.0191,
67.1015,
67.1838,
67.2662,
67.3485,
67.427,
67.5054,
67.5799,
67.6544,
67.7289,
67.8034,
67.8819,
67.9603,
68.0466,
68.1328,
68.2191,
68.3054,
68.3877,
68.4701,
68.5485,
68.6269,
68.7171,
68.8073,
68.8975,
68.9916,
69.0858,
69.1838,
69.2818,
69.3799,
69.474,
69.5681,
69.6622,
69.7563,
69.8504,
69.9446,
70.0387,
70.1328,
70.2191,
70.3053,
70.3916,
70.4779,
70.5642,
70.6504,
70.7367,
70.823,
70.921,
71.0191,
71.1132,
71.2073,
71.3014,
71.3916,
71.4818,
71.572,
71.6622,
71.7524,
71.8426,
71.9328,
72.023,
72.1132,
72.2034,
72.2935,
72.3837,
72.4739,
72.5641,
72.6543,
72.7445,
72.8347,
72.9249,
73.0151,
73.1092,
73.2033,
73.2975,
73.3916,
73.4857,
73.5798,
73.6739,
73.768,
73.8582,
73.9484,
74.0386,
74.1288,
74.2229,
74.317,
74.4112,
74.5053,
74.5955,
74.6857,
74.7759,
74.8661,
74.9563,
75.0464,
75.1366,
75.2268,
75.317,
75.4072,
75.4974,
75.5876,
75.6778,
75.7641,
75.8504,
75.9366,
76.0307,
76.1209,
76.2111,
76.3013,
76.3915,
76.4856,
76.5798,
76.6778,
76.768,
76.8582,
76.9484,
77.0386,
77.1288,
77.219,
77.3092,
77.3994,
77.4935,
77.5876,
77.6817,
77.7758,
77.8699,
77.9641,
78.0582,
78.1523,
78.2464,
78.3405,
78.4346,
78.5287,
78.6189,
78.7091,
78.7993,
78.8895,
78.9836,
79.0778,
79.1719,
79.266,
79.3601,
79.4542,
79.5483,
79.6425,
79.7327,
79.8228,
79.913,
80.0032,
80.0934,
80.1836,
80.2738,
80.364,
80.4542,
80.5444,
80.6346,
80.7248,
80.815,
80.9052,
80.9954,
81.0856,
81.1679,
81.2503,
81.3326,
81.415,
81.4973,
81.5797,
81.662,
81.7444,
81.8267,
81.9091,
81.9914,
82.0738,
82.1561,
82.2385,
82.3208,
82.4032,
82.4895,
82.5757,
82.662,
82.7483,
82.8346,
82.9208,
83.0071,
83.0934,
83.1836,
83.2738,
83.364,
83.4542,
83.5444,
83.6346,
83.7247,
83.8149,
83.9051,
83.9953,
84.0855,
84.1757,
84.2659,
84.3561,
84.4463,
84.5365,
84.6267,
84.7169,
84.8071,
84.8973,
84.9875,
85.0777,
85.1679,
85.2581,
85.3483,
85.4384,
85.5286,
85.6188,
85.709,
85.7992,
85.8894,
85.9796,
86.0737,
86.1639,
86.2541,
86.3443,
86.4345,
86.5208,
86.6071,
86.6933,
86.7796,
86.8659,
86.9521,
87.0384,
87.1247,
87.211,
87.2972,
87.3835,
87.4698,
87.5561,
87.6423,
87.7286,
87.8149,
87.9012,
87.9874,
88.0737,
88.1639,
88.2541,
88.3443,
88.4345,
88.5247,
88.6149,
88.7051,
88.7953,
88.8894,
88.9835,
89.0776,
89.1717,
89.2658,
89.36,
89.4541,
89.5482,
89.6384,
89.7286,
89.8188,
89.909,
89.9992,
90.0894,
90.1796,
90.2697,
90.3599,
90.4501,
90.5403,
90.6305,
90.7207,
90.8109,
90.9011,
90.9913,
91.0854,
91.1795,
91.2737,
91.3638,
91.454,
91.5442,
91.6344,
91.7207,
91.8109,
91.9011,
91.9952,
92.0893,
92.1795,
92.2697,
92.356,
92.4423,
92.5364,
92.6305,
92.7246,
92.8187,
92.9128,
93.007,
93.1011,
93.1952,
93.2854,
93.3756,
93.4658,
93.556,
93.6462,
93.7364,
93.8266,
93.9167,
94.003,
94.0893,
94.1756,
94.2618,
94.3481,
94.4344,
94.5207,
94.6069,
94.6971,
94.7834,
94.8697,
94.9559,
95.0383,
95.1207,
95.1991,
95.2775,
95.3599,
95.4422,
95.5246,
95.6069,
95.6853,
95.7638,
95.8422,
95.9206,
96.0069,
96.0932,
96.1795,
96.2657,
96.352,
96.4383,
96.5246,
96.6108,
96.6932,
96.7755,
96.8579,
96.9402,
97.0265,
97.1128,
97.1991,
97.2853,
97.3716,
97.4579,
97.5441,
97.6304,
97.7167,
97.803,
97.8892,
97.9755,
98.0618,
98.1481,
98.2304,
98.3128,
98.3912,
98.4696,
98.548,
98.6226,
98.6971,
98.7716,
98.85,
98.9284,
99.0108,
99.0971,
99.1833,
99.2696,
99.352,
99.4343,
99.5206,
99.6069,
99.6971,
99.7872,
99.8774,
99.9716,
100.058,
100.144,
100.23,
100.317,
100.403,
100.489,
100.575,
100.662,
100.752,
100.842,
100.932,
101.023,
101.109,
101.195,
101.281,
101.368,
101.454,
101.54,
101.626,
101.713,
101.799,
101.885,
101.972,
102.058,
102.144,
102.23,
102.317,
102.403,
102.489,
102.575,
102.662,
102.748,
0.215686,
0.431373,
0.647059,
0.862745,
1.07843,
1.29412,
1.5098,
1.72549,
1.94118,
2.15686,
2.37255,
2.58824,
2.80392,
3.01961,
3.23529,
3.45098,
3.66275,
3.87451,
4.08628,
4.29804,
4.50588,
4.70981,
4.91373,
5.11765,
5.31373,
5.50981,
5.70588,
5.90196,
6.09804,
6.29412,
6.4902,
6.68628,
6.8902,
7.09412,
7.29804,
7.50981,
7.72157,
7.93333,
8.15294,
8.37255,
8.59216,
8.81177,
9.0353,
9.26667,
9.50196,
9.74118,
9.98824,
10.2353,
10.4824,
10.7255,
10.9686,
11.2118,
11.4627,
11.7177,
11.9804,
12.2471,
12.5294,
12.8196,
13.1177,
13.4235,
13.7451,
14.0745,
14.4157,
14.7608,
15.1059,
15.4588,
15.8196,
16.1843,
16.5412,
16.898,
17.2667,
17.6431,
18.0196,
18.4039,
18.8,
19.2118,
19.6392,
20.0784,
20.5177,
20.9647,
21.4039,
21.8471,
22.302,
22.7647,
23.2275,
23.6863,
24.1373,
24.5804,
25.0314,
25.4824,
25.9294,
26.3647,
26.7647,
27.1255,
27.4431,
27.7333,
28.0353,
28.3333,
28.6275,
28.9137,
29.1961,
29.4745,
29.7529,
30.0314,
30.3059,
30.5726,
30.8314,
31.0784,
31.3216,
31.5608,
31.8039,
32.0471,
32.2824,
32.5177,
32.7529,
32.9882,
33.2275,
33.4706,
33.7137,
33.9569,
34.1961,
34.4353,
34.6745,
34.9137,
35.153,
35.3922,
35.6314,
35.8706,
36.1137,
36.3569,
36.5922,
36.8275,
37.0628,
37.2902,
37.5177,
37.7451,
37.9726,
38.2,
38.4275,
38.6471,
38.8667,
39.0863,
39.3059,
39.5216,
39.7373,
39.953,
40.1686,
40.3843,
40.6,
40.8157,
41.0314,
41.2471,
41.4628,
41.6785,
41.8941,
42.1098,
42.3294,
42.549,
42.7726,
42.9961,
43.2079,
43.4196,
43.6314,
43.8432,
44.0588,
44.2785,
44.4981,
44.7177,
44.9334,
45.149,
45.3647,
45.5804,
45.7961,
46.0118,
46.2275,
46.4432,
46.6628,
46.8824,
47.102,
47.3216,
47.5412,
47.7608,
47.9804,
48.2,
48.4157,
48.6314,
48.8471,
49.0628,
49.2785,
49.4942,
49.7098,
49.9255,
50.1412,
50.3569,
50.5726,
50.7883,
51.004,
51.2197,
51.4353,
51.651,
51.8667,
52.0824,
52.2981,
52.5138,
52.7295,
52.9451,
53.1608,
53.3765,
53.5922,
53.8079,
54.0236,
54.2393,
54.455,
54.6706,
54.8863,
55.102,
55.3177,
55.5334,
55.7491,
55.9648,
56.1804,
56.3961,
56.6118,
56.8275,
57.0432,
57.255,
57.4628,
57.6667,
57.8706,
58.0785,
58.2903,
58.5059,
58.7177,
58.9295,
59.1412,
59.353,
59.5648,
59.7765,
59.9883,
60.2001,
60.4158,
60.6314,
60.8471,
61.0628,
61.2785,
61.4942,
61.7099,
61.9256,
62.1373,
62.3491,
62.5609,
62.7726,
62.9844,
63.1962,
63.4079,
63.6197
};
composite BoxIntegral(output stream<double x>Out,input stream<double x> In)
{	
	Out = BoxIntegral(In)
	{
		work
		{
			int row = In[0].x, col = In[1].x, rows = In[2].x, cols = In[3].x;
			int step = WIDTHSTEP/4;
			int r1=row-1, c1=col-1, r2=row+rows-1, c2=col+cols-1;
			double A=0.0, B=0.0, C=0.0, D=0.0;
			
			if(row > HEIGHT)	r1 = HEIGHT - 1;
			if(col > WIDTH)		c1 = WIDTH - 1;
			if((row + rows) > HEIGHT)	r2 =  HEIGHT- 1;
			if((col + cols) > WIDTH)	c2 =  WIDTH - 1;
			
			if (r1 >= 0 && c1 >= 0) A = int_img[r1 * step + c1];
			if (r1 >= 0 && c2 >= 0) B = int_img[r1 * step + c2];
			if (r2 >= 0 && c1 >= 0) C = int_img[r2 * step + c1];
			if (r2 >= 0 && c2 >= 0) D = int_img[r2 * step + c2];
			
			if((A - B - C + D) > 0)
				Out[0].x = A - B - C + D;
			else
				Out[0].x = 0.0;
		}
		window
		{
			In sliding(4,4);
			Out tumbling(1);
		}
	};
}

composite getResponse(output stream<double x>Out, input stream<int x>In)
{
	param
		int src,int t_width,int rr,int cc,
		double lay1[2][R1],double lay2[2][R1],double lay3[2][R1],double lay4[2][R1],double lay5[2][R5],double lay6[2][R5],
		double lay7[2][R7],double lay8[2][R7],double lay9[2][R9],double lay10[2][R9],double lay11[2][R11],double lay12[2][R11];
	
	const int width = R_w[src],scale = width / t_width; 
	
	Out = getResponsex(In){
		work
		{
			int r,c,row,column,index;
			r = In[0].x / t_width;
			c = In[0].x % t_width;
			row = rr + r;
			column = cc + c;
			index = (scale * row) * width + (scale * column);
			
			if(src==0){
				Out[0].x = lay1[0][index];
			}else if(src==1){
				Out[0].x = lay2[0][index];
			}else if(src==2){
				Out[0].x = lay3[0][index];
			}else if(src==3){
				Out[0].x = lay4[0][index];
			}else if(src==4){
				Out[0].x = lay5[0][index];
			}else if(src==5){
				Out[0].x = lay6[0][index];
			}else if(src==6){
				Out[0].x = lay7[0][index];
			}else if(src==7){
				Out[0].x = lay8[0][index];
			}else if(src==8){
				Out[0].x = lay9[0][index];
			}else if(src==9){
				Out[0].x = lay10[0][index];
			}else if(src==10){
				Out[0].x = lay11[0][index];
			}else{
				Out[0].x = lay12[0][index];
			}
		}
	};
}

composite candidateCheck(output stream<double x> Out,input stream<int x> In)
{
	param
		int t_width, int t_height, int layerBorder;
	
	Out = candidateCheck(In){
		work
		{
			int r,c;
			double flag=1.0;
			r = In[0].x / t_width;
			c = In[0].x % t_width;
			if (r <= layerBorder || r >= t_height - layerBorder || c <= layerBorder || c >= t_width - layerBorder)
				flag = 0.0;
			Out[0].x = flag;
		}
	};
}

composite isExtremum(output stream<int x>Out, input stream<int x>In)
{
	param
		int b,int m,int t,
		double lay1[2][R1],double lay2[2][R1],double lay3[2][R1],double lay4[2][R1],double lay5[2][R5],double lay6[2][R5],
		double lay7[2][R7],double lay8[2][R7],double lay9[2][R9],double lay10[2][R9],double lay11[2][R11],double lay12[2][R11];
	
	const int t_filter = R_filter[t], t_step = R_step[t], t_height = R_h[t], t_width = R_w[t];
	int layerBorder = (int)((t_filter + 1) / (2 * t_step));
	stream<double x> Response;
	
	Response = splitjoin(In){
		int rr=0,cc=0;
		split duplicate();
		add candidateCheck(t_width,t_height,layerBorder);
		add getResponse(m,t_width,rr,cc,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		for (rr = -1; rr <=1; ++rr)
		{
			for (cc = -1; cc <=1; ++cc)
			{
				add getResponse(t,t_width,rr,cc,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
				add getResponse(m,t_width,rr,cc,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
				add getResponse(b,t_width,rr,cc,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
			}
		}
		join roundrobin();	
	};
	Out = isExtremum(Response){
		work
		{
			int flag = 1;
			double candidate;
			int i;
			flag = (int)Response[0].x;
			if(flag == 1){
				candidate = Response[1].x;
				if (candidate < thres)
					flag = 0;
				for(i=2;i<11;i=i+3)
					if( flag==1 && (Response[i].x >= candidate || Response[i+1].x > candidate ||
					Response[i+2].x >= candidate) )
						flag = 0;
			}
			Out[0].x  = flag;
		}
		window
		{
			Response sliding(11,11);
			Out tumbling(1);
		}
	};
}

composite dump(output stream<int x>Out,input stream<int x>In)
{
	Out = dump(In){
		work
		{
			Out[0].x = In[0].x;
		}
	};
}

composite interpolateStep(output stream<double x,double y,double scale,double laplacian>Out,input stream<double x>In)
{
	param
		int b, int m, int t,
		double lay1[2][R1],double lay2[2][R1],double lay3[2][R1],double lay4[2][R1],double lay5[2][R5],double lay6[2][R5],
		double lay7[2][R7],double lay8[2][R7],double lay9[2][R9],double lay10[2][R9],double lay11[2][R11],double lay12[2][R11];
	
	int t_width = R_w[t];
	int filterStep = (R_filter[m] - R_filter[b]);
	int t_step = R_step[t];
	stream<int x> Pos1,Pos2,Flag;
	stream<double x> Responses;
	
	(Pos1,Flag) = separate(In){
		work
		{
			Pos1[0].x = In[0].x;
			Pos2[0].x = In[0].x;
			Flag[0].x = In[1].x;
		}
		window
		{
			In sliding(2,2);
		}
	};
	Responses = splitjoin(Pos1){ //共6+1+4*3 = 19 个
		split duplicate();
		add getResponse(m,t_width,0,1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,0,-1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,-1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(t,t_width,0,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(b,t_width,0,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);//上面的是deriv3D所需要的值
		add getResponse(m,t_width,0,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);  //v
		add getResponse(m,t_width,1,1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,1,-1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,-1,1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(m,t_width,-1,-1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(t,t_width,0,1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(t,t_width,0,-1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(b,t_width,0,1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(b,t_width,0,-1,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(t,t_width,1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(t,t_width,-1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(b,t_width,1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
		add getResponse(b,t_width,-1,0,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);		
		join roundrobin(); 
	};
	
	Out = interpolateStep(Pos2,Flag,Responses){
		work
		{
			double dx, dy, ds;
			double v, dxx, dyy, dss, dxy, dxs, dys;
			double rank;
			double H[3][3];
			double xi,xr,xc;
			double laplacian;
			int scale,index,r,c;
			if(Flag[0].x == 1){
				dx = (Responses[0].x - Responses[1].x) / 2.0;
				dy = (Responses[2].x - Responses[3].x) / 2.0;
				ds = (Responses[4].x - Responses[5].x) /2.0;
				v = Responses[6].x;
				dxx = Responses[0].x + Responses[1].x - 2*v;
				dyy = Responses[2].x + Responses[3].x - 2*v;
				dss = Responses[4].x + Responses[5].x - 2*v;
				dxy = ( Responses[7].x - Responses[8].x -
					    Responses[9].x + Responses[10].x ) / 4.0;
				dxs = ( Responses[11].x - Responses[12].x -
					    Responses[13].x + Responses[14].x ) / 4.0;
				dys = ( Responses[15].x - Responses[16].x -
						Responses[17].x + Responses[18].x ) / 4.0;	
				rank = dxx*dyy*dss + dxy*dys*dxs*2 -dxx*dys*dys - dss*dxy*dxy - dyy*dxs*dxs;
				rank = (rank > 0 ? rank : rank*(-1.0));
				H[0][0] = (dyy*dss-dys*dys)/rank;  H[1][1] = (dxx*dss - dxs*dxs) /rank; H[2][2] = (dxx*dyy - dxy*dxy) /rank;
				H[0][1] = (-1) * (dxy*dss-dxs*dys) / rank;H[1][0] = (-1) * (dxy*dss-dxs*dys) / rank;  
				H[0][2] = (dxy*dys - dyy*dxs) / rank;H[2][0] = (dxy*dys - dyy*dxs) / rank;
				H[1][2] = (-1) * (dxx*dys - dxs*dxy) /rank;H[2][1] = (-1) * (dxx*dys - dxs*dxy) /rank;
				
				xi = H[0][0] * dx + H[0][1] * dy +H[0][2] *ds;
				xr = H[1][0] * dx + H[1][1] * dy +H[1][2] *ds;
				xc = H[2][0] * dx + H[2][1] * dy +H[2][2] *ds;
				
				xi = (xi > 0 ? xi :xi*(-1.0));
				xr = (xr > 0 ? xr :xr*(-1.0));
				xc = (xc > 0 ? xc :xc*(-1.0));
				scale = R_w[m] / R_w[t];
				r = Pos2[0].x / t_width;
				c = Pos2[0].x % t_width;
				index = (scale * r) * R_w[m] + (scale * c);
				switch(m)
				{
					case 0:
						laplacian = lay1[1][index];
						break;
					case 1:
						laplacian = lay2[1][index];
						break;
					case 2:
						laplacian = lay3[1][index];
						break;
					case 3:
						laplacian = lay4[1][index];
						break;
					case 4:
						laplacian = lay5[1][index];
						break;
					case 5:
						laplacian = lay6[1][index];
						break;
					case 6:
						laplacian = lay7[1][index];
						break;
					case 7:
						laplacian = lay8[1][index];
						break;
					case 8:
						laplacian = lay9[1][index];
						break;
					case 9:
						laplacian = lay10[1][index];
						break;
					case 10:
						laplacian = lay11[1][index];
						break;
					case 11:
						laplacian = lay12[1][index];
						break;		
				}
				if( xi  < 0.5  &&   xr  < 0.5  &&   xc < 0.5 ){
					Out[0].x = (c + xc) * t_step;
					Out[0].y = (r + xr) * t_step;
					Out[0].scale = (0.1333f) * (R_filter[m] + xi * filterStep);
					Out[0].laplacian = laplacian;
				}else{
					Out[0].x = -1;
					Out[0].y = -1;
					Out[0].scale = -1;
					Out[0].laplacian = -1;
				}
			}
		}
		window
		{
			Pos2 sliding(1,1);
			Flag sliding(1,1);
			Responses sliding(19,19);
			Out tumbling(1);
		}
	};
}

composite interpolateExtremum(output stream<double x,double y,double scale,double laplacian>Out,input stream<int x>In)
{
	param
		int b, int m, int t,
		double lay1[2][R1],double lay2[2][R1],double lay3[2][R1],double lay4[2][R1],double lay5[2][R5],double lay6[2][R5],
		double lay7[2][R7],double lay8[2][R7],double lay9[2][R9],double lay10[2][R9],double lay11[2][R11],double lay12[2][R11];;
	//interpolate 得到的数据 依次为 点的坐标信息以及xi、xr、xc的值
	Out = pipeline(In){
	// 获得坐标和  标记
		add splitjoin{   
			split duplicate();
			add dump();
			add	isExtremum(b,m,t,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);	
			join roundrobin();	
		};
		add interpolateStep(b,m,t,lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
	};
		
}
composite getHaarArg1(output stream<double x>Out,input stream<double x,double y,double scale,
		double laplacian>In)
{
	param
		int j,int i;
	Out = getHaarArg1(In){
		work
		{
			int r,c,s;
			r = floor(In[0].y + 0.5);
			c = floor(In[0].x + 0.5);
			s = floor(In[0].scale + 0.5);
			Out[0].x = r + j*s;
			Out[1].x = c + i*s;
			Out[2].x = 4*s;
 		}
		window
		{
			In sliding(1,1);
			Out tumbling(3);
		}
	};
}
composite Haar2Boxarg(output stream<double x>Out,input stream<double x>In)
{
	Out = Haar2Boxarg(In){
		work
		{
			int row, column, s;
			row = In[0].x;
			column = In[1].x;
			s = In[2].x;
			Out[0].x = row - s/2;
			Out[1].x = column;
			Out[2].x = s;
			Out[3].x = s/2;
			Out[4].x = row - s/2;
			Out[5].x = column - s/2;
			Out[6].x = s;
			Out[7].x = s/2;
			Out[8].x = row;
			Out[9].x = column - s/2;
			Out[10].x = s/2;
			Out[11].x = s;
			Out[12].x = row - s/2;
			Out[13].x = column-s/2;
			Out[14].x = s/2;
			Out[15].x = s;
		}
		window
		{
			In sliding(3,3);
			Out tumbling(16);
		}
	};
}
composite getHaar(output stream<double haarX,double haarY>Out,input stream<double x>In)
{
	stream<double x>BoxValue;
	BoxValue = pipeline(In){
		add Haar2Boxarg();
		add splitjoin{
			int i;
			split roundrobin(4,4,4,4);
			for(i=0;i<4;i++)
				add BoxIntegral();
			join roundrobin();
		};
	};
	Out = getHaar(BoxValue){
		work
		{
			Out[0].haarX = BoxValue[0].x - BoxValue[1].x;
			Out[0].haarY = BoxValue[2].x - BoxValue[3].x;
		}
		window
		{
			BoxValue sliding(4,4);
			Out tumbling(1);
		}
	};
}
composite getHaarRes(output stream<double x,double y,double angle>Out,input stream<double haarX,double haarY>In)
{
	param
		double gauss;
	Out = getHaarRes(In)
	{
		work
		{
			double pi = 3.14159;
			double x = gauss * In[0].haarX;
			double y = gauss * In[0].haarY;
			Out[0].x = x;
			Out[0].y = y;
			if(x > 0 && y >= 0)
				Out[0].angle = atan(y/x);
			else if(x < 0 && y >= 0)
				Out[0].angle = pi - atan(-y/x);
			else if(x < 0 && y < 0)
				Out[0].angle =  pi + atan(y/x);
			else if(x > 0 && y < 0)
				Out[0].angle =  2*pi - atan(-y/x); 
		}
	};
}

composite getResTmp(output stream<double x,double y,double angle>Out,input stream<double x,double y,double scale,
		double laplacian>In)
{
	param
		int j,int i,double gauss;
	
	Out = pipeline(In){
		add getHaarArg1(j,i);
		add getHaar();
		add getHaarRes(gauss);
	};
}


composite getRes(output stream<double x,double y,double angle>Out,input stream<double x,double y,double scale,
		double laplacian>In)
{
	 const int id[] = {6,5,4,3,2,1,0,1,2,3,4,5,6};
	 const double gauss25 [7][7] = {
	  0.02546481,	0.02350698,	0.01849125,	0.01239505,	0.00708017,	0.00344629,	0.00142946,
	  0.02350698,	0.02169968,	0.01706957,	0.01144208,	0.00653582,	0.00318132,	0.00131956,
	  0.01849125,	0.01706957,	0.01342740,	0.00900066,	0.00514126,	0.00250252,	0.00103800,
	  0.01239505,	0.01144208,	0.00900066,	0.00603332,	0.00344629,	0.00167749,	0.00069579,
	  0.00708017,	0.00653582,	0.00514126,	0.00344629,	0.00196855,	0.00095820,	0.00039744,
	  0.00344629,	0.00318132,	0.00250252,	0.00167749,	0.00095820,	0.00046640,	0.00019346,
	  0.00142946,	0.00131956,	0.00103800,	0.00069579,	0.00039744,	0.00019346,	0.00008024
	};
	 
	 Out = splitjoin(In){
	 	int i,j;
		double gauss;
		split duplicate();
		for(i=-6;i<6;i++){
			for(j=-6;j<6;j++){
				if(i*i + j*j < 36){
					gauss = gauss25[id[i+6]][id[j+6]];  
					 // could use abs() rather than id lookup, but this way is faster
					// add pipeline{
						// add getHaarArg1(j,i);
						// add getHaar();
						// add getHaarRes(gauss);
					// };
					add getResTmp(j,i,gauss);
				}
			}
		}
		join roundrobin();
	 };
}

composite getSum(output stream<double sumX,double sumY>Out, input stream<double x,double y,double angle>In)
{
	param
		double ang1;
	
	const double pi = 3.14159;
	double ang2 = ( ang1+pi/3.0f > 2*pi ? ang1-5.0f*pi/3.0f : ang1+pi/3.0f);
	
	Out = getSum(In){
		work
		{
			unsigned int k=0;
			double ang;
			double sumX=0.0,sumY=0.0;
			for(;k<109;k++){
				ang = In[0].angle;
				if (ang1 < ang2 && ang1 < ang && ang < ang2) 
				{
					sumX+=In[k].x;  
					sumY+=In[k].y;
				} 
				else if (ang2 < ang1 &&((ang > 0 && ang < ang2) || (ang > ang1 && ang < 2*pi) )) 
				{
					sumX+=In[k].x;  
					sumY+=In[k].y;
				}
			}
			Out[0].sumX = sumX;
			Out[0].sumY = sumY;
		}
		window
		{
			In sliding(109,109);
			Out tumbling(1);
		}
	};
}
composite getMax(output stream<double x>Out, input stream<double sumX,double sumY>In)
{
	Out = getMax(In){
		work
		{
			double max=0.0;
			double orientation;
			int i;
			for(i=0;i<42;i++){
				if (In[i].sumX*In[i].sumX + In[i].sumY*In[i].sumY > max) 
				{
				  // store largest orientation
				  max = In[i].sumX*In[i].sumX + In[i].sumY*In[i].sumY;
				  orientation = getAngle(In[i].sumX, In[i].sumY);
				}
			}
			Out[0].x = orientation;
		}
		window
		{
			In sliding(42,42);
			Out tumbling(1);
		}
	};
}
composite getOrientation(output stream<double x,double y,double scale,double laplacian,double orientation>Out,
						 input stream<double x,double y,double scale,double laplacian>In)
{
	
	stream<double x,double y,double scale,double laplacian> data1,data2;
	stream<double x> orientation;
	const double pi = 3.14159;
	
	(data1,data2) = separate(In){
		work
		{
			data1[0].x = In[0].x;
			data1[0].y = In[0].y;
			data1[0].scale = In[0].scale;
			data1[0].laplacian = In[0].laplacian;
			data2[0].x = In[0].x;
			data2[0].y = In[0].y;
			data2[0].scale = In[0].scale;
			data2[0].laplacian = In[0].laplacian;
		}
		window
		{
			In sliding(1,1);
		}
	};
	orientation = pipeline(data1){
		add getRes();
		add splitjoin{
			double ang1;
			split duplicate();
			 for(ang1 = 0; ang1 < 2*pi;  ang1+=0.15f) { // 循环42次
				add getSum(ang1);
			 }
			 join roundrobin();
		};
		add getMax();
	};
	Out = getOrientation(data2,orientation){
		work
		{
			Out[0].x = data2[0].x;
			Out[0].y = data2[0].y;
			Out[0].scale = data2[0].x;
			Out[0].laplacian = data2[0].laplacian;
			Out[0].orientation = orientation[0].x;
		}
	};
}
composite getHaarArg2(output stream<double x>Out,input stream<double x,double y,double scale,double laplacian,double orientation>In)
{
	param
		int l,int k;
	Out = getHaarArg2(In){
		work
		{
			double x=In[0].x, y=In[0].y, scale = In[0].scale;
			double orientation = In[0].orientation;
			double si = sin(orientation),co = cos(orientation);
			int sample_x,sample_y;
			sample_x = floor(x + (-l*scale*si + k*scale*co) + 0.5);
			sample_y = floor(y + ( l*scale*co + k*scale*si) + 0.5);
			Out[0].x = sample_y;
			Out[1].x = sample_x;
			Out[2].x = 2 * floor(scale + 0.5);
		}
		window
		{
			In sliding(1,1);
			Out tumbling(3);
		}
	};
}
//stream成员不能是数组
//69个
composite getDesc(output stream<double x>Out,input stream<double x,double y,double scale,double laplacian,double orientation>In)
{
	param
		int i,int j,double cx,double cy;
	
	int ix =i+5,jx = j+5;
	const int pi = 3.14159;
	stream<double x,double y,double scale,double laplacian,double orientation>data1,data2;
	stream<double haarX,double haarY>haarValue;
	(data1,data2) = separate(In){
		work
		{
			data1[0].x = In[0].x;
			data1[0].y = In[0].y;
			data1[0].scale = In[0].scale;
			data1[0].laplacian = In[0].laplacian;
			data1[0].orientation = In[0].orientation;
			data2[0].x = In[0].x;
			data2[0].y = In[0].y;
			data2[0].scale = In[0].scale;
			data2[0].laplacian = In[0].laplacian;
			data2[0].orientation = In[0].orientation;
		}
	};
	haarValue = splitjoin(data1){
		int k,l;
		split duplicate();
		for(k=i;k<i+9;++k){
			for(l=j;l<j+9;++l){
				add getHaarArg2(l,k);
				add getHaar();
			}
		}
		join roundrobin();
	};
	Out = getDesc(data2,haarValue){
		work
		{
			double scale, dx, dy, mdx, mdy, co, si,gauss_s1,gauss_s2,g_s,rrx,rry;
			int index=0,x,y,k,l,xs,ys,sample_x,sample_y,rx,ry,g_x,g_y;
			
			x = data2[0].x;
			y = data2[0].y;
			scale = data2[0].scale;
			co = cos(data2[0].orientation);
			si = sin(data2[0].orientation);
			xs = floor(x + ( -jx*scale*si + ix*scale*co) + 0.5);
			ys = floor(y + ( jx*scale*co + ix*scale*si) + 0.5);
			dx = dy = mdx = mdy =0;
			for(k=i;k<i+9;++k){
				for(l=j;l<j+9;++l){
					//Get coords of sample point on the rotated axis
					sample_x = floor(x + (-l*scale*si + k*scale*co) + 0.5);
					sample_y = floor(y + ( l*scale*co + k*scale*si) + 0.5);
					rx = haarValue[index].haarX;
					ry = haarValue[index++].haarY;
					g_x = xs-sample_x;
					g_y = ys-sample_y;
					g_s = 2.5f*scale;
					gauss_s1 =  GS(g_x,g_y,g_s);
					
					//Get the gaussian weighted x and y responses on rotated axis
					rrx = gauss_s1*(-rx*si + ry*co);
					rry = gauss_s1*(rx*co + ry*si);

					dx += rrx;
					dy += rry;
					mdx += fabs(rrx);
					mdy += fabs(rry);
				}
			}
			//Add the values to the descriptor vector
			gauss_s2 = GS(cx-2.0,cy-2.0,1.5);
			
			Out[0].x = dx*gauss_s2;
			Out[1].x = dy*gauss_s2;
			Out[2].x = mdx*gauss_s2;
			Out[3].x = mdy*gauss_s2;
			Out[4].x = (dx*dx + dy*dy + mdx*mdx + mdy*mdy) * gauss_s2*gauss_s2;
		}
		window
		{
			data2 sliding(1,1);
			haarValue sliding(81,81);
			Out tumbling(5);
		}
	};
}
composite getDescriptor(output stream<double x>Out,input stream<double x,double y,double scale,double laplacian,double orientation>In)
{
	stream<double x> Desc;
	stream<double x,double y,double scale,double laplacian,double orientation>data1,data2;
	(data1,data2) = separate(In){
		work
		{
			data1[0].x = In[0].x;
			data1[0].y = In[0].y;
			data1[0].scale = In[0].scale;
			data1[0].laplacian = In[0].laplacian;
			data1[0].orientation = In[0].orientation;
			data2[0].x = In[0].x;
			data2[0].y = In[0].y;
			data2[0].scale = In[0].scale;
			data2[0].laplacian = In[0].laplacian;
			data2[0].orientation = In[0].orientation;
		}
	};
	Desc = splitjoin(data1){
		int i,j;
		double  cx = -0.5, cy = 0.0; //Subregion centers for the 4x4 gaussian weighting
		split duplicate();
		for(i=-8;i<12;i += 9){
			j = -8;
			i = i-4;
			cx += 1.f;
			cy = -0.5f;
			for(;j<12;j += 9){
				cy += 1.f;
				j = j -4;
				add getDesc(i,j,cx,cy);
			}
		}
		join roundrobin(5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5); //前4个为desc信息 第5个是len
	};
	Out = getDescriptor(data2,Desc){
		work
		{
			int i,j=0,len=0;
			Out[j++].x = data2[0].x;
			Out[j++].x = data2[0].y;
			Out[j++].x = data2[0].scale;
			Out[j++].x = data2[0].laplacian;
			Out[j++].x = data2[0].orientation;

			for(i=0;i<80;i++)
				if((i+1)%5 == 0)
					len += Desc[i].x;
			for(i=0;i<80;i++)
				if((i+1)%5 != 0)
					Out[j++].x = Desc[i].x / len;
		}
		window
		{
			data2 sliding(1,1);
			Desc sliding(80,80);
			Out tumbling(69);
		}
	};
}
composite getIpoint(output stream<double x>Out, input stream<int x>In)
{
	param
		double lay1[2][R1],double lay2[2][R1],double lay3[2][R1],double lay4[2][R1],double lay5[2][R5],double lay6[2][R5],
		double lay7[2][R7],double lay8[2][R7],double lay9[2][R9],double lay10[2][R9],double lay11[2][R11],double lay12[2][R11];
	
	const int octaves=5;
	const int init_sample=2;
	const int intervals = 4;
	// filter index map
	const int filter_map [5][4] = {{0,1,2,3}, {1,3,4,5}, {3,5,6,7}, {5,7,8,9}, {7,9,10,11}};
	
	int b[10], m[10],t[10]; // 空间金字塔的底、中、上层
	int i,k,o;
	
	for(k=0,o = 0; o < octaves; ++o) 
		for (i = 0; i <= 1; ++i,k++)
		{
			b[k] = filter_map[o][i];
			m[k] = filter_map[o][i+1];
			t[k] = filter_map[o][i+2];	
		}
	
	Out = splitjoin(In)
	{
		int i;
		split duplicate();
		for(i=0;i<10;i++)
		{
			add interpolateExtremum(b[i],m[i],t[i],lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
			add getOrientation();
			add getDescriptor();
		}
		join roundrobin();
	};
}

composite getSource(output stream<int x> Out)
{
	double lay1[2][R1],lay2[2][R1],lay3[2][R1],lay4[2][R1],lay5[2][R5],lay6[2][R5];
	double lay7[2][R7],lay8[2][R7],lay9[2][R9],lay10[2][R9],lay11[2][R11],lay12[2][R11];
	
	int i,j;
	
	int step,b,l,w,r,c,ar,ac,index;
	
	double inverse_area; 
	
	double Dxx, Dyy, Dxy;
	//记录每层的点信息
	double tmp_res;
	double tmp_lap;
	int r1[8],c1[8],r2[8],c2[8];
	int ir1,ir2,ic1,ic2;
	double box[8];
	double A=0.0,B=0.0,C=0.0,D=0.0;
	int steps = (int)(WIDTHSTEP /4);
	
	// 构造12个响应层(ResponseLayer)
	
	
	stream<double x> Source;
	for(i=0;i<12;++i)
	{
		step = R_step[i];                      // step size for this filter
		b = (R_filter[i] - 1) / 2;             // border for this filter
		l = R_filter[i] / 3;                   // lobe for this filter (filter size / 3)
		w = R_filter[i];                       // filter size
		inverse_area = 1.f/(w*w);           // normalisation factor
		for(ar = 0, index = 0; ar < R_h[i]; ++ar) 
		{
			for(ac = 0; ac < R_w[i]; ++ac, index++) 
			{
			  // get the image coordinates
				r = ar * step;
				c = ac * step; 
				// Compute response components
				r1[0] =  r - l + 1;r1[1]=r - l + 1;r1[2]=r - b;r1[3]=r - l/2;r1[4]=r - l;r1[6]=r - l;r1[5]=r + 1;r1[7]=r + 1;
				c1[0] = c - b;c1[1]= c - l/2;c1[2]=c - l + 1;c1[3] = c - l + 1;c1[4]=c + 1;c1[5]=c - l;c1[6]=c - l;c1[7]=c + 1;
				r2[0] =  2*l - 1;r2[1] =2*l - 1;r2[2] = w;r2[3] =l;r2[4] =l;r2[5] =l;r2[6] =l;r2[7] = l;
				c2[0] = w;c2[1] = l;c2[2] = 2*l - 1;c2[3] = 2*l - 1;c2[4] = l;c2[5] =l; c2[6] = l;c2[7] = l;
				for(j=0;j<8;j++)
				{
					if(r1[j] <HEIGHT)
						ir1 = r1[j]-1;
					else
						ir1 = HEIGHT-1;
					if(c1[j] <WIDTH )
						ic1 = c1[j]-1;
					else
						ic1 = WIDTH-1;
					if(r1[j]+r2[j] <HEIGHT)
						ir2 = r1[j]+r2[j]-1;
					else
						ir2 = HEIGHT-1;	
					if(c1[j]+c2[j] <WIDTH)
						ic2 = c1[j]+c2[j]-1;
					else
						ic2 = WIDTH-1;
					
					if(ir1 >= 0 && ic1 >= 0) 
						A = int_img[ir1 * steps + ic1];
					if(ir1 >= 0 && ic2 >= 0) 
						B = int_img[ir1 * steps + ic2];
					if(ir2 >= 0 && ic1 >= 0) 
						C = int_img[ir2 * steps + ic1];
					if(ir2 >= 0 && ic2 >= 0) 
						D = int_img[ir2 * steps + ic2];
					if((A-B-C+D) > 0)
						box[j] = A - B - C + D;
					else
						box[j] = 0.0;
				}
				Dxx = box[0] - box[1] *3;
				Dyy = box[2] - box[3] *3;
				Dxy = box[4] + box[5] - box[6] - box[7];
				Dxx *= inverse_area;
				Dyy *= inverse_area;
				Dxy *= inverse_area;
				// Get the determinant of hessian response & laplacian sign
				tmp_res = Dxx * Dyy - 0.81 * Dxy * Dxy;
				if(Dxx + Dyy >=0)
					tmp_lap = 1.0;
				else
					tmp_lap =0.0;
				if(i==0)
				{
					lay1[0][index] = tmp_res;
					lay1[1][index] = tmp_lap;
				} else if(i==1){
						lay2[0][index] = tmp_res;
						lay2[1][index] = tmp_lap;
				} else if(i==2){
						lay3[0][index] = tmp_res;
						lay3[1][index] = tmp_lap;
				} else if(i==3){
						lay4[0][index] = tmp_res;
						lay4[1][index] = tmp_lap;
				} else if(i==4){
						lay5[0][index] = tmp_res;
						lay5[1][index] = tmp_lap;
				} else if(i==5){
						lay6[0][index] = tmp_res;
						lay6[1][index] = tmp_lap;
				} else if(i==6){
						lay7[0][index] = tmp_res;
						lay7[1][index] = tmp_lap;
				} else if(i==7){
						lay8[0][index] = tmp_res;
						lay8[1][index] = tmp_lap;
				} else if(i==8){
						lay9[0][index] = tmp_res;
						lay9[1][index] = tmp_lap;
				} else if(i==9){
						lay10[0][index] = tmp_res;
						lay10[1][index] = tmp_lap;
				} else if(i==10){
						lay11[0][index] = tmp_res;
						lay11[1][index] = tmp_lap;
				} else {
						lay12[0][index] = tmp_res;
						lay12[1][index] = tmp_lap;		
				}
			}
		}
	}
	//12个响应层构建完毕
	Source = getSource(){
		int i=0;
		work
		{
			Out[0].x = i;
			i++;
		}
	};
	Out = getIpoint(Source)(lay1,lay2,lay3,lay4,lay5,lay6,lay7,lay8,lay9,lay10,lay11,lay12);
}
composite Main()
{
	stream<double x> Ipts;
	//Ipts 每组成员 含有69个数据

	Ipts = getSource()();
	Sink(Ipts)
	{
		work
		{
			int i;
			for(i=0;i<69;i++)
				println(Ipts[i].x);
		}
	};
}
