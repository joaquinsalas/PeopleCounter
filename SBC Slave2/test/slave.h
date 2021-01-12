
#ifndef SLAVE_H
#define SLAVE_H

using namespace std;
using namespace cv;
using namespace openni;

void send_data(int* u, int* v, int* z, int sockfd, int rcap);
void depth_map(Mat Z, DepthPixel* pDepth);
void detection(int* ud, int* vd, int* zd, Mat Z, Mat fgmask);
void hmintransform(Mat fgmask, Mat Zm, Mat Z, float h);
float fitCircle(vector<Point> contour, int* xc, int* yc, int* rc, float th);
void depth_fix(Mat Z);
void initialize_socket(int& sockfd);
void error(const char *msg);

class Kinect{
  public:
    Kinect();
    ~Kinect();
    Device device;
    VideoStream depth;
    VideoFrameRef frame_depth;
    int error;
};

#endif
