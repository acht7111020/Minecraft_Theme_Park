//
//  Lighting.hpp
//  Assignment 2 Framework
//
//  Created by Blurry on 2016/5/30.
//  Copyright © 2016年 NTHU. All rights reserved.
//

#include <glew.h>
#include <freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>

using namespace glm;
using namespace std;

struct LightSourceParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 spotDirection;
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
}typedef LightSource;

/*

Use : Lighting light1 = new Lighting(0, vec4(0.6f, 0.6f, 0.6f, 1), vec4(0.6f, 0.6f, 0.6f, 1), vec4(1, 1, 1, 1),
vec4(-73.425728f, 358.380005f, -19.493023f, 1), 1);
//LightIndex, ambient, diffuse, specular, position, OpenOrNot
*/


class Lighting{

public:
	Lighting(int lightnumber, int open);
	//~Lighting();

	void LightingGetUniformLocation(GLuint p);
	void SpotLightingGetUniformLocation(GLuint p);
	void LightingUniformGiven();
	void SpotLightingUniformGiven();
	void ChangeOpenStatus();
	void ChangeOpenStatus(int i);
	void SetLight(float* ambient, float* diffuse, float* specular, float* position);
	void SetSpotLight(vec3 SpotDir, float SpotExp, float SpotCut);
	vec3 GetLightPosition();
	void SetLightPosition(vec3 pos);
	void SetLightAmbient(vec3 a);
	vec3 getLightAmbient();
private:
	int lightnum;
	int lightopen;
	LightSource lightsource;

	GLuint uniformDiffuse;
	GLuint uniformAmbient;
	GLuint uniformSpecular;
	GLuint uniformPosition;
	GLuint uniformopen;

	//Spot light
	GLuint uniformSpotDir;
	GLuint uniformSpotCut;
	GLuint uniformSpotExp;
	GLuint uniformSpotCutCos;

};
