#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/timeb.h>
#include "master.h"

using namespace std;

//programa para el contador de personas en la tarjeta maestra
//para compilar: g++ master_test.cpp -o master_test -O3

int main(int argc, char **argv) {
  //numero de imagenes que cada tarjeta esclava va a capturar
  int num_imgs = atoi( argv[1] );

  //parametros de las camaras
  //R, T = Rotacion y translacion respecto a la camara de referencia
  //c0, c1 = constantes para convertir disparidad a profundidad

  int nc = 3;  //numero de camaras
  cameraParameters cam[nc];
  //numero de observaciones por imagen en todas las camaras
  int nd = 30*nc;
  read_parameters(cam, nc);
  int u[nd], v[nd], z[nd];
  tracklet tr[nd];

  //archivos de eventos y de tracklets
  char fn_eventos[] = "/home/orbbec/contador/test/eventos_test.txt";
  char fn_tracklets[] = "/home/orbbec/contador/test/tracklets_test.txt";
  FILE* fid;

  //tiempo inicial con resolucion de milisegundos
  timeb tmb;
  ftime( &tmb );
  double tini = (double)tmb.time + (double)tmb.millitm/1000;

  //socket para recibir y transmitir datos
  int sockfd[nc], estado[2*nc];
  initialize_socket(sockfd, nc);

  double t, ta = 0, tsg, fps;
  int stcams = 0, stevents = 0;

  for(int img_nr=0; img_nr<num_imgs; img_nr++){
    //tiempo transcurrido
    ftime( &tmb );
    tsg = (double)tmb.time + (double)tmb.millitm/1000;
    t = tsg - tini;

    //fecha y hora actual
    time_t t0 = time(0);
    tm* nw = localtime(&t0);

    //obtiene las detecciones actuales de cada camara y revisa el estado de las camaras
    get_data(u, v, z, nc, sockfd, estado);

    //transforma las coordenadas a un sistema de referencia global
    for(int ncam=1; ncam<nc; ncam++)
        transform_RT(u, v, z, cam[ncam], ncam);

    //combina detecciones (borra las duplicadas)
    combine_detections(u, v, z, nc);

    //actualiza los tracklets
    update_tracklets(tr, u, v, z, t, nd);

    //anota en el archivo de tracklets
    fid = fopen(fn_tracklets, "a");
    for(int i=0; i<30; i++){
      int n = tr[i].t.size();
      for(int j=0; j<n; j++){
        fprintf(fid,"%d,%d,%d,%d\n",img_nr,i,tr[i].u[j],tr[i].v[j]);
      }
    }
    fclose(fid);

    //actualiza el conteo de personas
    int E = 0, S = 0;
    if( update_count(E, S, tr, nd) ){
        //anota en el archivo la salida del contador
        fid = fopen(fn_eventos, "a");
        fprintf(fid,"%d,%d,%d\n",img_nr,E,S);
        fclose(fid);
    }
  }
  for(int i=0; i<nc; i++)
    close( sockfd[i] );
  return 0;
}

//============================================================================
void get_data(int* u, int* v, int* z, int nc, int* sockfd, int* estado){
  //obtiene las detecciones de cada camara
  int buffer[90];
  bzero(buffer, 90);
  char res[2];
  int rd, rcap;
  for(int i=0; i<2*nc; i++){estado[i] = 0;}
  //envía orden de captura a las cámaras 
  for(int ncam=0; ncam<nc; ncam++){
    write(sockfd[ncam], "1", 1);
    rd = read(sockfd[ncam], res, 1);
    if( rd>=0 )
      estado[2*ncam] = 1;
  }
  //recibe las coordenadas de las detecciones
  for(int ncam=0; ncam<nc; ncam++){
    read(sockfd[ncam], &rcap, sizeof(int));
    write(sockfd[ncam], "1", 2);
    if( rcap==1 )
      estado[2*ncam+1] = 1;
    read(sockfd[ncam], buffer, sizeof(buffer));
    write(sockfd[ncam], "1", 2);
    int ofs = 30*ncam;
    for(int i=0; i<30; i++){
      int idx = 3*i;
      u[i+ofs] = buffer[idx];
      v[i+ofs] = buffer[idx+1];
      z[i+ofs] = buffer[idx+2];
    }
  }
}

//===========================================================================
void transform_RT(int* u, int* v, int* z, cameraParameters cam, int num_camera){
  //transform coordinates from a particular to global reference system
  float fx = 280, fy = 280, cx = 160, cy = 120;
  float X, Y, Z, Xt, Yt, Zt, D;
  int umin = 20, umax = 300;
  int ini = 30*num_camera;
  for(int i=ini; i<(ini+30); i++){
    if( u[i]<umin || u[i]>umax ){ u[i]=0; v[i]=0; z[i]=0; }
    if( z[i]>0 ){
      D = (3.15 - 1000.0/(float)z[i])/0.0028;
      Z = 1.0/(cam.c0*D + cam.c1);
      X = Z * (u[i] - cx) / fx;
      Y = Z * (v[i] - cy) / fy;      
      Xt = cam.R[0][0]*X + cam.R[0][1]*Y + cam.R[0][2]*Z + cam.T[0];
      Yt = cam.R[1][0]*X + cam.R[1][1]*Y + cam.R[1][2]*Z + cam.T[1];
      Zt = cam.R[2][0]*X + cam.R[2][1]*Y + cam.R[2][2]*Z + cam.T[2];
      u[i] = (int)((fx*Xt)/Zt + cx);
      v[i] = (int)((fy*Yt)/Zt + cy);
    }
  }
}

//======================================================================
void combine_detections(int* u, int* v, int* z, int nc){
  //combina las detecciones de una cámara con la siguiente
  float deltamax = 200;  //umbral de distancia en milimetros 
  //longitud focal y punto principal
  float fx=280, fy=280, cx=160, cy=120;
  float D[30][30];
  for(int ncam=(nc-2); ncam>=0; ncam--){
    int ofs1 = 30*ncam,  ofs2 = 30*(ncam+1);
    //anota las distancias en la matriz D
    for(int i=0; i<30; i++){
      for(int j=0; j<30; j++){
        D[i][j] = 10000.0;
        int u1=u[i+ofs1], u2=u[j+ofs2], v1=v[i+ofs1], v2=v[j+ofs2];
        if( (u1!=0 || v1!=0) && (u2!=0 || v2!=0) ){
          float Z1=(float)z[i+ofs1], Z2=(float)z[j+ofs2];
          float X1 = Z1*(u1 - cx)/fx;
          float X2 = Z2*(u2 - cx)/fx;
          float Y1 = Z1*(v1 - cy)/fy;
          float Y2 = Z2*(v2 - cy)/fy;
          D[i][j] = sqrt((X2-X1)*(X2-X1) + (Y2-Y1)*(Y2-Y1));
        }
      }
    }
    while(1){
      float Dmin = 10000.0;
      int im=0,  jm = 0;
      for(int i=0; i<30; i++){
        for(int j=0; j<30; j++){
          if( D[i][j] < Dmin ){Dmin = D[i][j];  im = i;  jm = j;}
        }
      }
      if( Dmin > deltamax )
        break;
      else{
        //combina detecciones
        u[im+ofs1] = (u[im+ofs1] + u[jm+ofs2]) / 2;
        v[im+ofs1] = (v[im+ofs1] + v[jm+ofs2]) / 2;
        u[jm+ofs2] = 0;
        v[jm+ofs2] = 0;
        for(int i=0; i<30; i++){
          D[i][jm] = 10000.0;
          D[im][i] = 10000.0;
        }
      }
    }
  }
}

//==============================================================================
void update_tracklets(tracklet* tr, int* u, int* v, int* z, double t, int num_obs){
  //parámetros
  float dt = 1.0, thd = 60;
  int thz = 100;
  //elimina los tracklets finalizados
  for(int i=0; i<num_obs; i++){
    if( tr[i].status==2 ){
      tr[i].status = 0;
      tr[i].u.clear();  tr[i].v.clear(); tr[i].z.clear();  tr[i].t.clear();
    }
  }
  //distancias entre coordenadas estimadas y nuevas detecciones
  float us, vs, mu, mv;
  int D[num_obs][num_obs], n;
  for(int i=0; i<num_obs; i++){
    for(int j=0; j<num_obs; j++){
      D[i][j] = 1000;
      if( tr[i].status==1 && (u[j]!=0 || v[j]!=0) ){
	n = tr[i].t.size();
	if( n>1 ){
	  mu = (float)(tr[i].u[n-1]-tr[i].u[n-2])/(float)(tr[i].t[n-1]-tr[i].t[n-2]);
	  mv = (float)(tr[i].v[n-1]-tr[i].v[n-2])/(float)(tr[i].t[n-1]-tr[i].t[n-2]);
	  us = (float)tr[i].u[n-1] + mu*(float)(t-tr[i].t[n-1]);
	  vs = (float)tr[i].v[n-1] + mv*(float)(t-tr[i].t[n-1]);
	  D[i][j] = sqrt(pow((int)us-u[j],2) + pow((int)vs-v[j],2));
	}
	else{
	  us = (float)tr[i].u[n-1];
	  vs = (float)tr[i].v[n-1];
	  D[i][j] = sqrt(pow((int)us-u[j],2) + pow((int)vs-v[j],2))/2;
	}
        if( abs(tr[i].z[n-1] - z[j]) > thz )
          D[i][j] = 1000;
      }
    }
  }
  //asigna nuevas detecciones a los tracklets si la distancia es menor al umbral
  int asg[num_obs];
  for(int i=0; i<num_obs; i++)
    asg[i] = 0;
  while( true ){
    int Dmin = 1000, im, jm;
    for(int i=0; i<num_obs; i++){
      for(int j=0; j<num_obs; j++){
	if( D[i][j] < Dmin ){
	  Dmin = D[i][j];
	  im = i;
	  jm = j;
	}
      }
    }
    if( Dmin < thd ){
      tr[im].u.push_back(u[jm]);
      tr[im].v.push_back(v[jm]);
      tr[im].z.push_back(z[jm]);
      tr[im].t.push_back(t);
      asg[jm] = 1;
      for(int i=0; i<num_obs; i++)
	D[i][jm] = 100;
      for(int j=0; j<num_obs; j++)
	D[im][j] = 100;
    }
    else
      break;
  }
  //inicia un tracklet para cada deteccion que no haya sido asignada
  for(int j=0; j<num_obs; j++){
    if( asg[j]==0 && (u[j]!=0 || v[j]!=0) ){
      int ind = 0;
      while( ind<num_obs ){
	if( tr[ind].status==0 ){
	  tr[ind].status = 1;
	  tr[ind].u.push_back(u[j]);
	  tr[ind].v.push_back(v[j]);
          tr[ind].z.push_back(z[j]);
	  tr[ind].t.push_back(t);
	  asg[j] = 1;
	  break;
	}
	ind++;
      }
    }
  }
  //cierra un tracklet si no se actualiza durante un tiempo
  //o si es muy largo
  for(int i=0; i<num_obs; i++){
    if( tr[i].status==1 ){
      n = tr[i].t.size();
      if( t-tr[i].t[n-1]>dt || n>1000 )
	tr[i].status = 2;
    }
  }
}

//==============================================================
int update_count(int& E, int& S, tracklet* tr, int num_obs){
  //update count when a tracket ends
  int v1 = 100,  v2 = 140;  
  //minimal number of detections on a tracklet
  int nmin = 4;
  int n, res = 0;
  for(int i=0; i<num_obs; i++){
    if( tr[i].status==2 ){
      n = tr[i].t.size();
      if( tr[i].v[0]<=v1 && tr[i].v[n-1]>=v2 && n>=nmin ){ E++; res=1; }
      if( tr[i].v[0]>=v2 && tr[i].v[n-1]<=v1 && n>=nmin ){ S++; res=1; }
    }
  }
  return res;
}

//===============================================================
void read_parameters(cameraParameters* cam, int nc){
  //read rotation R and translation T
  float Rs[3][3], Ts[3];
  FILE* fid;
  char fn[30];
  cam[0].R[0][0] = 1.0;  cam[0].R[0][1] = 0.0;  cam[0].R[0][2] = 0.0;
  cam[0].R[1][0] = 0.0;  cam[0].R[1][1] = 1.0;  cam[0].R[1][2] = 0.0;
  cam[0].R[2][0] = 0.0;  cam[0].R[2][1] = 0.0;  cam[0].R[2][2] = 1.0;
  cam[0].T[0] = 0.0;  cam[0].T[1] = 0.0;  cam[0].T[2] = 0.0;
  for(int n=1; n<nc; n++){
    sprintf(fn, "pose_cam%dtocam%d.txt", n+1, n);
    fid = fopen(fn, "r");
    for(int i=0; i<3; i++)
      fscanf(fid, "%f %f %f", &Rs[i][0], &Rs[i][1], &Rs[i][2]);
    fscanf(fid, "%f %f %f", &Ts[0], &Ts[1], &Ts[2]);
    fclose(fid);
    for(int i=0; i<3; i++){
      cam[n].T[i] = 0.0;
      for(int j=0; j<3; j++){
	cam[n].R[i][j] = 0.0;
	for(int k=0; k<3; k++)
	  cam[n].R[i][j] += cam[n-1].R[i][k] * Rs[k][j];
	cam[n].T[i] += cam[n-1].R[i][j] * Ts[j];
      }
      cam[n].T[i] += cam[n-1].T[i];
    }    
  }
  //read depth parameters c0 and c1
  sprintf(fn, "depth_parameters.txt");
  fid = fopen(fn, "r");
  for(int n=0; n<nc; n++)
    fscanf(fid, "%f %f", &cam[n].c0, &cam[n].c1);
  fclose(fid);
}
    
//==================================================
void error(const char *msg){
   perror(msg);
   exit(1);
}

//=====================================================
void initialize_socket(int* sockfd, int nc){
  int i = 0;
  while( i<nc ){
    int portno = 51717 + i;
    int sockfd_tmp;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd_tmp = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd_tmp<0 )
      error("ERROR opening socket");

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if( bind(sockfd_tmp, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR on binding");
    listen(sockfd_tmp, 5);
    clilen = sizeof(cli_addr);
    sockfd[i] = accept(sockfd_tmp, (struct sockaddr*)&cli_addr, &clilen);

    if( sockfd[i]<0 ){
      error("ERROR on accept");
      close( sockfd[i] );
      usleep(5000000);
    }
    else{
      char resc[30];
      sprintf(resc,"inicio camara %d ok",i+1);
      cout << resc << endl;
      i++;
    }
    close( sockfd_tmp );
  }
  //establece un timeout de 3 segs para los sockets
  for(i=0; i<nc; i++){
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
  }
}
