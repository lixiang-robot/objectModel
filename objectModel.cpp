#include "objectModel.h"

using namespace std;

Model::Model() {
  //noObject = 0;
  //par_k = 2;
  //sita = 1;  
  //sitaContext = 0.5;
  //norm = gamma(sita,par_k,sita);
  ;
}


int Model::learnModel(featureCluster fea, vector<sift> cur) {

  modelData newModel;
 
  for(int i = 0; i < fea.no.size(); i++) 
    newModel.gfea.push_back(cur[fea.no[i]]);

  learnShape(newModel); // Learn shape
  learnSpatial(newModel); // Learn Spatial Coherence Vector (SCV)
      
  learnedModels.push_back(newModel);

  
  
}

int Model::learnModel(featureCluster fea, vector<sift> cur, int *roi) {

  modelData newModel;
 
  for(int i = 0; i < fea.no.size(); i++) {
    if(inROI(cur[fea.no[i]], roi))
      newModel.gfea.push_back(cur[fea.no[i]]);
  }
      
  learnedModels.push_back(newModel);
  
}


int Model::inROI(sift cur, int *roi) {
  if(cur.x < roi[0])
    return 0;

  if(cur.x > roi[1])
    return 0;

  if(cur.y < roi[2])
    return 0;

  if(cur.y > roi[3])
    return 0;

  return 1;
}


float Model::compare(modelData learned, vector<sift> cur, short int *list) {
  /*
  featureCluster fea;
  //Initialize by the nearest neighbor
  for(int i = 0; i < learned.vertices.size(); i++) {
    int index = list[learned.vertices[i].no*KNN];
    int found = 0;
    for(int k = 0; k < fea.no.size(); k++) {
      if(fea.no[k] == index)
	found = 1;
    }

    if(found == 0)
      fea.no.push_back(list[learned.vertices[i].no*KNN]);
  }

  modelData thisModel;
  for(int i = 0; i < fea.no.size(); i++) 
    thisModel.gfea.push_back(cur[fea.no[i]]);
  
  learnShape(thisModel);

  for(int k = 0; k < thisModel.vertices.size(); k++) 
    cout<<thisModel.gfea[thisModel.vertices[k].no].x<<" "<<thisModel.gfea[thisModel.vertices[k].no].y<<endl;
  cout<<endl;
  */
  featureCluster fea;
  //Initialize by the nearest neighbor
  for(int i = 0; i < learned.vertices.size(); i++) 
    fea.no.push_back(list[learned.vertices[i].no*KNN]);

  for(int i = 0; i < learned.vertices.size(); i++) {
    
    int maxShapeIndex = 0;
    float maxShape = 1;

    for(int j = 0; j < KNN; j++) {
      //cout<<list[learned.vertices[i].no*KNN + j]<<" ";
      fea.no[i] = list[learned.vertices[i].no*KNN + j];

      featureCluster canFea;
      canFea.no.push_back(fea.no[0]);
      for(int k = 1; k < learned.vertices.size(); k++) {
	int found = 0;
	for(int m = 0; m < canFea.no.size(); m++) {
	  if(canFea.no[m] == fea.no[k])
	    found = 1;
	}

	if(found == 0)
	  canFea.no.push_back(fea.no[k]);
      }

      
      modelData thisModel;
      for(int k = 0; k < canFea.no.size(); k++) 
	thisModel.gfea.push_back(cur[canFea.no[k]]);

      learnShape(thisModel);
      
      ofstream ofs("test.txt");
      for(int k = thisModel.vertices.size(); k > 0; k--) 
	ofs << thisModel.vertices[k-1].x <<" "<< thisModel.vertices[k-1].y <<endl;

      ofs<<endl;

      for(int k = learned.vertices.size(); k > 0; k--) 
	ofs << learned.vertices[k-1].x <<" "<< learned.vertices[k-1].y <<endl;

      ofs.close();
      
      //./sim < test.txt  > out.dat
      string cmd("./glx/sim < test.txt > out.dat");
      FILE* test = popen(cmd.c_str(), "r");
      pclose(test);

      
      ifstream ifs("out.dat");
      if(!ifs) {
	cout << "error while opening out.dat" << endl;
	return 0;
      }

      float f;
      ifs >> f;
      cout<<"Shape "<<f<<endl;
      ifs.close();  

      if(f < maxShape) {
	maxShape = f;
	maxShapeIndex = j;
      }
    }

    fea.no[i] = list[learned.vertices[i].no*KNN + maxShapeIndex];    
  }
  
}


int Model::learnShape(modelData &curModel) {

  FILE *f;
  if((f=fopen("rbox.txt","w"))==NULL) {
    printf("unable to open file <%s>\n", "rbox.txt");
    exit(1);
  }

  fprintf(f, "2 rbox %d D2\n", curModel.gfea.size());
  fprintf(f, "%d\n", curModel.gfea.size());

  for (int i = 0; i < curModel.gfea.size(); i++) {
    fprintf(f,"%f ",curModel.gfea[i].x);
    fprintf(f,"%f ",curModel.gfea[i].y);
    fprintf(f,"\n");
  }
	
  fclose(f);

  //run qhull
  string cmd = "./glx/qhull Fx < rbox.txt TO qhull.txt";
  FILE* test = popen(cmd.c_str(), "r");
  pclose(test);
  
  ifstream ifs("qhull.txt");
  if(!ifs) {
    cout << "error while opening qhull.txt" << endl;
    return 0;
  }

  int no, total;
  vertex curVertex;
  ifs >> total;
  //while(!ifs.eof()) {
  for(int i = 0; i < total; i++) {
    ifs >> no;
    curVertex.x = curModel.gfea[no].x;
    curVertex.y = curModel.gfea[no].y;
    curVertex.no = no;
    
    curModel.vertices.push_back(curVertex);
  }
  
  /*
  curVertex.x = 173;
  curVertex.y = 300;

  cout<<"Poly : "<<InsidePolygon(curModel.vertex, curModel.vertex.size(), curVertex)<<endl;
  */

  return 1;
}

int Model::InsidePolygon(vector<point> polygon,int N,point p) {
  int counter = 0;
  int i;
  double xinters;
  point p1,p2;

  p1 = polygon[0];
  for (i=1;i<=N;i++) {
    p2 = polygon[i % N];
    if (p.y > MIN(p1.y,p2.y)) {
      if (p.y <= MAX(p1.y,p2.y)) {
        if (p.x <= MAX(p1.x,p2.x)) {
          if (p1.y != p2.y) {
            xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
            if (p1.x == p2.x || p.x <= xinters)
              counter++;
          }
        }
      }
    }
    p1 = p2;
  }

  if (counter % 2 == 0)
    //return(OUTSIDE);
    return 0;
  else
    //return(INSIDE);
    return 1;
}

int Model::learnSpatial(modelData &curModel) {

  curModel.scv = new short int**[2];
  for(int n = 0; n < 2; n++) {
    curModel.scv[n] = new short int*[curModel.gfea.size()];
    for(int i = 0; i < curModel.gfea.size(); i++) 
      curModel.scv[n][i] = new short int[curModel.gfea.size()];
  }

  // Build SCV based on x and y cordinate
  for(int i = 0; i < curModel.gfea.size(); i++)
    for(int j = 0; j < curModel.gfea.size(); j++) {

      if(i == j) {
	curModel.scv[0][i][j] = 2;
	curModel.scv[1][i][j] = 2;
	continue;
      }

      if(abs(curModel.gfea[i].x - curModel.gfea[j].x) < 1)
	curModel.scv[0][i][j] = 0;
      else if(curModel.gfea[i].x > curModel.gfea[j].x)
	curModel.scv[0][i][j] = 1;
      else 
	curModel.scv[0][i][j] = -1;

	
      if(abs(curModel.gfea[i].y - curModel.gfea[j].y) < 1)
	curModel.scv[1][i][j] = 0;
      else if(curModel.gfea[i].y > curModel.gfea[j].y)
	curModel.scv[1][i][j] = 1;
      else 
	curModel.scv[1][i][j] = -1;
    }	
      
}

/*
int Model::outRbox(int num, char *name, int *matched, float **point)
{
  FILE *f;
  float zero = 0;
  if((f=fopen("rbox.txt","w"))==NULL)
    {
      printf("unable to open file <%s>\n",name);
      exit(1);
    }

  fprintf(f, "2 rbox %d D2\n", num);
  fprintf(f, "%d\n", num);

  for (int i=0; i<num; i++) {
    fprintf(f,"%f ",point[matched[i]][0]);
    fprintf(f,"%f ",point[matched[i]][1]);
    fprintf(f,"\n");
  }
	
  fclose(f);

  //run qhull
  char com3[100] = "./glx/qhull Fx < ";
  strcat (com3, "rbox.txt");
  strcat (com3, " TO qhull.txt");

  if(system (com3) == -1) 
    exit(1);	

  
  ifstream openfile; 
  openfile.open("qhull.txt",ios::in);

  int q;
  openfile >> noVer;
  //cout<<"vertex"<<endl;
  for(int j=0;j<noVer;j++) 
    { 
      //openfile >> sift[j][0];
      //openfile >> sift[j][1];
      openfile >> q;
      vertex[j].x = point[q][0];
      vertex[j].y = point[q][1];
      //cout<<vertex[j].x<<" "<<vertex[j].y<<endl;
    } 	
  //cout<<"vertex end"<<endl;
  
  Point p;
  p.x = 452;
  p.y = 280;
  cout<<"point p is "<<InsidePolygon(vertex, noVer, p);
  

  openfile.close();
  
  return noVer;

}
*/
