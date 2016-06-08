#include<stdio.h>
#include<map>
#include<array>

using namespace std;
typedef struct{
    float x;
    float y;
} coord;

class Phyengine
{
public:
    
    Phyengine();
    
    void InsertMap(float triX, float triY, float triZ);
    void MakeDataFile();
    void GenMap();
    float Collision(float rawx, float rawy, float rawz);
    float Gravity(float rawy);
    float SmoothClimb(float rawy);
    void GenTerrain();
    coord Walk();
    float GetHighest();
    int GetMapHeight();
    int GetMapWidth();
    float GetPositionHeight(int x, int y);
    
private:
    map<array<float, 2>, float> heightMap;
    map<array<int, 2>, float> resizedMap;
    float left = -100000;
    float right = 100000;
    float top = 100000;
    float down = -100000;
    float highest = -100000;
    float gv = 0;
    float climbv = 0;
    bool falling = true;
    coord shake;
    float amplitude = 0.2;
    float t = 0;
    float f = 1;
    void FillMap();
};