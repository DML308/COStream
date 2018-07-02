extern volatile char *barrierBuffer;
void masterSync(int n);
void workerSync(const int);
void allocBarrier(int n);
