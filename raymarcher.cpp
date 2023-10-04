// Landscape Raymarcher attempt - port of scratch project 843305852
#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;

// Settings - these are changeable, try some combinations out!
const float grassDensity=0.6,epsilon=0.01,skyR=140,skyG=220,skyB=255,fogVal=5000,waterHeight=250,lightDiffuse=5000,fov=240;

int WIDTH=480,HEIGHT=360;

// Camera position
const float camX=0,camY=170,camZ=0,camRotX=-15,camRotY=160;

// Definitions
float sinA,cosA,sinB,cosB,r,reflectivity,temp1,temp2,brightR,brightG,brightB,distX,distY,distZ,dist,rayDirX,rayDirY,rayDirZ,rayX,rayY,rayZ,SDF,rayLen,noise,dot,SmoothStep,surfaceR,surfaceG,surfaceB,height,hitX,hitY,hitZ,normalX,normalY,normalZ,finalR,finalG,finalB,fog,reflectionR,reflectionG,reflectionB;
bool hit,stats,hitGround;


void calcDist(float x1,float y1,float z1,float x2,float y2,float z2){
    distX=x2-x1;
    distY=y2-y1;
    distZ=z2-z1;
    dist=sqrt((distX*distX)+(distY*distY)+(distZ*distZ));
}
void rotateRay(float x,float y,float z,float camX,float camY){
    rayDirY=(z*sinA)+(y*cosA);
    rayDirZ=(z*cosA)-(y*sinA);
    rayDirX=(rayDirZ*sinB)+(x*cosB);
    rayDirZ=(rayDirZ*cosB)-(x*sinB);
}
void calcDot(float x1, float y1,float z1,float x2, float y2, float z2){
    dot=(x1*x2)+(y1*y2)+(z1*z2);
}
void calcSmoothStep(float n1,float n2,float k){
    SmoothStep=n1+((k*k*(3-(2*k)))*(n2-n1));
}
void calcNoise(float x, float y, float s,float h){
    calcDot(ceil (x/s)*s,0,floor(y/s)*s,12.9898,0,78.233);
    temp1=fmod(sin(dot)*1000,1);
    calcDot(ceil (x/s)*s,0,ceil (y/s)*s,12.9898,0,78.233);
    calcSmoothStep(temp1,fmod(sin(dot)*1000,1),fmod(y,s)/s);
    temp2=SmoothStep;
    calcDot(floor(x/s)*s,0,floor(y/s)*s,12.9898,0,78.233);
    temp1=fmod(sin(dot)*1000,1);
    calcDot(floor(x/s)*s,0,ceil (y/s)*s,12.9898,0,78.233);
    calcSmoothStep(temp1,fmod(sin(dot)*1000,1),fmod(y,s)/s);
    calcSmoothStep(SmoothStep,temp2,fmod(x,s)/s);
    noise+=SmoothStep*h;
}

void terrainColSDF(float r,float g,float b,float px,float py,float pz){
    noise=100;
    /*calcNoise(px,pz,700,450);
    calcNoise(px,pz,250,200);
    calcNoise(px,pz,120,50);
    calcNoise(px,pz,50,25);
    calcNoise(px,pz,20,10);
    calcNoise(px,pz,10,3);
    calcNoise(px,pz,3,1.5);
    calcNoise(px,pz,1,0.5);*/
    if(py-noise<SDF){
        SDF=py-noise;
        if(SDF<epsilon&&stats){
            surfaceR=r;
            surfaceG=g;
            surfaceB=b;
            hitGround=true;
        }
    }
    height=noise;
}
void planeNormalsSDF(float nx,float ny,float nz,float d,float px,float py,float pz,float r,float g,float b,float ref){
    calcDot(px,py,pz,nx,ny,nz);
    dot-=d;
    if(dot<SDF){
        SDF=dot;
        if(SDF<epsilon&&stats){
            surfaceR=r;
            surfaceG=g;
            surfaceB=b;
            reflectivity=ref;
        }
    }
}
void sceneSDF(float x,float y,float z){
    SDF=1e+100;
    terrainColSDF(120,120,120,x,y,z);
    planeNormalsSDF(0,1,0,waterHeight,x,y,z,60,120,200,0.6);
}
void marchRay(float x,float y,float z,float dx,float dy,float dz,int renderDistance){
    rayX=x;
    rayY=y;
    rayZ=z;
    SDF=100000;
    hit=false;
    rayLen=0;
    for(int i=0;i<1000&&rayLen<renderDistance;){
        sceneSDF(rayX,rayY,rayZ);
        if(SDF<epsilon){
            hit=true;
            break;
        }
        i++;
        rayX+=SDF*dx;
        rayY+=SDF*dy;
        rayZ+=SDF*dz;
        rayLen+=SDF;
    }
}
void calcLighting(float px,float py, float pz, float nx,float ny,float nz,float lx,float ly,float lz,float r,float g,float b,float brightness,bool infinite){
    calcDist(px,py,pz,lx,ly,lz);
    calcDot(normalX,normalY,normalZ,distX/dist,distY/dist,distZ/dist);
    if(!infinite){
        dot-=dist/lightDiffuse;
    }
    if(dot>0){
        brightR=brightness*r*dot;
        brightG=brightness*g*dot;
        brightB=brightness*b*dot;
    }
}
void colorGround(){
    if(hitGround){
        calcDot(normalX,normalY,normalZ,0,1,0);
        if(dot>grassDensity){
            surfaceR=100;
            surfaceG=130;
            surfaceB= 80;
        }else{
            surfaceR=140;
            surfaceG=140;
            surfaceB=120;
        }
    }
}
void getNormals(float x,float y, float z){
    sceneSDF(x+epsilon,y,z);
    normalX=SDF;
    sceneSDF(x-epsilon,y,z);
    normalX-=SDF;
    sceneSDF(x,y+epsilon,z);
    normalY=SDF;
    sceneSDF(x,y-epsilon,z);
    normalY-=SDF;
    sceneSDF(x,y,z+epsilon);
    normalZ=SDF;
    sceneSDF(x,y,z-epsilon);
    normalZ-=SDF;
    calcDist(0,0,0,normalX,normalY,normalZ);
    normalX=distX/dist;
    normalY=distY/dist;
    normalZ=distZ/dist;
}
void applyFog(){
    fog=fog/fogVal;
    if(fog>1){
        fog=1;
    }
    finalR=((1-fog)*finalR)+(fog*skyR);
    finalG=((1-fog)*finalG)+(fog*skyG);
    finalB=((1-fog)*finalB)+(fog*skyB);
}
void renderPixel(float x,float y){
    r=0;
    reflectivity=0;
    hitGround=false;
    brightR=0.3;
    brightG=0.3;
    brightB=0.35;
    stats=true;
    calcDist(0,0,0,x,y,fov);
    rotateRay(distX/dist,distY/dist,distZ/dist,camRotX,camRotY);
    marchRay(camX,camY,camZ,rayDirX,rayDirY,rayDirZ,1000);
    if(hit){
        fog=rayLen;
        hitX=rayX;
        hitY=rayY;
        hitZ=rayZ;
        stats=false;
        getNormals(rayX,rayY,rayZ);
        colorGround();
        calcLighting(rayX,rayY,rayZ,normalX,normalY,normalZ,-100000,10000,0,1,0.8,0.7,1.5,true);
        finalR=surfaceR*brightR;
        finalG=surfaceG*brightG;
        finalB=surfaceB*brightB;
        if(reflectivity>0){
            r=reflectivity;
            stats=true;
            marchRay(hitX,hitY+epsilon*2,hitZ,rayDirX,0-rayDirY,rayDirZ,500);
            if(hit){
                stats=false;
                getNormals(rayX,rayY,rayZ);
                colorGround();
                calcLighting(rayX,rayY,rayZ,normalX,normalY,normalZ,-100000,10000,0,1,0.9,0.8,1.5,true);
                reflectionR=surfaceR*brightR;
                reflectionG=surfaceG*brightG;
                reflectionB=surfaceB*brightB;
            }else{
                reflectionR=skyR;
                reflectionG=skyG;
                reflectionB=skyB;
            }
        }/*
        finalR=((1-r)*finalR)+(r*reflectionR);
        finalG=((1-r)*finalG)+(r*reflectionG);
        finalB=((1-r)*finalB)+(r*reflectionB);*/
        finalR=255;
        finalG=255;
        finalB=((1-r)*finalB)+(r*reflectionB);
    }else{
        finalR=skyR;
        finalG=skyG;
        finalB=skyB;
    }
}


void reshape(int w, int h) { // Handle resizing the window
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    WIDTH = w;
    HEIGHT = h;
    glutPostRedisplay();
}
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    sinA=sin(camRotX);
    sinB=sin(camRotY);
    cosA=cos(camRotX);
    cosB=cos(camRotY);
    glBegin(GL_POINTS);
    for(int yPixel=0;yPixel<HEIGHT;yPixel++){
        for(int xPixel=0;xPixel<WIDTH;xPixel++){
            renderPixel(xPixel,yPixel);
            glColor3f(finalR/255,finalG/255,finalB/255);
            glVertex2i(xPixel,yPixel);
        }
    }
    glEnd();
    glFlush();
    glutSwapBuffers();
}
int main(int argc, char** argv){ // Initialize stuff
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH,HEIGHT);
    glutCreateWindow("Landscape Raymarcher");

    glMatrixMode(GL_PROJECTION);
    glClearColor(skyR/255,skyB/255,skyG/255,1.0f);
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glLoadIdentity();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
