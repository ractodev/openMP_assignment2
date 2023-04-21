#define S 100000000
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

typedef struct{double val; char pad[128];} pad_double;

void generate_random(double *input, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    input[i] = rand() / (double)(RAND_MAX);
  }
}

double serial_sum(double *x, size_t size)
{
  double sum_val = 0.0;
  
    for (size_t i = 0; i < size; i++) {
      sum_val += x[i];
    }

  return sum_val;
}

double omp_sum(double *x, size_t size)
{
  double sum_val = 0.0;

  #pragma omp parallel
  {
    #pragma omp for
    for (size_t i = 0; i < size; i++) {
      sum_val += x[i];
    }
  }
  return sum_val;
}


double omp_critical_sum(double *x, size_t size)
{
  double sum_val = 0.0;

  #pragma omp parallel
  {
    #pragma omp for
    for (size_t i = 0; i < size; i++) {
      #pragma omp critical
      {
        sum_val += x[i];
      }
    }
  }
  return sum_val;
}

double opt_critical_sum(double *x, size_t size)
{
  double sum_val = 0.0;

  #pragma omp parallel
  {
    double local_sum = 0.0;
    #pragma omp for
      for (size_t i = 0; i < size; i++) {
        local_sum += x[i];
      }   

    #pragma omp critical
    {
      sum_val += local_sum;
    }
  }
  return sum_val;
}

double omp_local_sum(double *x, size_t size, int threads)
{
  double sum_val = 0.0;
  double local_sum[threads];
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    local_sum[id] = 0.0;
    #pragma omp for
      for (size_t i = 0; i < size; i++){
        local_sum[id] += x[i];
      }
  }
  for (int i = 0; i<threads; i++){
    sum_val += local_sum[i];
  }
  return sum_val;
}

double opt_local_sum(double *x, size_t size, int threads)
{
  double sum_val = 0.0;
  pad_double local_sum[threads];
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    local_sum[id].val = 0.0;
    #pragma omp for
      for (size_t i = 0; i < size; i++){
        local_sum[id].val += x[i];
      }
  }
  for (int i = 0; i<threads; i++){
    sum_val += local_sum[i].val;
  }
  return sum_val;
}

int main(int argc, char *argv[]){
  double *elements;
  elements = (double*)malloc(S*sizeof(double));
  generate_random(elements,S);
  double start_time;
  double sum;
  int func = 1;
  if(argc == 2){
    func = atoi(argv[1]);
  }
  int threadlist[4] = {1,32,64,128};
  for(int i = 0; i<4; i++){
    omp_set_num_threads(threadlist[i]);
    start_time = omp_get_wtime();
    switch(func){
      case 2:
        sum = omp_sum(elements,S);
        break;
      case 3:
        sum = omp_critical_sum(elements,S);
        break;
      case 4:
        sum = opt_critical_sum(elements,S);
        break;
      case 5:
        sum = omp_local_sum(elements,S,threadlist[i]);
        break;
      case 6:
        sum = opt_local_sum(elements,S,threadlist[i]);
        break;
      default:
        sum = serial_sum(elements,S);
    }
    double end_time = omp_get_wtime();
    printf("Threads = %d, Sum = %f, time = %fs\n",threadlist[i], sum, end_time-start_time);
  }
}
