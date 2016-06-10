//
//  Lighting.cpp
//  Assignment 2 Framework
//
//  Created by Blurry on 2016/5/30.
//  Copyright © 2016年 NTHU. All rights reserved.
//

#include "Lighting.h"
#include <string>
#include <stdio.h>
#include <iostream>

Lighting::Lighting(int lightnumber, int open){
	lightnum = lightnumber;
	lightopen = open;
}

void
Lighting::LightingGetUniformLocation(GLuint p){

	//string lightnumStr;
	cout << lightnum << endl;
	string lightnumStr = to_string(lightnum);

	string lightsource = "LightSource[";
	string title = lightsource + lightnumStr;
	string diffuseStr = title + "].diffuse";
	string ambientStr = title + "].ambient";
	string specularStr = title + "].specular";
	string positionStr = title + "].position";
	string openStr = title + "].open";

	uniformDiffuse = glGetUniformLocation(p, diffuseStr.c_str());
	uniformAmbient = glGetUniformLocation(p, ambientStr.c_str());
	uniformSpecular = glGetUniformLocation(p, specularStr.c_str());
	uniformPosition = glGetUniformLocation(p, positionStr.c_str());
	uniformopen = glGetUniformLocation(p, openStr.c_str());

}

void
Lighting::SpotLightingGetUniformLocation(GLuint p){

	string lightnumStr = to_string(lightnum);

	string lightsource = "LightSource[";
	string title = lightsource + lightnumStr;
	string spotDirectionStr = title + "].spotDirection";
	string spotExponentStr = title + "].spotExponent";
	string spotCutoffStr = title + "].spotCutoff";

	uniformSpotDir = glGetUniformLocation(p, spotDirectionStr.c_str());
	uniformSpotExp = glGetUniformLocation(p, spotExponentStr.c_str());
	uniformSpotCut = glGetUniformLocation(p, spotCutoffStr.c_str());
}

void
Lighting::LightingUniformGiven(){

	glUniform4fv(uniformAmbient, 1, value_ptr(lightsource.ambient));
	glUniform4fv(uniformPosition, 1, value_ptr(lightsource.position));
	glUniform4fv(uniformDiffuse, 1, value_ptr(lightsource.diffuse));
	glUniform4fv(uniformSpecular, 1, value_ptr(lightsource.specular));
	glUniform1i(uniformopen, lightopen);

}

void
Lighting::SpotLightingUniformGiven(){
	glUniform4fv(uniformSpotDir, 1, value_ptr(lightsource.spotDirection));
	glUniform1f(uniformSpotExp, lightsource.spotExponent);
	glUniform1f(uniformSpotCut, lightsource.spotCutoff);

}

void
Lighting::ChangeOpenStatus(){
	lightopen = (lightopen == 1) ? 0 : 1;
}

void
Lighting::SetLight(float* ambient, float* diffuse, float* specular, float* position){

	lightsource.position = vec4(position[0], position[1], position[2], position[3]);
	lightsource.ambient = vec4(ambient[0], ambient[1], ambient[2], ambient[3]);
	lightsource.diffuse = vec4(diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
	lightsource.specular = vec4(specular[0], specular[1], specular[2], specular[3]);
}

void
Lighting::SetSpotLight(vec3 SpotDir, float SpotExp, float SpotCut){

	lightsource.spotDirection = vec4(SpotDir, 1);
	lightsource.spotExponent = SpotExp;
	lightsource.spotCutoff = SpotCut;
}

vec3
Lighting::GetLightPosition(){

	return vec3(lightsource.position);

}

void
Lighting::SetLightPosition(vec3 pos){
	lightsource.position.x += pos.x;
	lightsource.position.y += pos.y;
	lightsource.position.z += pos.z;
}