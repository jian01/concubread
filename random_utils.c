#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
 #include <unistd.h>
#include <math.h>
#include <time.h>
#include "random_utils.h"


double random_uniform(double a, double b){
  /*
  Devuelve un sample de un número de una distribucion uniforme entre (a,b)
  */
  srand( time(NULL) +clock()+random());
  double rand_num = (double)rand();
  double rand_uniform = rand_num/RAND_MAX;
  rand_uniform *= (b-a);
  rand_uniform += a;
  return rand_uniform;
}
