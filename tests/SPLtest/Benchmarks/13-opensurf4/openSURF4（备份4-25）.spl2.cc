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

#define BOXVALUE(r,c,rs,cs,boxvalue) do{\
int step = (int)(WIDTHSTEP/4); \
int r1=(int)(r-1),c1=(int)(c-1),r2=(int)(r+rs-1),c2=(int)(c+cs-1); \
double A=0.0,B=0.0,C=0.0,D=0.0; \
if(HEIGHT-1 < r1) \
	r1 = HEIGHT -1 ; \
if(WIDTH-1 < c1) \
	c1 = WIDTH - 1; \
if(HEIGHT-1 < r2) \
	r2 = HEIGHT -1;\
if(WIDTH-1 <  c2) \
	c2 = WIDTH -1; \
if (r1 >= 0 && c1 >= 0) A = int_img[r1 * step + c1]; \
if (r1 >= 0 && c2 >= 0) B = int_img[r1 * step + c2]; \
if (r2 >= 0 && c1 >= 0) C = int_img[r2 * step + c1]; \
if (r2 >= 0 && c2 >= 0) D = int_img[r2 * step + c2]; \
boxvalue = A-B-C+D; \
if(boxvalue < 0) \
	boxvalue = 0;\
}while(0)

#define HAARX(r,c,s,rs) do{ \
double boxvalue[2]; \
BOXVALUE(r-(s)/2,c,s,(s)/2,boxvalue[0]); \
BOXVALUE(r-(s)/2,c-(s)/2,s,(s)/2,boxvalue[1]); \
rs = boxvalue[0] - boxvalue[1]; \
}while(0)

#define HAARY(r,c,s,rs) do{ \
double boxvalue[2]; \
BOXVALUE(r,c-(s)/2,(s)/2,s,boxvalue[0]); \
BOXVALUE(r-(s)/2,c-(s)/2,(s)/2,s,boxvalue[1]); \
rs = boxvalue[0] - boxvalue[1]; \
}while(0)

#define GETANGLE(x,y,res) do{ \
  double pi =3.14159; \
  if(x > 0 && y >= 0) \
    res = atan(y/x); \
  if(x < 0 && y >= 0) \
    res = pi - atan(-y/x); \
  if(x < 0 && y < 0) \
    res = pi + atan(y/x); \
  if(x > 0 && y < 0) \
    res = 2*pi - atan(-y/x); \
}while(0)

#define GETRES(src,index,type,res) do{\
int r = (index / R_w[src]) * R_step[src]; \
int c = (index % R_w[src]) * R_step[src]; \
int rs_b = (R_filter[src] - 1) / 2; \
int l = (R_filter[src]) / 3;  \
int w = R_filter[src];\
double box[8]; \
double Dxx,Dyy,Dxy; \
double inverse_area = 1.0/(w*w);\
BOXVALUE(r - l + 1, c - rs_b,     2*l - 1, w,       box[0]);\
BOXVALUE(r - l + 1, c - l / 2, 2*l - 1, l,       box[1]);\
BOXVALUE(r - rs_b,     c - l + 1, w,       2*l - 1, box[2]);\
BOXVALUE(r - l / 2, c - l + 1, l,       2*l - 1, box[3]);\
BOXVALUE(r - l,c + 1,l,l,box[4]);\
BOXVALUE(r + 1,c - l,l,l,box[5]);\
BOXVALUE(r - l,c - l,l,l,box[6]);\
BOXVALUE(r + 1,c + 1,l,l,box[7]);\
Dxx = box[0] - box[1] *3;\
Dyy = box[2] - box[3] *3;\
Dxy = box[4] + box[5] - box[6] - box[7];\
Dxx *= inverse_area;\
Dyy *= inverse_area;\
Dxy *= inverse_area;\
if(type==0) \
	res = Dxx * Dyy - 0.81 * Dxy * Dxy;\
else if(Dxx + Dyy >=0) \
	res = 1.0; \
else \
	res =0.0; \
} while(0)


// Get image attributes	
const int R_filter[12] = {9,15,21,27,39,51,75,99,147,195,291,387};
const  int init_sample = 2;
const  double thresh = 0.0004;
const int R_w[12] = {GW,GW,GW,GW,GW/2,GW/2,GW/4,GW/4,GW/8,GW/8,GW/16,GW/16};
const int R_h[12] = {GH,GH,GH,GH,GH/2,GH/2,GH/4,GH/4,GH/8,GH/8,GH/16,GH/16};
const int R_step[12] = {2,2,2,2,4,4,8,8,16,16,32,32};

double int_img[WIDTH*HEIGHT] = {
0.964706,
1.92549,
2.89412,
3.87451,
4.82745,
5.79608,
6.76471,
7.73333,
8.71373,
9.65882,
10.6431,
11.5961,
12.5529,
13.5216,
14.4902,
15.451,
16.4118,
17.3882,
18.3569,
19.3216,
20.2784,
21.2392,
22.2,
23.1647,
24.1333,
25.0902,
26.0431,
27.0275,
28,
28.9569,
29.9373,
30.898,
31.8706,
32.8353,
33.8157,
34.7882,
35.7451,
36.7255,
37.6941,
38.6667,
1.73333,
3.4902,
5.21961,
6.97647,
8.70196,
10.4627,
12.1961,
13.9608,
15.7216,
17.4667,
19.2235,
20.9608,
22.6941,
24.4588,
26.2235,
27.9608,
29.7412,
31.451,
33.1922,
34.9608,
36.7177,
38.4588,
40.2157,
41.9686,
43.7216,
45.4745,
47.2118,
48.9686,
50.7255,
52.4784,
54.2235,
55.9961,
57.7686,
59.498,
61.251,
63.0039,
64.749,
66.4863,
68.2471,
69.9961,
2.54118,
4.80784,
6.94118,
9.18431,
11.4588,
13.6392,
15.8314,
18.0902,
20.302,
22.5451,
24.7922,
27.0118,
29.2784,
31.5255,
33.8118,
36.0275,
38.3647,
40.6118,
42.8902,
45.1294,
47.4196,
49.7059,
51.9922,
54.2941,
56.6,
58.9098,
61.2079,
63.5451,
65.8745,
68.2,
70.4824,
72.8235,
75.1569,
77.4431,
79.7726,
82.0981,
84.4549,
86.753,
89.1451,
91.4863,
3.29804,
6,
8.26275,
10.6471,
12.9961,
15.3373,
17.7059,
20.0275,
22.4118,
24.7647,
27.0588,
29.4549,
31.8078,
34.1137,
36.5529,
38.898,
41.2863,
43.6392,
46.0588,
48.3961,
50.8863,
53.2353,
55.6275,
58.0745,
60.4824,
62.9765,
65.3569,
67.8275,
70.2784,
72.6863,
75.1373,
77.5529,
79.9922,
82.4118,
84.8863,
87.2902,
89.7686,
92.1647,
94.6941,
97.1373,
4.09804,
7.2902,
9.65882,
12.1451,
14.6157,
17.0941,
19.5843,
21.9608,
24.4706,
26.949,
29.3608,
31.8627,
34.3726,
36.7922,
39.349,
41.7686,
44.2588,
46.7216,
49.2118,
51.6314,
54.1961,
56.6628,
59.1843,
61.6745,
64.1882,
66.8118,
69.302,
71.902,
74.502,
77,
79.5608,
82.1216,
84.6902,
87.2079,
89.7922,
92.2431,
94.8314,
97.3608,
99.9804,
102.506,
4.86275,
8.5451,
11.0353,
13.8392,
17.2706,
20.549,
23.749,
26.9569,
30.2,
33.4627,
36.4941,
39.5412,
42.7529,
45.6863,
48.7961,
51.7961,
54.7373,
57.6196,
60.4745,
63.0235,
65.898,
68.6667,
71.7686,
74.6039,
77.3647,
80.1059,
83.1882,
86.2471,
89.2275,
92.102,
95.1177,
98.0549,
100.882,
103.49,
106.196,
108.792,
111.608,
114.384,
117.169,
119.824,
5.66667,
9.82745,
12.4157,
15.5569,
19.9059,
24.1098,
28.051,
31.7608,
35.6118,
39.2824,
42.7726,
46.6902,
50.2627,
53.7529,
57.3843,
60.949,
64.5765,
67.9255,
71.0118,
73.8784,
76.9843,
80.4314,
84.0628,
87.5333,
90.4471,
93.753,
97.2353,
101.027,
104.161,
107.486,
111.118,
114.71,
117.922,
120.584,
123.365,
126.063,
129.008,
131.918,
134.843,
137.533,
6.43922,
11.1216,
13.8078,
17.2431,
22.5647,
27.6706,
32.2824,
36.5059,
40.5373,
44.9059,
48.6628,
53.5059,
57.3686,
61.4196,
65.5686,
69.6353,
73.9922,
77.898,
81.2745,
84.498,
87.8,
91.5726,
95.8863,
100.063,
103.275,
106.71,
110.929,
115.427,
118.914,
122.471,
126.624,
130.949,
134.745,
137.565,
140.475,
143.275,
146.298,
149.322,
152.529,
155.341,
7.21961,
12.3804,
15.2118,
18.9726,
25.1961,
31.2078,
36.6196,
41.4078,
45.9451,
50.8275,
55.5177,
60.6,
64.7294,
69.1843,
73.8588,
78.3608,
83.3137,
87.7294,
91.5333,
95.1647,
98.7177,
102.788,
107.976,
112.659,
116.063,
119.667,
124.627,
129.847,
133.651,
137.659,
142.275,
147.435,
151.635,
154.773,
157.812,
160.702,
163.882,
167.235,
170.894,
173.969,
8.00784,
13.6824,
16.6235,
20.702,
27.8549,
34.7255,
40.7294,
45.7608,
51.0275,
56.5608,
61.5843,
67.6196,
72.0784,
76.851,
81.8784,
86.6902,
92.1333,
97.102,
101.392,
105.463,
109.416,
114.035,
119.502,
124.322,
128.09,
132.29,
137.635,
142.91,
146.957,
151.392,
156.486,
161.871,
166.161,
169.475,
172.671,
175.659,
178.918,
182.392,
186.235,
189.486,
8.78824,
14.9529,
18.0431,
22.3726,
30.451,
38.1373,
44.7098,
50.2863,
56.4431,
62.3059,
67.902,
74.0588,
78.7216,
83.7255,
89.2314,
94.8431,
100.596,
106.165,
110.875,
115.212,
119.396,
124.247,
129.976,
135.059,
139.118,
143.482,
149.063,
154.604,
158.933,
163.588,
168.898,
174.514,
179.075,
182.635,
186.133,
189.408,
192.922,
196.682,
200.796,
204.29,
9.58824,
16.2588,
19.4588,
24.0784,
33.0549,
41.3216,
48.4118,
54.251,
60.9686,
67.5569,
73.7647,
80.5882,
86.0549,
91.9804,
98.3098,
104.286,
110.412,
116.306,
121.259,
125.918,
130.51,
135.788,
141.941,
147.486,
151.98,
156.812,
162.777,
168.765,
173.514,
178.58,
184.302,
190.337,
195.365,
199.31,
203.231,
206.929,
210.843,
215.047,
219.6,
223.506,
10.349,
17.5608,
20.898,
25.7843,
35.5059,
44.6549,
52.3177,
58.5216,
65.9255,
72.9451,
79.3686,
86.6471,
92.1882,
98.2078,
104.6,
110.914,
117.349,
123.706,
129.196,
134.373,
139.486,
144.984,
151.247,
156.874,
161.514,
166.455,
172.553,
178.69,
183.573,
188.788,
194.663,
200.859,
206,
210.098,
214.161,
217.996,
222.067,
226.427,
231.153,
235.271,
11.149,
18.8706,
22.3098,
27.5177,
37.7686,
47.5059,
55.6628,
62.3216,
69.8667,
77.3804,
83.9804,
91.9294,
98.1333,
104.412,
111.188,
117.788,
124.412,
131.302,
137.255,
142.659,
148,
153.753,
160.192,
166.031,
170.851,
175.992,
182.314,
188.647,
193.737,
199.188,
205.286,
211.702,
217.086,
221.463,
225.773,
229.882,
234.255,
238.918,
243.973,
248.396,
11.9216,
20.1529,
23.6549,
29.0353,
40.0235,
50.3294,
58.9765,
65.902,
74.0941,
81.7961,
88.7804,
97.5137,
104.498,
111.647,
119.094,
125.871,
132.722,
140.161,
146.608,
152.176,
157.702,
163.616,
170.227,
176.282,
181.314,
186.643,
193.169,
199.757,
205.086,
210.761,
217.067,
223.769,
229.427,
234.082,
238.671,
243.09,
247.753,
252.706,
258.094,
262.796,
12.7059,
21.4784,
25.1098,
30.7686,
42.4118,
53.3059,
62.4471,
70.0392,
78.4941,
86.5686,
93.9569,
103.184,
110.714,
118.396,
126.055,
133.055,
140.169,
148.188,
155.086,
160.867,
166.549,
172.686,
179.459,
185.733,
190.957,
196.51,
203.271,
210.059,
215.62,
221.553,
228.149,
235.11,
241.027,
245.984,
250.855,
255.569,
260.565,
265.824,
271.494,
276.478,
13.502,
22.8235,
26.5529,
32.4824,
44.6745,
56.0824,
65.7647,
73.7922,
82.8079,
91.3334,
99.0392,
108.475,
116.255,
124.126,
131.937,
139.192,
146.439,
155.082,
162.467,
168.455,
174.345,
180.671,
187.647,
194.114,
199.553,
205.326,
212.306,
219.322,
225.122,
231.314,
238.184,
245.42,
251.608,
256.871,
262.039,
267.051,
272.337,
277.902,
283.816,
289.055,
14.2824,
24.1529,
27.9804,
34.1882,
46.9647,
58.8941,
69.0667,
77.4941,
86.7922,
95.4314,
103.69,
113.651,
121.886,
130.271,
138.576,
146.306,
154.051,
163.373,
171.212,
177.373,
183.431,
189.957,
197.145,
203.824,
209.475,
215.463,
222.671,
229.933,
236,
242.467,
249.616,
257.129,
263.62,
269.169,
274.675,
279.98,
285.576,
291.459,
297.624,
303.114,
15.0667,
25.451,
29.4431,
35.8157,
49.1765,
61.6784,
72.498,
81.3059,
91.1922,
100.424,
109.357,
119.871,
128.69,
137.635,
146.553,
154.875,
163.192,
173.067,
181.314,
187.694,
193.945,
200.694,
208.094,
215.004,
220.878,
227.094,
234.545,
242.082,
248.439,
255.192,
262.627,
270.431,
277.247,
283.118,
288.918,
294.514,
300.404,
306.533,
312.973,
318.741,
15.8588,
26.7569,
30.902,
37.4824,
51.4706,
64.5804,
75.898,
84.9373,
95.3883,
105.027,
114.498,
125.11,
133.992,
142.984,
152.094,
160.557,
169.024,
179.071,
187.541,
194.145,
200.631,
207.624,
215.22,
222.333,
228.459,
234.926,
242.647,
250.478,
257.133,
264.176,
271.906,
280.02,
287.145,
293.341,
299.4,
305.31,
311.498,
317.941,
324.663,
330.804,
16.6392,
28.0745,
32.4118,
39.2667,
53.9608,
67.5373,
79.1843,
88.7922,
100.078,
110.078,
119.667,
130.357,
139.337,
148.424,
157.761,
166.463,
175.161,
185.451,
194.137,
200.941,
207.643,
214.855,
222.675,
230.031,
236.424,
243.173,
251.2,
259.353,
266.318,
273.655,
281.686,
290.122,
297.549,
304.027,
310.369,
316.592,
323.094,
330.031,
336.961,
343.431,
17.4275,
29.4039,
33.7726,
40.902,
56.2196,
70.4078,
82.3569,
92.7843,
104.576,
114.714,
124.443,
135.243,
144.31,
153.553,
163.129,
172.039,
180.961,
191.529,
200.459,
207.494,
214.443,
221.886,
229.949,
237.565,
244.235,
251.294,
259.659,
268.153,
275.439,
283.078,
291.412,
300.157,
307.867,
314.62,
321.224,
327.749,
334.624,
342.106,
349.31,
356.024,
18.2039,
30.7529,
35.2863,
42.7451,
58.7451,
73.6196,
85.8549,
97.0588,
109.361,
119.604,
129.412,
140.318,
149.51,
159.012,
168.878,
178.094,
187.227,
198.043,
207.212,
214.478,
221.659,
229.322,
237.639,
245.553,
252.537,
259.937,
268.659,
277.498,
285.106,
293.055,
301.686,
310.718,
318.682,
325.702,
332.584,
339.443,
346.776,
354.667,
362.314,
369.302,
18.9882,
32.102,
36.7765,
44.498,
61.1765,
76.6863,
89.1373,
101.224,
114.027,
124.392,
134.325,
145.424,
154.914,
164.529,
174.624,
184.059,
193.51,
204.608,
214.055,
221.6,
229.051,
236.98,
245.576,
253.812,
261.149,
268.922,
278.008,
287.18,
295.098,
303.353,
312.278,
321.58,
329.824,
337.071,
344.337,
351.565,
359.361,
367.443,
375.502,
382.741,
19.7726,
33.4784,
38.2667,
46.2118,
63.5882,
79.6078,
92.3412,
105.326,
118.612,
129.031,
139.271,
150.427,
160.063,
169.867,
180.18,
189.922,
199.639,
210.996,
220.698,
228.506,
236.235,
244.467,
253.396,
261.988,
269.671,
277.831,
287.255,
296.725,
304.953,
313.502,
322.698,
332.255,
340.765,
348.275,
355.816,
363.443,
371.682,
380.012,
388.369,
395.886,
20.5529,
34.8314,
39.7059,
47.8353,
65.8314,
82.4667,
95.4667,
109.345,
123.153,
133.663,
144.271,
155.761,
165.847,
175.973,
186.498,
196.565,
206.576,
218.227,
228.224,
236.326,
244.353,
252.906,
262.188,
271.161,
279.251,
287.773,
297.518,
307.286,
315.824,
324.647,
334.114,
343.922,
352.706,
360.486,
368.4,
376.408,
384.926,
393.98,
402.851,
410.651,
21.3373,
36.1922,
41.1961,
49.553,
68.0588,
85.2353,
98.4549,
113.227,
127.616,
138.188,
149.196,
161.251,
171.953,
182.322,
193.133,
203.58,
213.898,
225.855,
236.157,
244.569,
252.922,
261.827,
271.49,
280.859,
289.357,
298.216,
308.275,
318.345,
327.153,
336.239,
345.957,
356.012,
365.035,
373.086,
381.424,
389.788,
398.592,
407.98,
417.228,
425.169,
22.1373,
37.5608,
42.6824,
51.2588,
70.251,
87.8941,
101.369,
117.047,
131.992,
142.706,
154.322,
166.784,
178.227,
189.012,
200.165,
210.906,
221.553,
233.827,
244.439,
253.173,
261.878,
271.169,
281.224,
290.98,
299.847,
309.047,
319.408,
329.773,
338.827,
348.204,
358.165,
368.447,
377.749,
386.137,
394.812,
403.439,
412.404,
421.98,
431.424,
439.431,
22.9059,
38.9177,
44.1726,
52.9961,
72.5412,
90.6628,
104.341,
120.906,
136.404,
147.267,
159.353,
172.294,
184.31,
195.416,
206.8,
217.906,
228.902,
241.522,
252.478,
261.561,
270.639,
280.326,
290.78,
300.925,
310.153,
319.694,
330.353,
340.988,
350.318,
359.984,
370.188,
380.682,
390.216,
398.741,
407.6,
416.376,
425.478,
435.177,
444.714,
452.835,
23.698,
40.302,
45.6353,
54.6706,
74.702,
93.2431,
106.988,
124.49,
140.557,
151.522,
164.082,
177.514,
190.035,
201.506,
213.122,
224.651,
235.984,
248.965,
260.298,
269.761,
279.224,
289.31,
300.177,
310.729,
320.306,
330.169,
341.145,
352.047,
361.655,
371.529,
381.977,
392.671,
402.278,
410.961,
419.941,
428.82,
438.039,
447.855,
457.478,
465.714,
24.4902,
41.6824,
47.1098,
56.4196,
76.8706,
95.8118,
109.663,
128.086,
144.671,
155.769,
168.62,
182.455,
195.161,
206.878,
218.827,
230.741,
242.424,
255.773,
267.49,
277.345,
287.212,
297.718,
309.004,
319.961,
329.875,
340.047,
351.329,
362.529,
372.376,
382.439,
393.137,
403.965,
413.698,
422.51,
431.545,
440.439,
449.784,
459.745,
469.475,
477.8,
25.2588,
43.0706,
48.6,
58.0353,
78.5255,
97.5882,
111.604,
130.906,
148.075,
159.475,
172.498,
186.62,
199.549,
211.639,
223.929,
236.196,
248.275,
262.02,
274.129,
284.384,
294.678,
305.627,
317.326,
328.651,
338.918,
349.404,
360.937,
372.463,
382.525,
392.859,
403.839,
414.71,
424.545,
433.439,
442.549,
451.553,
460.98,
470.977,
480.871,
489.263
};


composite getResXY(output stream<double x,double y,double angle>Out,input stream<double x,double y,double scale,
		double laplacian>In)
{
	 const double gauss25 [7][7] = {
	  0.02546481,	0.02350698,	0.01849125,	0.01239505,	0.00708017,	0.00344629,	0.00142946,
	  0.02350698,	0.02169968,	0.01706957,	0.01144208,	0.00653582,	0.00318132,	0.00131956,
	  0.01849125,	0.01706957,	0.01342740,	0.00900066,	0.00514126,	0.00250252,	0.00103800,
	  0.01239505,	0.01144208,	0.00900066,	0.00603332,	0.00344629,	0.00167749,	0.00069579,
	  0.00708017,	0.00653582,	0.00514126,	0.00344629,	0.00196855,	0.00095820,	0.00039744,
	  0.00344629,	0.00318132,	0.00250252,	0.00167749,	0.00095820,	0.00046640,	0.00019346,
	  0.00142946,	0.00131956,	0.00103800,	0.00069579,	0.00039744,	0.00019346,	0.00008024
	};
	 
	 Out = getResXY(In){
		work
		{
			int i,j;
			double gauss;
			double rX,rY,angle,resX,resY;
			int r,c,s,idx=0;
			int id[13] = {6,5,4,3,2,1,0,1,2,3,4,5,6};
			if(In[0].x != -1){
				r = (int)floor(In[0].y + 0.5);
				c = (int)floor(In[0].x + 0.5);
				s = (int)floor(In[0].scale + 0.5);
				for(i=-6;i<6;i++){
					for(j=-6;j<6;j++){
						if(i*i + j*j < 36){
							gauss = gauss25[id[i+6]][id[j+6]];  
							HAARX(r+j*s,c+i*s,4*s,rX);
							HAARY(r+j*s,c+i*s,4*s,rY);
							resX = rX * gauss;
							resY = rY * gauss;
							GETANGLE(resX,resY,angle);
							Out[idx].x = resX;
							Out[idx].y = resY;
							Out[idx].angle = angle;
							idx++;
						}
					}
				}
			}else{
				for(i=0;i<109;i++){
					Out[0].x = 0.0;
					Out[0].y = 0.0;
					Out[0].angle = 0.0;
				}
			}
		}
		window
		{
			In sliding(1,1);
			Out tumbling(109);
		}
	 };
}

composite getSum(output stream<double x>Out, input stream<double x,double y,double angle>In)
{
	const double pi = 3.14159;
	
	Out = getSum(In){
		work
		{
			int k;
			double ang,ang1,ang2;
			double sumX=0.0,sumY=0.0;
			double max =0.0;
			double orien;
			if(In[0].x != 0.0 && In[0].y != 0.0){
				for(ang1 = 0; ang1 < 2*pi;  ang1+=0.15f){
					ang2 = ( ang1+pi/3.0f > 2*pi ? ang1-5.0f*pi/3.0f : ang1+pi/3.0f);
					sumX=0.0;
					sumY=0.0;
					for(k=0;k<109;k++){
						ang = In[k].angle;
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
					if (sumX*sumX + sumY*sumY > max) 
					{
						max = sumX*sumX + sumY*sumY;
						GETANGLE(sumX, sumY,orien);
					}
				}
				Out[0].x = orien;
			} else {
				Out[0].x = -1.0;
			}
		}
		window
		{
			In sliding(109,109);
			Out tumbling(1);
		}
	};
}

composite getOrientation(output stream<double x,double y,double scale,double laplacian,double orientation>Out,
						 input stream<double x,double y,double scale,double laplacian>In)
{
	
	stream<double x,double y,double scale,double laplacian> data1,data2;
	stream<double x> orientation;
	
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
		add getResXY();
		add getSum();
	};
	Out = getOrientation(data2,orientation){
		work
		{
			Out[0].x = data2[0].x;
			Out[0].y = data2[0].y;
			Out[0].scale = data2[0].scale;
			Out[0].laplacian = data2[0].laplacian;
			Out[0].orientation = orientation[0].x;
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
	Out = getDesc(In){
		work
		{
			double x,y,scale, haar_s,dx, dy, mdx, mdy, co, si,gauss_s1,gauss_s2,g_s,rrx,rry;
			int k,l,xs,ys,sample_x,sample_y,rx,ry,g_x,g_y;
			
			if(In[0].x != -1){
				x = (int)floor(In[0].x + 0.5);
				y = (int)floor(In[0].y + 0.5);
				scale = In[0].scale;
				haar_s = (int)floor(scale+0.5);
				co = cos(In[0].orientation);
				si = sin(In[0].orientation);
				xs = (int)floor(x -jx*scale*si + ix*scale*co + 0.5);
				ys = (int)floor(y + jx*scale*co + ix*scale*si + 0.5);
				dx = dy = mdx = mdy =0;
				for(k=i;k<i+9;++k){
					for(l=j;l<j+9;++l){
						//Get coords of sample point on the rotated axis
						sample_x = (int)floor(x -l*scale*si + k*scale*co + 0.5);
						sample_y = (int)floor(y + l*scale*co + k*scale*si+ 0.5);
						HAARX(sample_y,sample_x,2.5*haar_s,rx);
						HAARY(sample_y,sample_x,2.5*haar_s,ry);
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
			}else {
				Out[0].x = -1.0;
				Out[1].x = -1.0;
				Out[2].x = -1.0;
				Out[3].x = -1.0;
				Out[4].x = -1.0;
			}
		}
		window
		{
			In sliding(1,1);
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
			i = i-4;
			cx += 1.0;
			cy = -0.5;
			for(j=-8;j<12;j += 9){
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
			int i,j=0;
			float len=0.0;
			Out[j++].x = data2[0].x;
			Out[j++].x = data2[0].y;
			Out[j++].x = data2[0].scale;
			Out[j++].x = data2[0].laplacian;
			Out[j++].x = data2[0].orientation;
			if(data2[0].x != -1){
				for(i=0;i<80;i++)
					if((i+1)%5 == 0)
						len += (int)Desc[i].x;
				len = sqrt(len);
				for(i=0;i<80;i++)
					if((i+1)%5 != 0)
						Out[j++].x = Desc[i].x / len;
			} else {
				for(i=0;i<80;i++)
					if((i+1)%5 != 0)
						Out[j++].x = -5;
			}
		}
		window
		{
			data2 sliding(1,1);
			Desc sliding(80,80);
			Out tumbling(69);
		}
	};
}


composite getExtremum(output stream<int x>Out,input stream<int x>In)
{
	param
		int t,int m,int b;
	
	int layerBorder = (R_filter[t] + 1) / (2 * R_step[t]);
	
	Out = getExtremum(In){
		work
		{
			int flag = 1,scale,index;
			int r=In[0].x / R_w[t];
			int c=In[0].x % R_w[t];
			int rr,cc;
			double candidate,res;
			if (r <= layerBorder || r >= R_h[t] - layerBorder || c <= layerBorder || c >= R_w[t] - layerBorder)
				flag = 0;
			if(flag==1){
				scale = (int)(R_w[m] / R_w[t]);
				index = (scale * r) * R_w[m] + (scale * c);
				GETRES(m,index,0,candidate); // 第3参数为类型值 0表示获得res值 1获得lap值
				if(candidate < thresh)
					flag = 0;
			}
			if(flag==1){
				for(rr=-1;rr<=1;++rr){
					for(cc=-1;cc<=1;++cc){
						index = (r+rr)*R_w[t] + (c+cc);
						GETRES(t,index,0,res);
						if(res >= candidate){
							flag = 0;
							break;
						}
						scale = (int)(R_w[m] / R_w[t]);
						index = (scale * (rr+r)) * R_w[m] + (scale * (c+cc));
						GETRES(m,index,0,res);
						if((rr != 0 || cc != 0) && res >= candidate){
							flag = 0;
							break;
						}
						scale = (int)(R_w[b] / R_w[t]);
						index = (scale * (rr+r)) * R_w[b] + (scale * (c+cc));
						GETRES(b,index,0,res);
						if(res >= candidate){
							flag=0;
							break;
						}
					}
					if(flag == 0)
						break;
				}
			}
			Out[0].x = In[0].x;
			Out[1].x = flag;
		}
		window
		{
			In sliding(1,1);
			Out tumbling(2);
		}
	};
}

composite interpolateStep(output stream<double x,double y,double scale,double laplacian>Out,input stream<int x>In)
{ //输入为flag信息和点坐标信息  每次输入2个数据
	param
		int b, int m, int t;

	int t_width = R_w[t];
	int filterStep = (R_filter[m] - R_filter[b]);
	int t_step = R_step[t];
	
	Out = interpolateStepx(In){
		int r_off[19] = {0,0,1,-1,0,0,0,1,1,-1,-1,0,0,0,0,1,-1,1,-1};
		int c_off[19] = {1,-1,0,0,0,0,0,1,-1,1,-1,1,-1,1,-1,0,0,0,0};
		work
		{
			double rs[19];
			int src[19];
			double dx, dy, ds;
			double v, dxx, dyy, dss, dxy, dxs, dys;
			double rank;
			double H[3][3];
			double xi,xr,xc;
			double lap;
			int scale,index,r,c,i,tmp_s,tmp_w;
			int flag;
			
			flag = In[1].x;
			
			if(flag == 1){
				r = In[0].x / t_width;
				c = In[0].x % t_width;
				src[0]=m;src[1]=m;src[2]=m;src[3]=m;src[4]=t;src[5]=b;
				src[6]=m;src[7]=m;src[8]=m;src[9]=m;src[10]=m;src[11]=t;
				src[12]=t;src[13]=b;src[14]=b;src[15]=t;src[16]=t;
				src[17]=b;src[18]=b;
				
				for(i=0;i<19;i++){
					tmp_w = R_w[src[i]];
					tmp_s = (int)(tmp_w / R_w[t]);
					index = tmp_s*(r + r_off[i]) * tmp_w + (tmp_s * (c + c_off[i]));
					GETRES(src[i],index,0,rs[i]);
				}
				dx = (rs[0] - rs[1]) / 2.0;
				dy = (rs[2] - rs[3]) / 2.0;
				ds = (rs[4] - rs[5]) /2.0;
				v = rs[6];
				dxx = rs[0] + rs[1] - 2*v;
				dyy = rs[2] + rs[3] - 2*v;
				dss = rs[4] + rs[5] - 2*v;
				dxy = ( rs[7] - rs[8] -
					    rs[9] + rs[10] ) / 4.0;
				dxs = ( rs[11] - rs[12] -
					    rs[13] + rs[14] ) / 4.0;
				dys = ( rs[15] - rs[16] -
						rs[17] + rs[18] ) / 4.0;	
				rank = dxx*dyy*dss + dxy*dys*dxs*2 -dxx*dys*dys - dss*dxy*dxy - dyy*dxs*dxs;
				//rank = (rank > 0 ? rank : rank*(-1.0));
				H[0][0] = (dyy*dss-dys*dys)/rank;  H[1][1] = (dxx*dss - dxs*dxs) /rank; H[2][2] = (dxx*dyy - dxy*dxy) /rank;
				H[0][1] = (-1) * (dxy*dss-dxs*dys) / rank;H[1][0] = (-1) * (dxy*dss-dxs*dys) / rank;  
				H[0][2] = (dxy*dys - dyy*dxs) / rank;H[2][0] = (dxy*dys - dyy*dxs) / rank;
				H[1][2] = (-1) * (dxx*dys - dxs*dxy) /rank;H[2][1] = (-1) * (dxx*dys - dxs*dxy) /rank;
				
				xi = H[2][0] * dx + H[2][1] * dy +H[2][2] *ds;
				xr = H[1][0] * dx + H[1][1] * dy +H[1][2] *ds;
				xc = H[0][0] * dx + H[0][1] * dy +H[0][2] *ds;
				xi = xi * (-1);
				xr = xr * (-1);
				xc = xc * (-1);

				if( fabs(xi)  >= 0.5  ||   fabs(xr)  >= 0.5  ||   fabs(xc) >= 0.5 ){
					flag = 0;
				}
			}
			
			if(flag==1){
				scale = R_w[m] / R_w[t];
				index = (scale * r) * R_w[m] + (scale * c);
				GETRES(m,index,1,lap);
				Out[0].x = (c + xc) * t_step;
				Out[0].y = (r + xr) * t_step;
				Out[0].scale = (0.1333f) * (R_filter[m] + xi * filterStep);
				Out[0].laplacian = lap;
			}else{
				Out[0].x = -1.0;
				Out[0].y = -2.0;
				Out[0].scale = -3.0;
				Out[0].laplacian = -4.0;
			}
		}
		window
		{
			In sliding(2,2);
			Out tumbling(1);
		}
	};
}

composite getIptCore(output stream<double x>Out,input stream<int x>In)
{
	param
		int t;
	int m=t-1;
	int b;
	if(t!=2 && t%2==0)
		b = t-3;
	else
		b = t-2;
	
	Out = pipeline(In){
		add getExtremum(t,m,b); // 输出值（2） 为点坐标信息  和flag值
		add interpolateStep(t,m,b);
		add getOrientation();
		add getDescriptor();
	};
}
composite getIpoint(output stream<double x>Out,input stream<int x>In)
{
	param
		int i;
	int j=2*i+2;
	Out = splitjoin(In){
		split duplicate();
		add getIptCore(j);
		add getIptCore(j+1);
		join roundrobin(69,69);	
	};
}
composite dataExpand(output stream<int x>Out, input stream<int x>In)
{
	param
		int expand;
	Out = dataExpand(In){
		work
		{
			int i=0;
			for(i=0;i<expand;i++){
				Out[i].x = In[0].x * expand + i;
			}
		}
		window
		{
			In sliding(1,1);
			Out tumbling(expand);
		}
	};
}
composite SourceExpand(output stream<double x>Out, input stream<int x>In)
{
	int expand[5] = {256,64,16,4,1};
	Out = splitjoin(In)
	{
		int i;
		split duplicate();
		for(i=0;i<5;i++)
		{
			add pipeline{
				add dataExpand(expand[i]);
				add getIpoint(i);
			};
		}
		join roundrobin(256*69,64*69,16*69,4*69,1*69);
	};
}

composite Main()
{
	stream<int x> Source;
	stream<double x> Ipts;
	//Ipts 每组成员 含有69个数据

	Source = getSource(){
		int x=0;
		work
		{
			Source[0].x = x;
			x++;
		}
	};
	Ipts = SourceExpand(Source)();
	Sink(Ipts)
	{
		work
		{
			int i;
			for(i=0;i<69;i++){
				if(Ipts[0].x == -1)
					break;
				println(Ipts[i].x);
				println(" ");
			}
		}
		window
		{
			Ipts sliding(69,69);
		}
	};
}
