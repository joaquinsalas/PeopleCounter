#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <OpenNI.h>

#define SAMPLE_READ_WAIT_TIMEOUT 2000

using namespace std;
using namespace openni;

//para compilar:
//g++ calibration_slave.cpp -o calibration_slave -I/home/orbbec/OpenNI-Linux-Arm-2.3/OpenNI-Linux-Arm-2.3/Include
// -L/home/orbbec/OpenNI-Linux-Arm-2.3/OpenNI-Linux-Arm-2.3/Redist -lOpenNI2

class Kinect{
  public:
    Kinect();  ~Kinect();
    Device device;
    VideoStream color;
    VideoFrameRef frame_color;
    int error;
};


void send_data(unsigned char* data, int sockfd);
void error(const char *msg);
void initialize_socket(int& sockfd);

//las computadoras esclavas capturan imágenes a color del patrón
//y la maestra determina la rotación y translación y guarda el archivo
int main(int argc, char **argv) {
  Kinect knt;
  if( knt.error ) return 0;

  //vector para guardar los datos de la imagen capturada
  int cols = 640,  rows = 480;
  unsigned char data[cols*rows*3];

  //socket para recibir y transmitir dados
  int sockfd;
  initialize_socket(sockfd);
  
  //captura 10 imágenes (las primeras no son buenas a veces)
  for(int i=0; i<10; i++){
    int changedStreamDummy;
    VideoStream* pStream;
    pStream = &knt.color;
    OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
    knt.color.readFrame( &knt.frame_color );
    RGB888Pixel* pColor = (RGB888Pixel*)knt.frame_color.getData();
  }
   
  //captura una imagen después de recibir la orden de la maestra
  char res[2];
  read(sockfd, res, 1);
  write(sockfd, "1", 1);

  int changedStreamDummy;
  VideoStream* pStream;
  pStream = &knt.color;
  OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
  knt.color.readFrame( &knt.frame_color );
  RGB888Pixel* pColor = (RGB888Pixel*)knt.frame_color.getData();

  //copia la imagen capturada al vector de datos
  for(int i=0; i<rows; i++){
    for(int j=0; j<cols; j++){
      int indp = i*cols + (cols-j-1);
      int indd = 3*(i*cols + j);
      data[indd] = (unsigned char)pColor[indp].b;
      data[indd+1] = (unsigned char)pColor[indp].g;
      data[indd+2] = (unsigned char)pColor[indp].r;
    }
  }
  
  //envía la imagen a la computadora maestra
  send_data(data, sockfd);

  close( sockfd );
  return 0;
}

//=========================================================
void send_data(unsigned char* data, int sockfd){
  unsigned char buffer[640];
  char res[2];
  int nblocks = 480*3;
  for(int i=0; i<nblocks; i++){
    bzero(buffer, 640);
    int ind = 640*i;
    for(int j=0; j<640; j++)
      buffer[j] = data[ind+j];
    write(sockfd, buffer, 640);
    read(sockfd, res, 1);
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
    rc = color.create(this->device, SENSOR_COLOR);
    if( rc != STATUS_OK ){
      printf("Couldn't find color stream\n%s\n", OpenNI::getExtendedError());
      this->error = 1;
    }
  }
  if( this->error==0 ){
    VideoMode color_videoMode = color.getVideoMode();
    color_videoMode.setResolution(640,480);
    color.setVideoMode( color_videoMode );
    rc = color.start();
    if( rc != STATUS_OK ){
      printf("Couldn't start color stream\n%s\n", OpenNI::getExtendedError());
      color.destroy();
      this->error = 1;
    }
  }
}

//=======================================================
//Kinect destructor
Kinect::~Kinect(){
  this->color.stop();
  this->color.destroy();
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
  int portno = 52717;
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
  
