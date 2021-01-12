#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

//para compilar:
//g++ process_images.cpp -o process_images `pkg-config --cflags --libs opencv`

void join_images(Mat Im, Mat T, Mat Z1, Mat Z2, Mat Z3);
void write_events(Mat T, int img_nr);
void write_tracklets(Mat T, int img_nr);


int main(int argc, char **argv) {
  //numero de imagenes capturadas
  int num_imgs = atoi( argv[1] );

  //imagen compuesta
  int cols=320, rows=240;
  Size size_img = Size(3*cols,2*rows);
  Mat Im(size_img, CV_8UC3);
  char fn[100];  

  //imagen de tracklets
  for(int img_nr=0; img_nr<num_imgs; img_nr++){
    Mat T = Mat::zeros(Size(3*cols,rows),CV_8UC3);
    write_events(T,img_nr);
    write_tracklets(T,img_nr);

    //imagenes de detecciones
    sprintf(fn,"/home/orbbec/contador/test/images/im%04d_cam1.png",img_nr);
    Mat Z1 = imread(fn);
    sprintf(fn,"/home/orbbec/contador/test/images/im%04d_cam2.png",img_nr);
    Mat Z2 = imread(fn);
    sprintf(fn,"/home/orbbec/contador/test/images/im%04d_cam3.png",img_nr);
    Mat Z3 = imread(fn);
    join_images(Im, T, Z1, Z2, Z3);

    sprintf(fn,"/home/orbbec/contador/test/images/img%04d.png",img_nr);
    imwrite(fn,Im);

  }
  return 0;
}


//=======================================================
void join_images(Mat Im, Mat T, Mat Z1, Mat Z2, Mat Z3){
  int cols=320, rows=240;
  Vec3b cx;
  for(int i=0; i<rows; i++){
    for(int j=0; j<cols; j++){
      cx = Z1.at<Vec3b>(i,j);
      Im.at<Vec3b>(i,j) = cx;
      cx = Z2.at<Vec3b>(i,j);
      Im.at<Vec3b>(i,j+cols) = cx;
      cx = Z3.at<Vec3b>(i,j);
      Im.at<Vec3b>(i,j+2*cols) = cx;
    }
  }
  for(int i=0; i<rows; i++){
    for(int j=0; j<3*cols; j++){
      cx = T.at<Vec3b>(i,j);
      if( cx[0]==0  &&  cx[1]==0  && cx[2]==0 ){
        cx[0]=128;  cx[1]=128;  cx[2]==128;
      }
      Im.at<Vec3b>(i+rows,j) = cx;
    }
  }
}

//=======================================================
void write_events(Mat T, int img_nr){
  char fn_eventos[] = "/home/orbbec/contador/test/eventos_test.txt";
  char line_text[20];
  char txt_eventos[50];
  int E, S, nimg;
  int Et = 0,  St = 0;

  FILE* fid = fopen(fn_eventos,"r");
  if( fid ){
    while(!feof(fid)){
      E = 0;
      S = 0;
      sprintf(line_text,"%d,%d,%d",0,0,0);
      fgets(line_text,20,fid);
      sscanf(line_text,"%d,%d,%d",&nimg,&E,&S);
      if( img_nr>=nimg ){ Et += E; St += S; }
    }
    fclose(fid);
  }
  sprintf(txt_eventos,"Entradas: %d, Salidas: %d",Et,St);
  putText(T, txt_eventos, Point(20,20), FONT_HERSHEY_PLAIN, 2, Scalar(255,255,0));
}

//=======================================================
void write_tracklets(Mat T, int img_nr){
  char fn_tracklets[] = "/home/orbbec/contador/test/tracklets_test.txt";
  char line_text[30];
  int u, v, nimg, ntr;
  FILE* fid;  
 
  for(int i=0; i<30; i++){
    //selecciona un color para el traclet
    int res = i % 6;
    Scalar color = Scalar(255,0,0);
    if( res==1 ){ color = Scalar(0,255,0); }
    if( res==2 ){ color = Scalar(0,0,255); }
    if( res==3 ){ color = Scalar(128,128,0); }
    if( res==4 ){ color = Scalar(0,128,128); }
    if( res==5 ){ color = Scalar(128,0,128); }
    
    //dibuja los tracklets
    int ini = 0;
    int u0 = 0,  v0 = 0;
    fid = fopen(fn_tracklets,"r");
    while( !feof(fid) ){
      fgets(line_text,30,fid);
      sscanf(line_text,"%d,%d,%d,%d",&nimg,&ntr,&u,&v);
      if( nimg==img_nr  &&  ntr==i ){
        if( ini==1 ){
          circle(T,Point(u,v),5,color,-1);
          line(T,Point(u0,v0),Point(u,v),color,1); 
        }
        else{
          circle(T,Point(u,v),5,color,-1);
          ini = 1;
        }
        u0 = u;
        v0 = v;
      }
    }
    fclose(fid);
  }
}




