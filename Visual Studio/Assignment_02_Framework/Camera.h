#include<stdio.h>
#include<vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace glm;
using namespace std;


typedef struct{
    vec3 eye;
    vec3 track;
}Roller_data;

class Camera
{
public:
	Camera(char * filename);
    vec3 Bezier(float t, vec3 P0, vec3 P1, vec3 P2);
    Roller_data RollerPlay(int mode, float speedScale);
	Roller_data GetStartPoint();
	void Restart();
	void SetSpeed(float a);
	float GetSpeed();
	void SetSpeedScale(float s);
	int GetBIndex();
    
private:
    vector<Roller_data> b_data;
	void LoadData(char * filename);
    float accelerate = 0;
    int b_index = 0;
    float b_timer = 0;
};