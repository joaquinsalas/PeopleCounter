#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include "json.h"

using namespace std;

//======================================================
void create_directories(tm* now){
  //crea directorios que no existen
  // mode DEFFILEMODE : rw-rw-rw (0666)
  // mode ACCESSPERMS : rwxrwxrwx (0777)
  char proot[] = "/home/orbbec/contador/data";
  struct stat info;
  char pathname[100];

  //subdirectorios con el año actual
  sprintf(pathname,"%s/monthly/%d", proot, now->tm_year+1900);
  if( stat( pathname, &info) != 0 )
    mkdir( pathname, ACCESSPERMS );
  sprintf(pathname,"%s/daily/%d", proot, now->tm_year+1900);
  if( stat( pathname, &info) != 0 )
    mkdir( pathname, ACCESSPERMS );
  sprintf(pathname,"%s/general/%d", proot, now->tm_year+1900);
  if( stat( pathname, &info) != 0 )
    mkdir( pathname, ACCESSPERMS );

  //subdirectorios con el mes actual
  sprintf(pathname,"%s/daily/%d/%02d", proot, now->tm_year+1900, now->tm_mon+1);
  if( stat(pathname, &info) != 0 )
    mkdir( pathname, ACCESSPERMS );
  sprintf(pathname,"%s/general/%d/%02d", proot, now->tm_year+1900, now->tm_mon+1);
  if( stat(pathname, &info) != 0 )
    mkdir( pathname, ACCESSPERMS );
}

//======================================================
void update_json_general(int entradas, int salidas, tm* now){
  //actualiza los datos del archivo general del día actual
  char proot[] = "/home/orbbec/contador/data";
  char fn[100];
  sprintf(fn, "%s/general/%d/%02d/records-%02d.json", proot,
    now->tm_year + 1900, now->tm_mon+1, now->tm_mday);
  int fd = 0;
  ifstream fid( fn );
  if( fid.is_open() ){ fd = 1; }
  fid.close();

  //crea el archivo general si no existe
  if( fd==0 ){
    char header[200];
    sprintf(header, "{\n\t\"cols\":[\n\t\t{\"label\":\"Fecha\",\"type\":\"date\"},");
    sprintf(header, "%s\n\t\t{\"label\":\"Entradas\",\"type\":\"number\"},\n\t\t", header);
    sprintf(header, "%s{\"label\":\"Salidas\",\"type\":\"number\"}\n\t\t],\n\t\"rows\":[\n", header);
    ofstream fidw(fn);
    fidw.write(header, strlen(header));
    char linew[100];
    sprintf(linew, "\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,%d,%d,%d)\"},{\"v\":%d},{\"v\":%d}]}\n\t\t]\n}",
      now->tm_year+1900, now->tm_mon, now->tm_mday, now->tm_hour,
      now->tm_min, now->tm_sec, entradas, salidas);
    fidw.write(linew, strlen(linew));
    fidw.close();
  }

  //actualiza el archivo general del día actual si ya existe
  else{
    //lee el último valor de entradas y salidas del archivo
    int ent, sal, yy, mo, dd, hh, mn, ss;
    string line;
    ifstream fidr(fn);
    fidr.seekg(-70, ios::end);
    for(int i=0; i<2; i++)
      getline(fidr, line);
    fidr.close();
    const char* liner = line.c_str();
    sscanf(liner,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,%d,%d,%d)\"},{\"v\":%d},{\"v\":%d}]}",
      &yy,&mo,&dd,&hh,&mn,&ss,&ent,&sal);

    //agrega una línea con los nuevos datos
//    ent += entradas;
//    sal += salidas;
    ent = entradas;
    sal = salidas;
    char linew[100];
    sprintf(linew, ",\n\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,%d,%d,%d)\"},{\"v\":%d},{\"v\":%d}]}\n\t\t]\n}",
      now->tm_year+1900, now->tm_mon, now->tm_mday, now->tm_hour,
      now->tm_min, now->tm_sec, ent, sal);
    fstream fidw(fn);
    fidw.seekg(0, ios::end);
    int pos = fidw.tellg();
    fidw.seekp(pos-6);
    fidw.write(linew, strlen(linew));
    fidw.close();
  }
}  

//======================================================
void update_json_year(int entradas, int salidas, tm* now){
  //actualiza los datos de entradas y salidas del mes del año correspondiente
  char proot[] = "/home/orbbec/contador/data";
  //determina si ya existe el archivo del año actual
  char fn[100];
  sprintf(fn, "%s/yearly/%d.json", proot, now->tm_year + 1900);
  int fd = 0;
  ifstream fid( fn );
  if( fid.is_open() ){ fd = 1; }
  fid.close();

  // lee las entradas y salidas de cada mes y actualiza los del mes actual
  int ent[12], sal[12], yy, mo;
  if( fd==0 ){
    for(int i=0; i<12; i++){
      ent[i] = 0;
      sal[i] = 0;
    }
  }
  else{
    string line;
    ifstream fidr(fn);
    for(int i=0; i<7; i++)
      getline(fidr, line);
    for(int i=0; i<12; i++){
      getline(fidr, line);
      sscanf(line.c_str(), "\t\t{\"c\":[{\"v\":\"Date(%d,%d,1,0,0,0)\"},{\"v\":%d},{\"v\":%d}]}", 
        &yy, &mo, &ent[i], &sal[i]);
    }
    fidr.close();
  }

  //actualiza el archivo
  ent[now->tm_mon] += entradas;
  sal[now->tm_mon] += salidas;
  char header[200];
  sprintf(header, "{\n\t\"cols\":[\n\t\t{\"label\":\"Mes\",\"type\":\"date\"},");
  sprintf(header, "%s\n\t\t{\"label\":\"Entradas\",\"type\":\"number\"},\n\t\t", header);
  sprintf(header, "%s{\"label\":\"Salidas\",\"type\":\"number\"}\n\t\t],\n\t\"rows\":[\n", header);
  ofstream fidw(fn);
  fidw.write(header, strlen(header));
  char linew[100];
  for(int i=0; i<11; i++){
    sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,1,0,0,0)\"},{\"v\":%d},{\"v\":%d}]},\n",
      now->tm_year+1900, i, ent[i], sal[i]);
    fidw.write(linew, strlen(linew));
  }
  sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,11,1,0,0,0)\"},{\"v\":%d},{\"v\":%d}]}\n\t]\n}",
    now->tm_year+1900, ent[11], sal[11]);
  fidw.write(linew, strlen(linew));
  fidw.close();
}  

//======================================================
void update_json_month(int entradas, int salidas, tm* now){
  //actualiza los datos de entradas y salidas del día del mes correspondiente
  char proot[] = "/home/orbbec/contador/data";
  //determina si ya existe el archivo del mes actual
  char fn[100];
  sprintf(fn, "%s/monthly/%d/%02d.json", proot, now->tm_year + 1900, now->tm_mon+1);
  int fd = 0;
  ifstream fid( fn );
  if( fid.is_open() ){ fd = 1; }
  fid.close();

  // lee las entradas y salidas de cada día y actualiza los del día actual
  int ndays = 31,  month = now->tm_mon + 1;
  if( month==4 || month==6 || month==9 || month==11 ){ ndays = 30; }
  if( month == 2 ) {ndays = 28; }
  if( (month==2) && (now->tm_year % 4 == 0) ){ ndays = 29; }
  int ent[32], sal[32], yy, mo, dd;
  if( fd==0 ){
    for(int i=0; i<ndays; i++){
      ent[i] = 0;
      sal[i] = 0;
    }
  }
  else{
    string line;
    ifstream fidr(fn);
    for(int i=0; i<7; i++)
      getline(fidr, line);
    for(int i=0; i<ndays; i++){
      getline(fidr, line);
      sscanf(line.c_str(), "\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,0,0,0)\"},{\"v\":%d},{\"v\":%d}]}",
        &yy, &mo, &dd, &ent[i], &sal[i]);
    }
    fidr.close();
  }

  //actualiza el archivo
  ent[now->tm_mday-1] += entradas;
  sal[now->tm_mday-1] += salidas;
  char header[200];
  sprintf(header, "{\n\t\"cols\":[\n\t\t{\"label\":\"Dia\",\"type\":\"date\"},");
  sprintf(header, "%s\n\t\t{\"label\":\"Entradas\",\"type\":\"number\"},\n\t\t", header);
  sprintf(header, "%s{\"label\":\"Salidas\",\"type\":\"number\"}\n\t\t],\n\t\"rows\":[\n", header);
  ofstream fidw(fn);
  fidw.write(header, strlen(header));
  char linew[100];
  for(int i=0; i<(ndays-1); i++){
    sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,0,0,0)\"},{\"v\":%d},{\"v\":%d}]},\n",
      now->tm_year+1900, now->tm_mon, i+1, ent[i], sal[i]);
    fidw.write(linew, strlen(linew));
  }
  sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d,0,0,0)\"},{\"v\":%d},{\"v\":%d}]}\n\t]\n}",
    now->tm_year+1900, now->tm_mon, ndays, ent[ndays-1], sal[ndays-1]);
  fidw.write(linew, strlen(linew));
  fidw.close();
}

//======================================================
void update_json_day(int entradas, int salidas, tm* now){
  //actualiza los datos de entradas y salidas del intervalo del día correspondiente
  //dividiendo el día en 48 intervalos de media hora
  char proot[] = "/home/orbbec/contador/data";
  //determina si ya existe el archivo del año actual
  char fn[100];
  sprintf(fn, "%s/daily/%d/%02d/%02d.json", proot, now->tm_year + 1900,
    now->tm_mon+1, now->tm_mday);
  int fd = 0;
  ifstream fid( fn );
  if( fid.is_open() ){ fd = 1; }
  fid.close();

  // lee las entradas y salidas de cada intervalo del día y actualiza los del intervalo actual
  int ent[50], sal[50], yy, mo, dd, hh, mm;
  if( fd==0 ){
    for(int i=0; i<48; i++){
      ent[i] = 0;
      sal[i] = 0;
    }
  }
  else{
    string line;
    ifstream fidr(fn);
    for(int i=0; i<8; i++)
      getline(fidr, line);
    for(int i=0; i<48; i++){
      getline(fidr, line);
      sscanf(line.c_str(), "\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d)\"},{\"v\":[%d,%d,0,0]},{\"v\":%d},{\"v\":%d}]}",&yy, &mo, &dd, &hh, &mm, &ent[i], &sal[i]);
    }
    fidr.close();
  }

  //actualiza el archivo
  int ind = 2 * now->tm_hour;
  if( ind==48 ){ ind = 0; }
  if( now->tm_min > 29 ){ ind++; }
  ent[ind] += entradas;
  sal[ind] += salidas;
  char header[200];
  sprintf(header, "{\n\t\"cols\":[\n\t\t{\"label\":\"Fecha\",\"type\":\"date\"},");
  sprintf(header, "%s\n\t\t{\"label\":\"Hora\",\"type\":\"timeofday\"},", header);
  sprintf(header, "%s\n\t\t{\"label\":\"Entrantes\",\"type\":\"number\"},\n\t\t", header);
  sprintf(header, "%s{\"label\":\"Salientes\",\"type\":\"number\"}\n\t\t],\n\t\"rows\":[\n", header);
  ofstream fidw(fn);
  fidw.write(header, strlen(header));
  char linew[100];
  for(int i=0; i<47; i++){
    int hour = (int)(0.5*(float)i + 0.6);
    int min = 30 * ((i+1) % 2) ;
    sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d)\"},{\"v\":[%d,%d,0,0]},{\"v\":%d},{\"v\":%d}]},\n",
      now->tm_year+1900, now->tm_mon, now->tm_mday, hour, min, ent[i], sal[i]);
    fidw.write(linew, strlen(linew));
  }
  sprintf(linew,"\t\t{\"c\":[{\"v\":\"Date(%d,%d,%d)\"},{\"v\":[%d,0,0,0]},{\"v\":%d},{\"v\":%d}]}\n\t]\n}",
    now->tm_year+1900, now->tm_mon, now->tm_mday, 24, ent[47], sal[47]);
  fidw.write(linew, strlen(linew));
  fidw.close();
}
