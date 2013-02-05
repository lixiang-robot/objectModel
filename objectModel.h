#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include "learnandRec.h"
using namespace std;

#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)



struct modelData {
  vector<sift> gfea;
  vector<vertex> vertices;
  short int ***scv;
};


class Model {

 public:
  //variables
  vector< modelData > learnedModels;

  //functions
  Model();
  int learnModel(featureCluster fea, vector<sift> cur);
  int learnModel(featureCluster fea, vector<sift> cur, int *roi);
  float compare(modelData learned, vector<sift> cur, short int *list);

 private:
  //variables
  int inROI(sift cur, int *roi);

  //functions
  int learnShape(modelData &curModel);
  int learnSpatial(modelData &curModel);
  int InsidePolygon(vector<point> polygon,int N,point p);
};

#endif
