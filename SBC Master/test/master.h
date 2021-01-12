
#ifndef MASTER_H
#define MASTER_H

using namespace std;

struct detections{
  int u[30], v[30], z[30];
  double t;
};

struct tracklet{
  vector<int> u, v, z;
  vector<double> t;
  int status;
  tracklet(){status = 0;}
};

struct cameraParameters{
  float R[3][3], T[3];
  float c0, c1;
};

void save_tracklets(tracklet* tr, int& k, int E, int S);  
void get_data(int* u, int* v, int* z, int nc, int* sockfd, int* estado);
void read_parameters(cameraParameters* cam, int nc);
void transform_RT(int* u, int* v, int* z, cameraParameters cam, int num_camera);
void update_tracklets(tracklet* tr, int* u, int* v, int* z, double t, int num_obs);
void combine_detections(int* u, int* v, int* z, int nc);
int update_count(int& E, int& S, tracklet* tr, int num_obs);
void initialize_socket(int* sockfd, int nc);
void error(const char *msg);

#endif
