#include<stdio.h>
#include<map>
#include<array>
#include<vector>
#include <glew.h>
#include <freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

typedef struct{
	float x;
	float y;
} coord;

typedef struct{
	float y;
	bool collide;
} detect;

typedef struct {
	float floor;
	float roof;
} space;

class Phyengine
{
public:

	Phyengine();

	void InsertMap(float triX, float triY, float triZ);
	void GenMap();
	float Gravity(float rawy);
	float SmoothClimb(float rawy);
	coord Walk();
	void SetSpeed(float s);
	vec3 World(vec3 tempCamera, vec3 cameraEyes, bool flying, bool birth);
	void GenTerrainFile(char* filename);
	void LoadTerrainFile(char* filename);
	map<array<int, 2>, vector<space>> GetTerrain();
	float GetHighest();
	void PrintWidthHeight();
	coord GetPersonPosition(float x, float z);

private:
	map<array<float, 2>, vector<float>> heightMap;
	map<array<float, 2>, vector<space>> floorMap;
	map<array<int, 2>, vector<space>> resizedMap;
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
	float speedScale;
	void SortMap();
	float sceneScale = 40;
	detect Collide(vec3 camera);
	float charHeight = 0.2;
};