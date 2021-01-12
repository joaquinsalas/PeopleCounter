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
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

//para compilar:
//g++ calibration_master.cpp -o calibration_master `pkg-config --cflags --libs opencv`

int capture_images(Mat& img1, Mat& img2, int cam1, int cam2);
int calibration_rt(Mat& R, Mat& T, Mat img1, Mat img2, Size szPattern, float* szCorners);
float projection_error(Mat R, Mat T, Mat img1, Mat img2, Size szPattern, float* szCorners);
void save_rt(Mat R, Mat T, int cam1, int cam2);
void save_json(int cam1, int result);
void get_data(unsigned char* data, int sockfd);
void color_image(Mat img, unsigned char* data);
void error(const char *msg);
void initialize_sockets(int* sockfd, int* portno);

//las computadoras esclavas capturan imagenes a color del patron
//y la maestra determina la rotacion y translacion y guarda el archivo
int main(int argc, char **argv) {
  //patron de calibracion
  Size szPattern(9,5);  //numero de esquinas (horiz, vert)
  float szCorners[] = {0.030, 0.030};  //separacion entre esquinas en metros

  //tolerancia en la retroproyección (pixeles)
  int rtol = 5;
  
  //camaras de las que se van a capturar las imagenes
  int cam1 = atoi( argv[1] );
  int cam2 = atoi( argv[2] );

  //calibracion
  Mat img1, img2, R, T;
//  save_json(cam1, 0);
  if( capture_images(img1, img2, cam1, cam2) ){
    char fn1[50], fn2[50];
    sprintf(fn1, "imgcam%dright.png", cam1); 
    imwrite(fn1, img1);
    sprintf(fn2, "imgcam%dleft.png", cam2);
    imwrite(fn2, img2);

    cout << "captura imagenes ok" << endl;

    if( calibration_rt(R, T, img1, img2, szPattern, szCorners) ){
      float error = projection_error(R, T, img1, img2, szPattern, szCorners);
      if( error <= rtol ){
        save_rt(R, T, cam1, cam2);
 //       save_json(cam1, 1);
        cout << "calibration ok" << endl;
      }
    }
  }
  else{
    cout << "no capturo imagenes" << endl;
  }

  return 0;
}

//==================================================================
//transmite las imagenes de las computadoras esclavas a la maestra
//por medio de un socket
int capture_images(Mat& img1, Mat& img2, int cam1, int cam2){
  //sockets para recibir y transmitir datos
  int sockfd[2], portno[2];
  portno[0] = 52716 + cam1;
  portno[1] = 52716 + cam2;
  initialize_sockets(sockfd, portno);

  //espera 10 segundos para colocar el patrón debajo de las camaras
  usleep(10000000);

  //ejecuta el programa de captura en las computadoras esclavas
  //color images
  Size size_img = Size(640,480);
  Mat im1(size_img, CV_8UC3), im2(size_img, CV_8UC3);
  int n = 3 * size_img.height * size_img.width;
  unsigned char data[n];
  int resp = 1;
  char res[2];
  write(sockfd[0],"1",1);
  read(sockfd[0], res, 1);
  write(sockfd[1],"1",1);
  read(sockfd[1], res, 1);

  get_data(data, sockfd[0]);
  color_image(im1, data);
  if( *max_element(&data[0], &data[n-1])==0 )
    resp = 0;

  get_data(data, sockfd[1]);
  color_image(im2, data);
  if( *max_element(&data[0], &data[n-1])==0 )
    resp = 0;

  im1.copyTo(img1);
  im2.copyTo(img2);
  close( sockfd[0] );
  close( sockfd[1] );
  return resp;
}

//====================================================
void get_data(unsigned char* data, int sockfd){
  unsigned char buffer[640];
  int nblocks = 480*3;
  for(int i=0; i<nblocks; i++){
    bzero(buffer, 640);
    read(sockfd, buffer, 640);
    write(sockfd, "0", 1);
    int ind = 640*i;
    for(int j=0; j<640; j++)
      data[ind+j] = buffer[j];
  }
}

//==================================================
void color_image(Mat img, unsigned char* data){
  int rows = 480, cols = 640;
  Vec3b cx;
  for(int i=0; i<rows; i++){
    for(int j=0; j<cols; j++){
      int ind = 3*(i*cols + j);
      cx.val[0] = data[ind];  cx.val[1] = data[ind+1];  cx.val[2] = data[ind+2];
      img.at<Vec3b>(i,j) = cx;
    }
  }
}

//=========================================================================
int calibration_rt(Mat& R, Mat& T, Mat img1, Mat img2, Size szPattern, float* szCorners){
  int res = 0;
  //encuentra las esquinas
  vector<Point2f> imgPts1, imgPts2;
  bool res1 = findChessboardCorners(img1, szPattern, imgPts1, CV_ADAPTIVE_THRESH_GAUSSIAN_C);
  bool res2 = findChessboardCorners(img2, szPattern, imgPts2, CV_ADAPTIVE_THRESH_GAUSSIAN_C);
  if( res1!=0 && res2!=0 ){
    res = 1;
 
    //esquinas en el sistema de referencia del patrÃ³n
    vector<Point3f> objPoints;
    for(int i=0; i<szPattern.height; i++){
      for(int j=0; j<szPattern.width; j++){
        float X = j*szCorners[0],  Y = i*szCorners[1];
        objPoints.push_back( Point3f(X,Y,0.0) );
      }
    }
    //parametros intri­nsecos
    Matx33d K = Matx33d(560, 0.0, 320, 0, 560, 240, 0, 0, 1);
    Matx14d distCoeffs(0, 0, 0, 0);
    //rotacion y translacion
    Mat r1, t1, r2, t2, Rp, Tp;
    Mat R1(Size(3,3),CV_32F), R2(Size(3,3),CV_32F);
    solvePnP(objPoints, imgPts1, K, distCoeffs, r1, t1, 0, 0);
    solvePnP(objPoints, imgPts2, K, distCoeffs, r2, t2, 0, 0);
    Rodrigues(r1, R1);
    Rodrigues(r2, R2);
    transpose(R2, R2);
    Rp = R1 * R2;
    Tp = t1 - Rp*t2;
    Rp.copyTo(R);
    Tp.copyTo(T);
  }
  return res;
}

//=================================================================================
float projection_error(Mat R, Mat T, Mat img1, Mat img2, Size szPattern, float* szCorners){
  //encuentra las esquinas
  vector<Point2f> imgPts1, imgPts2;
  bool res1 = findChessboardCorners(img1, szPattern, imgPts1, CV_ADAPTIVE_THRESH_GAUSSIAN_C);
  bool res2 = findChessboardCorners(img2, szPattern, imgPts2, CV_ADAPTIVE_THRESH_GAUSSIAN_C);
  //esquinas en el sistema de referencia del patron
  vector<Point3f> objPoints;
  for(int i=0; i<szPattern.height; i++){
    for(int j=0; j<szPattern.width; j++){
      float X = j*szCorners[0],  Y = i*szCorners[1];
      objPoints.push_back( Point3f(X,Y,0.0) );
    }
  }
  //parametros intri­nsecos
  Matx33d K = Matx33d(560, 0.0, 320, 0, 560, 240, 0, 0, 1);
  Matx14d distCoeffs(0, 0, 0, 0);
  //transformacion de la camara 2 a la camara 1
  Mat r2, T2, R2;
  solvePnP(objPoints, imgPts2, K, distCoeffs, r2, T2, 0, 0);
  Rodrigues(r2, R2);
  vector<Point2f> imgPts;
  Mat A = (Mat)K;
  for(int i=0; i<objPoints.size(); i++){
    double X = objPoints[i].x,  Y = objPoints[i].y,  Z = objPoints[i].z;
    double Xc = R2.at<double>(0,0)*X + R2.at<double>(0,1)*Y + R2.at<double>(0,2)*Z + T2.at<double>(0);
    double Yc = R2.at<double>(1,0)*X + R2.at<double>(1,1)*Y + R2.at<double>(1,2)*Z + T2.at<double>(1);
    double Zc = R2.at<double>(2,0)*X + R2.at<double>(2,1)*Y + R2.at<double>(2,2)*Z + T2.at<double>(2);
    float Xt = R.at<double>(0,0)*Xc + R.at<double>(0,1)*Yc + R.at<double>(0,2)*Zc + T.at<double>(0);
    float Yt = R.at<double>(1,0)*Xc + R.at<double>(1,1)*Yc + R.at<double>(1,2)*Zc + T.at<double>(1);
    float Zt = R.at<double>(2,0)*Xc + R.at<double>(2,1)*Yc + R.at<double>(2,2)*Zc + T.at<double>(2);
    double u = A.at<double>(0,0)*Xt/Zt + A.at<double>(0,2);
    double v = A.at<double>(1,1)*Yt/Zt + A.at<double>(1,2);
    imgPts.push_back(Point2f(u,v));
  }
  //error en la proyeccion de puntos
  int n = imgPts.size();
  float mean = 0, stddev = 0, error[n];
  for(int i=0; i<n; i++){
    float du = imgPts[i].x - imgPts1[i].x;
    float dv = imgPts[i].y - imgPts1[i].y;
    error[i] = sqrt(du*du + dv*dv);
    mean += error[i]/(float)n;
  }
  return mean;
}

//================================================
void save_rt(Mat R, Mat T, int cam1, int cam2){
  //guarda el archivo con los valores de R y T
  char fn[100];
  sprintf(fn, "pose_cam%dtocam%d.txt", cam2, cam1);
  FILE* fid = fopen(fn, "w");
  for(int i=0; i<3; i++)
    fprintf(fid,"%0.4f %0.4f %0.4f\r\n",R.at<double>(i,0),R.at<double>(i,1),R.at<double>(i,2));
  fprintf(fid,"%0.4f %0.4f %0.4f",T.at<double>(0),T.at<double>(1),T.at<double>(2));
  fclose(fid);
}

//==================================================
void save_json(int cam1, int result){
  //guarda el resultado de si las camaras se calibraron o no
  fpos_t pos;
  pos.__pos = 15*(cam1-1) + 2;
  FILE* fid = fopen("calibration.json","r+");
  fsetpos(fid, &pos);
  if( cam1==1 )
    fprintf(fid, "\t\"cam1-2\":\"%d\",\n", result);
  else if( cam1==2 )
    fprintf(fid, "\t\"cam2-3\":\"%d\",\n", result);
  else if( cam1==3 )
    fprintf(fid, "\t\"cam3-4\":\"%d\"\n", result);
  fclose(fid);
}

//==================================================
void error(const char *msg){
   perror(msg);
   exit(1);
}

//=====================================================
void initialize_sockets(int* sockfd, int* portno){
  for(int i=0; i<2; i++){
    int sockfd_tmp;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd_tmp = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd_tmp<0 )
      error("ERROR opening socket");
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno[i]);
    if( bind(sockfd_tmp, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR on binding");
    listen(sockfd_tmp, 5);
    clilen = sizeof(cli_addr);

    sockfd[i] = accept(sockfd_tmp, (struct sockaddr*)&cli_addr, &clilen);
    if( sockfd[i]<0 )
      error("ERROR on accept");
    close( sockfd_tmp );
  }
}
