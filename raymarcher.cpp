// Landscape Raymarcher attempt - port of scratch project 843305852
#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;
// Settings - these are changeable, try some combinations out!
const float grassDensity=0.6,epsilon=0.01,skyR=140,skyG=220,skyB=255,fogVal=5000,waterHeight=250,lightDiffuse=5000,fov=240,renderDistance=1000,seed1=12.9898,seed2=78.233;

int WIDTH=480,HEIGHT=360;

// Camera position
const float camX=1420,camY=350,camZ=-250,camRotX=-15,camRotY=160;
const float sinA=sin(camRotX),sinB=sin(camRotY),cosA=cos(camRotX),cosB=cos(camRotY);
float r,reflectivity,temp1,temp2,brightR,brightG,brightB,distX,distY,distZ,dist,rayDirX,rayDirY,rayDirZ,rayX,rayY,rayZ,SDF,rayLen,noise,dot,SmoothStep,surfaceR,surfaceG,surfaceB,height,hitX,hitY,hitZ,normalX,normalY,normalZ,finalR,finalG,finalB,fog,reflectionR,reflectionG,reflectionB;
bool hit,stats,hitGround;

void calcSmoothStep(float n1,float n2,float k){ // not the issue
    SmoothStep=n1+((k*k*(3-(2*k)))*(n2-n1));
}
void calcNoise(float x, float y, float s,float h){// not the issue
    dot=(ceil (x/s)*s*seed1)+(floor(y/s)*s*seed2);
    temp1=fmod(sin(dot)*1000,1);
    dot=(ceil (x/s)*s*seed1)+(ceil (y/s)*s*seed2);
    calcSmoothStep(temp1,fmod(sin(dot)*1000,1),fmod(y,s)/s);
    temp2=SmoothStep;
    dot=(floor(x/s)*s*seed1)+(floor(y/s)*s*seed2);
    temp1=fmod(sin(dot)*1000,1);
    dot=(floor(x/s)*s*seed1)+(ceil (y/s)*s*seed2);
    calcSmoothStep(temp1,fmod(sin(dot)*1000,1),fmod(y,s)/s);
    calcSmoothStep(SmoothStep,temp2,fmod(x,s)/s);
    noise+=SmoothStep*h;
}

void sceneSDF(float x,float y,float z){ // Calculates the scene. Issue lies somewhere here...
    SDF=1e+100;
    noise=0;
    calcNoise(x,z,700,450); // Generate terrain by layering different noise things over eachother
    calcNoise(x,z,250,200);
    calcNoise(x,z,120,50 );
    calcNoise(x,z,50 ,25 );
    calcNoise(x,z,20 ,10 );
    calcNoise(x,z,10 ,3  );
    calcNoise(x,z,3  ,1.5);
    calcNoise(x,z,1  ,0.5);
    if(y-noise<SDF){ // Checks if SDF should be modified
        SDF=y-noise;
        if(SDF<epsilon&&stats){ // Checks if the ray intersects with the terrain.
            surfaceR=120;
            surfaceG=120;
            surfaceB=120;
            hitGround=true;
        }
    }
    height=noise;
    dot=y-waterHeight;
    if(dot<SDF){
        SDF=dot;
        if(SDF<epsilon&&stats){
            surfaceR=60;
            surfaceG=120;
            surfaceB=200;
            reflectivity=0.6;
        }
    }
}
void marchRay(float x,float y,float z,float dx,float dy,float dz,int renderDistance){ // ... Marches the ray
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
    distX=lx-pz;
    distY=ly-py;
    distZ=lz-px;
    dist=sqrt((distX*distX)+(distY*distY)+(distZ*distZ));
    dot=(normalX*(distX/dist))+(normalY*(distY/dist))+(normalZ*(distZ/dist));
    if(dot>0){
        brightR=brightness*r*dot;
        brightG=brightness*g*dot;
        brightB=brightness*b*dot;
    }
}
void colorGround(){
    if(hitGround){
        dot=normalY;
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
    distX=normalX;
    distY=normalY;
    distZ=normalZ;
    dist=sqrt((distX*distX)+(distY*distY)+(distZ*distZ));
    normalX/=dist;
    normalY/=dist;
    normalZ/=dist;
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
    dist=0;
    distX=x;
    distY=y;
    distZ=fov;
    dist=sqrt((distX*distX)+(distY*distY)+(distZ*distZ));
    rayDirY=((distZ/dist)*sinA)+((distY/dist)*cosA);
    rayDirZ=((distZ/dist)*cosA)-((distY/dist)*sinA);
    rayDirX=(rayDirZ*sinB)+((distX/dist)*cosA);
    rayDirZ=(rayDirZ*cosB)-((distX/dist)*cosA);
    marchRay(camX,camY,camZ,rayDirX,rayDirY,rayDirZ,1000); //x y z dx dy dz renderdistance
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
        }
        finalR=((1-r)*finalR)+(r*reflectionR);
        finalG=((1-r)*finalG)+(r*reflectionG);
        finalB=((1-r)*finalB)+(r*reflectionB);
        applyFog();
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
    glClearColor(skyR/255,skyG/255,skyB/255,1.0f);
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glLoadIdentity();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
