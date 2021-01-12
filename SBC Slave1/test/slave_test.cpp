#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <OpenNI.h>
#include "slave.h"

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms

using namespace std;
using namespace cv;
using namespace openni;

//programa para el contador de personas en la tarjeta esclava
//captura imagenes de profundidad, detecta personas y envi­a a la
//tarjeta maestra las coordenas y valores de profundidad de las
//detecciones, si capturo una imagen y el tiempo de captura en milisegundos
//para compilar:
// g++ slave.cpp -o slave `pkg-config --cflags --libs opencv` -I/home/pablo/OpenNI-Linux-Arm-2.3/Include
//-L/home/pablo/OpenNI-Linux-Arm-2.3/Redist -lOpenNI2 -O3

int main(int argc, char **argv) {
  //numero de imagenes que se van a capturar
  int num_imgs = atoi( argv[1] );

  //imagen de profundidad
  Size size_img = Size(320,240);
  Mat Z(size_img, CV_32F), Zgray(size_img, CV_8U), Zf(size_img, CV_32F);
  Mat Zcolor(size_img, CV_8UC3);

  //background
  Ptr<BackgroundSubtractor> pMOG = createBackgroundSubtractorMOG2(5,16,0);
  Mat fgmask;
  //inicializa la camara
  Kinect knt;
  if( knt.error ) return 0;
    
  //coordenadas de las detecciones
  int ud[30], vd[30], zd[30];
  //socket para recibir y transmitir dados
  int sockfd;
  initialize_socket(sockfd);
  char res[2];

  for(int img_nr=0; img_nr<num_imgs; img_nr++){
    //captura una imagen
    read(sockfd, res, 1);  //espera la orden de captura

    //termina el programa si recibe la orden de la tarjeta maestra
    //if( strcmp(res,"2")==0 )
    //  break;

    write(sockfd, "1", 1);  //avisa que recibio la orden
    int changedStreamDummy;
    VideoStream* pStream = &knt.depth;
    int rcap = 0;
    Status rc = OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    if( rc==STATUS_OK ){ rcap = 1; }
    knt.depth.readFrame( &knt.frame_depth );
    DepthPixel* pDepth = (DepthPixel*) knt.frame_depth.getData();
    depth_map(Z, pDepth);
    depth_fix( Z );
    Z.row(0) = Scalar(0);  Z.row(size_img.height-1) = Scalar(0);
    Z.col(0) = Scalar(0);  Z.col(size_img.width-1) = Scalar(0);

    //actualiza el fondo
    Z.convertTo(Zgray, CV_8U, 255.0/3.5, 0.0);
    pMOG->apply(Zgray, fgmask, 0.0001);

    //detecta personas y anota las coordenadas en la imagen
    detection(ud, vd, zd, Z, fgmask);
    //envi­a los datos de detecciones
    send_data(ud, vd, zd, sockfd, rcap);

    //guarda la imagen de gris
    cvtColor(Zgray, Zcolor, COLOR_GRAY2RGB);
    for(int i=0; i<30; i++){
      if( ud[i]!=0 || vd[i]!=0 )
        circle(Zcolor, Point(ud[i],vd[i]),5,Scalar(255,0,0),-1);
    }
    char fn[100];
    sprintf(fn,"/home/orbbec/contador/test/images/im%04d_cam1.png",img_nr);
    imwrite(fn, Zcolor);
  }
  close( sockfd );
  
  return 0;
}

//=============================================================================
void send_data(int* u, int* v, int* z, int sockfd, int rcap){
  //envi­a las coordenadas de las detecciones
  int buffer[90];
  char res[2];
  write(sockfd, &rcap, sizeof(int));
  read(sockfd, res, 2);
  for(int i=0; i<30; i++){
    int idx = 3*i;
    buffer[idx] = u[i];  buffer[idx+1] = v[i];  buffer[idx+2] = z[i];
  }
  write(sockfd, buffer, sizeof(buffer));
  read(sockfd, res, 2);
}

//============================================================
void depth_map(Mat Z, DepthPixel* pDepth){
  //crea la imagen de profundidad a partir de los datos
  //capturados por el kinect
  int cs = Z.cols, rs = Z.rows;
  for(int u=0; u<cs; u++){
    for(int v=0; v<rs; v++){
      int ind = v*cs + (cs-u-1);
      Z.at<float>(v,u) = (float)pDepth[ind]/1000;
    }
  }
}

//=======================================================================
void detection(int* ud, int* vd, int* zd, Mat Z, Mat fgmask){
  //realiza la deteccion y anota las coordenadas
  //parametros de deteccion
  //Ã¡rea de la cabeza en cm^2
  float Area, Amin = 100, Amax = 350;
  //umbrales para decidir si un contorno se ajusta a una circunferencia
  float fitmin = 0.7f, th = 0.18f; 
  //Area mi­nima de la cabeza de una persona en pixeles en la imagen
  float fx = 295;
  float amin = 300.0;
  //inicializa las coordenadas
  for( int i=0; i<30; i++ ){
    ud[i] = 0;   vd[i] = 0;   zd[i] = 0;
  }
  //deteccion de blobs y sus contornos
  int cols = Z.cols,  rows = Z.rows;   
  Mat Zm(Size(cols,rows), CV_32F);
  hmintransform(fgmask, Zm, Z, 0.25f);
  Mat bimage = (Zm-Z>0.18) & (fgmask>0);
  vector< vector<Point> > contours;
  findContours(bimage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
  //filtra los contornos que satisfacen los umbrales
  int k = 0, xc, yc, zc, rc;
  for(int i=0; i<(int)contours.size(); i++){
    int area = contourArea( contours[i] );
    if( area >= amin ){
      float thm = th * sqrt(area);
      float fit_val = fitCircle(contours[i], &xc, &yc, &rc, thm);
      if( fit_val >= fitmin && k<30 ){
	//valor de profundidad en el centro del blob en cm
        int xb = xc,  yb = yc;
        if( xb<0 ) xb = 0;  if( xb>=cols ) xb = cols-1;
        if( yb<0 ) yb = 0;  if( yb>=rows ) yb = rows-1;
        zc = (int)(Z.at<float>(yb,xb)*100);
	Area =  3.1416*(rc*rc)*(zc*zc)/(fx*fx);  //Ã¡rea del cÅ•culo en cm^2;
	if( Area>=Amin && Area<=Amax ){
	  ud[k] = xc;  vd[k] = yc;
	  zd[k] = (int)(Z.at<float>(yb,xb)*1000);  //depth in mm
	  k++;
	}
      }
    }
  }
}

//======================================================================
void hmintransform(Mat fgmask, Mat Zm, Mat Z, float h){
  Mat Zh = Z + h;
  int rows = Z.rows, cols = Z.cols, ind, u ,v;
  int n = rows*cols;
  float p[5], zmin;
  for(int i=0; i<2*n; i++){
     ind = i<n? i: 2*n-i-1;
     u = ind % cols;
     v = (ind-u) / cols;
  if((fgmask.at<unsigned char>(v,u)) > 0 ){
    if(u>0 && u<(cols-1) && v>0 && v<(rows-1)){
      p[0] = Zh.at<float>(v,u);
      p[1] = Zh.at<float>(v-1,u);  p[2] = Zh.at<float>(v+1,u);
      p[3] = Zh.at<float>(v,u-1);  p[4] = Zh.at<float>(v,u+1);
      zmin = *min_element(&p[0], &p[5]);
      zmin = zmin<Z.at<float>(v,u)? Z.at<float>(v,u): zmin;
      Zh.at<float>(v,u) =  zmin;
     }
   }   
  }
  Zh.copyTo(Zm);
}

//=======================================================================
//ajusta un contorno a una circunferencia usando Ransac y 
//devuelve el promedio de los puntos que se ajustan
float fitCircle(vector<Point> contour, int* xc, int* yc, int* rc, float th){
  xc[0] = 0;  yc[0] = 0;  rc[0] = 0;
  float fv = 0.0f, xi, yi;
  int nc = contour.size();
  float M[3][2], N[3];
  for(int iter=0; iter<100; iter++){
    for(int i=0; i<3; i++){
      int ind = rand() % nc;
      xi = (float)contour[ind].x;
      yi = (float)contour[ind].y;
      M[i][0] = xi;  M[i][1] = yi; N[i] = xi*xi+yi*yi;
    }
    float detM = M[0][0]*M[1][1] - M[0][1]*M[1][0] - M[0][0]*M[2][1]
      + M[0][1]*M[2][0] + M[1][0]*M[2][1] - M[1][1]*M[2][0];
    if( abs(detM)>=5.0f ){
      float xp = -0.5f*(M[0][1]*N[1] - M[0][1]*N[2] - M[1][1]*N[0]
        + M[1][1]*N[2] + M[2][1]*N[0] - M[2][1]*N[1]) / detM;
      float yp = 0.5f*(M[0][0]*N[1] - M[0][0]*N[2] - M[1][0]*N[0]
        + M[1][0]*N[2] + M[2][0]*N[0] - M[2][0]*N[1]) / detM;        
      float s = (M[0][0]*M[1][1]*N[2] - M[0][1]*M[1][0]*N[2] 
        - M[0][0]*M[2][1]*N[1] + M[0][1]*M[2][0]*N[1] + M[1][0]*M[2][1]*N[0]
        - M[1][1]*M[2][0]*N[0]) / detM;
      float rp = sqrt(s + xp*xp + yp*yp);
      float fp = 0.0f;
      for(int i=0; i<nc; i++){
        xi = (float)contour[i].x;  yi = (float)contour[i].y;
        float ri = sqrt((xi-xp)*(xi-xp) + (yi-yp)*(yi-yp));
        if( abs(ri-rp) <= th ){ fp += 1.0/(float)nc; }
      }
      if( fp>fv ){
	fv = fp;  xc[0] = (int)xp;  yc[0] = (int)yp;  rc[0] = (int)rp;
      }
    }
  }
  return fv;
}

//==================================================================
//fill the missing values of the depth map by using interpolation
void depth_fix(Mat Z){
  float zp, zmin = 0.5, zmax = 3.0;
  float zc[8], zr;
  for(int u=1; u<Z.cols-4; u++){
    for(int v=1; v<Z.rows-1; v++){
      zp = Z.at<float>(v,u);
      if( zp<zmin || zp>zmax ){
        zr = Z.at<float>(v-1,u);
        zc[0] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v-1,u+1);
        zc[1] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v,u+1);
        zc[2] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v+1,u+1);
        zc[3] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v+1,u);
        zc[4] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v+1,u-1);
        zc[5] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v,u-1);
        zc[6] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = Z.at<float>(v-1,u-1);
        zc[7] = (zr>=zmin && zr<=zmax)? zr: 0.0;
        zr = *max_element(&zc[0], &zc[8]);
        Z.at<float>(v,u) = (zr>0)? zr: zmax;
      }
    }
  }
}

//=================================================
//Kinect constructor
Kinect::Kinect(){
  this->error = 0;
  Status rc = OpenNI::initialize();
  if( rc != STATUS_OK ){
    printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
    this->error = 1;
  }
  if( this->error==0 ){
    rc = this->device.open(ANY_DEVICE);
    if( rc != STATUS_OK ){
      printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
      OpenNI::shutdown();
      this->error = 1;
    }
  }
  if( this->error==0 ){
    rc = depth.create(this->device, SENSOR_DEPTH);
    if( rc != STATUS_OK ){
      printf("Couldn't find depth stream\n%s\n", OpenNI::getExtendedError());
      this->error = 1;
    }
  }
  if( this->error==0 ){
    VideoMode depth_videoMode = depth.getVideoMode();
    depth_videoMode.setResolution(320, 240);
    depth.setVideoMode( depth_videoMode );
    rc = depth.start();
    if( rc != STATUS_OK ){
      printf("Couldn't start depth stream\n%s\n", OpenNI::getExtendedError());
      depth.destroy();
      this->error = 1;
    }
  }
}

//=======================================================
//Kinect destructor
Kinect::~Kinect(){
  this->depth.stop();
  this->depth.destroy();
  this->device.close();
  OpenNI::shutdown();
}

//==================================================
void error(const char *msg){
  perror(msg);
  exit(0);
}

//=====================================================
void initialize_socket(int& sockfd){
  int portno = 51717;
  char hostname[] = "192.168.10.10";
  struct sockaddr_in serv_addr;
  struct hostent *server;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if( sockfd<0 )
    error("ERROR opening socket");
  server = gethostbyname( hostname );
  if( server == NULL ){
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char*)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
//  if( connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 )
//    error("Error connecting");
  while(1){
    int res = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if( res==0 )
      break;
    usleep(1000000);
  }
}
  
