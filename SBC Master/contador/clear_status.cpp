#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <pthread.h>

using namespace std;

//programa para limpiar el estado de las camaras cuando termina el proceso master 
//para compilar: g++ clear_status.cpp json.cpp -o clear_status 

int main(int argc, char **argv) {
  int nc = 4;
  FILE* fid = fopen("status.json", "w");
  fprintf(fid, "{\n");
  for(int i=0; i<(nc-1); i++)
    fprintf(fid, "\t\"cam%d\":%d,\n", i+1, 0);
  fprintf(fid, "\t\"cma%d\":%d\n}", nc, 0);
  fclose(fid);

  return 0;
}


