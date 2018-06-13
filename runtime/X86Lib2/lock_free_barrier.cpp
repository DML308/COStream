#include <stdlib.h>
volatile char *barrierBuffer;
void masterSync(int n)
{
	int i,sum;
	do
	{
		for(i=1,sum=1;i<n;i++)
			sum	+= barrierBuffer[i];
	}
	while(sum<n);
	for(i=1; i<n; i++){
		barrierBuffer[i] = 0;
	}
}
void workerSync(const int tid)
{
	barrierBuffer[tid] = 1;
	while(barrierBuffer[tid]);		//wait for the masterSync to set it to 0
	
}
void allocBarrier(int n)
{
  int t;
  barrierBuffer = (volatile char *)malloc(sizeof(volatile char));
  for(t = 0; t < n; t++)
    barrierBuffer[t] = 0;
}
