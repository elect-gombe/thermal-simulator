#include <iostream>
#include <unistd.h>

struct vector{
  int x,y;
  vector(int x,int y){
    this->x=x;
    this->y=y;
  }
};

const vector dir8[]={
  {.x=-1,.y=-1},
  {.x=0,.y=-1},
  {.x=1,.y=-1},
  {.x=-1,.y=0},
  {.x=1,.y=0},
  {.x=-1,.y=1},
  {.x=0,.y=1},
  {.x=1,.y=1},
};

static inline
vector operator+(const vector &v1,const vector& v2){
  return vector(v1.x+v2.x,v1.y+v2.y);
}

const static int width=10;
const static int height=10;

double heater_current;
double room[height][width];
double next[height][width];
vector heater={5,5};
vector sensor={2,2};
double heat;
double outside;
int keisu=8;



double gettemp(vector v){
  if(v.x>=0&&v.x<width&&v.y>=0&&v.y<height){
    return room[v.x][v.y];
  }else{
    return 0;
  }
}

void update(double watt){
  heat = heat*0.8+(heat+watt)*0.1;
  room[heater.y][heater.x] += heat;
  for(int y=0;y<height;y++){
    for(int x=0;x<width;x++){
      double tt=0;
      for(int i=0;i<8;i++){
        vector t=dir8[i]+vector(y,x);
        tt+=gettemp(t);
      }
      next[y][x]=(tt+room[y][x]*keisu)/(keisu+8);
    }
  }
  for(int y=0;y<height;y++){
    for(int x=0;x<width;x++){
      room[y][x] = next[y][x];
    }
  }
}

void printmap(void){
  printf("\033[1;1H");
  for(int y=0;y<height;y++){
    for(int x=0;x<width;x++){
      int r=0,g=0,b=0;
      if(room[y][x]>0)r=room[y][x]*3;
      else b=-room[y][x]*3;
      r=r<255?r:255;
      b=b<255?b:255;
      g=127-abs(r)/10-abs(b)/10;
      printf("\x1b[48;2;%d;%d;%dm",r,g,b);
      if(x==heater.x&&y==heater.y)printf("**");
      else if(x==sensor.x&&y==sensor.y)printf("[]");
      else printf("  ");
    }
    puts("");
  }
  printf("heater:%f,sensor:%f\n",gettemp(heater),gettemp(sensor));
}

void printtemp(){
  printf("%+6f\t%+6f\n",gettemp(heater)/10,gettemp(sensor)-30);
}

//P, osci limit
const double P_GAIN = 18.45;
const double I_GAIN = 0.;//055;
const double D_GAIN = 0;//-50.5;
  
//PI
// const double P_GAIN = 8.45;
// const double I_GAIN = 0.095;
// const double D_GAIN = 0;

//PD, much better
// const double P_GAIN = 10.45;
// const double I_GAIN = 0.;//055;
// const double D_GAIN = -180.5;

//optimal
// const double P_GAIN = 8.45;
// const double I_GAIN = 0.095;
// const double D_GAIN = -140.5;

  
double pid(double diff){
  static double pdiff;
  static double sum;
  double p;
  double i;
  double d;

  p = diff*P_GAIN;
  sum += pdiff+diff;
  i = sum*I_GAIN;
  d = (pdiff-diff)*D_GAIN;

  pdiff=diff;
  
  return p+i+d;
}

int main(int argc,char **argv){
  std::cerr << "set title '(P,I,D)=("<<P_GAIN<<","<<I_GAIN<<","<<D_GAIN<<")'"<<std::endl;
  std::cerr<<"plot \"output\" u 1 w l title 'deviation',\"output\" u 2 w l title 'output'" << std::endl;
  for(int i=0;i<500;i++){
    update(pid(30-gettemp(sensor)));
    printmap();
    //printtemp();
    usleep(20000);
  }
  return 0;
}
