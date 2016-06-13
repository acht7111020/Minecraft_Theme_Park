#include "Phyengine.h"
#include<math.h>
#include<stdlib.h>
#include <iostream>

float PI = 3.14159;

float mapScale = 10;

Phyengine::Phyengine() {
	shake.x = 0;
	shake.y = 0;
}


vec3 Phyengine::World(vec3 tempCamera, vec3 cameraEyes, bool flying, bool birth) {
	vec3 camera = cameraEyes;
	detect detector = Collide(tempCamera);
	if (!detector.collide) {
		camera = tempCamera;
		if (camera.y < detector.y) {
			float temp = SmoothClimb(camera.y);
			if (temp < detector.y) {
				camera.y = temp;
			}
			else {
				camera.y = detector.y;
				climbv = 0;
			}
			gv = 0;
		}
		else if (camera.y >= detector.y && !flying) {
			float temp = Gravity(camera.y);
			if (temp > detector.y) {
				camera.y = temp;
			}
			else {
				camera.y = detector.y;
				gv = 0;
			}
		}
	}
	else {
		gv = 0;
	}
	return camera;
}

void Phyengine::InsertMap(float triX, float triY, float triZ) {
	if (triZ > left)
		left = triZ;
	if (triZ < right)
		right = triZ;
	if (triX > down)
		down = triX;
	if (triX < top)
		top = triX;
	if (triY > highest)
		highest = triY;
	array<float, 2> triXZ = { triX, triZ };
	heightMap[triXZ].push_back(triY);
}

int compare(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}

void Phyengine::SortMap() {
	map<array<float, 2>, vector<float>>::iterator it;
	for (it = heightMap.begin(); it != heightMap.end(); it++) {
		qsort(&it->second[0], it->second.size(), sizeof(float), compare);
		space s;
		s.floor = it->second[0];
		for (int i = 0; i < it->second.size() - 1; i++) {
			if (it->second[i + 1] - s.floor < 0.15) {
				s.floor = it->second[i + 1];
			}
			else {
				s.roof = it->second[i + 1];
				floorMap[it->first].push_back(s);
				s.floor = s.roof;
			}
		}
		s.floor = it->second.back();
		s.roof = 100000;
		floorMap[it->first].push_back(s);
	}
}

void Phyengine::GenMap() {
	SortMap();
	map<array<float, 2>, vector<space>>::iterator it;
	for (it = floorMap.begin(); it != floorMap.end(); it++)
	{
		int x = (int)round((it->first[0] - top) * mapScale);
		int z = (int)round((it->first[1] - right) * mapScale);
		array<int, 2> triXZ = { x, z };
		for (int i = 0; i < it->second.size(); i++) {
			resizedMap[triXZ].push_back(it->second[i]);
		}
	}
}

void Phyengine::GenTerrainFile(char* filename) {
	FILE * TFILE;
	TFILE = fopen(filename, "w");
	fprintf(TFILE, "%f %f %f %f %f\n", left, right, top, down, highest);
	map<array<int, 2>, vector<space>>::iterator it;
	for (it = resizedMap.begin(); it != resizedMap.end(); it++) {
		fprintf(TFILE, "%d %d ", it->first[0], it->first[1]);
		for (int i = 0; i < it->second.size(); i++) {
			fprintf(TFILE, "%f %f", it->second[i].floor, it->second[i].roof);
			if (i != it->second.size() - 1)
				fprintf(TFILE, " ");
			else
				fprintf(TFILE, " %f %f\n", -1.0f, -1.0f);
		}
	}
	fclose(TFILE);
}

void Phyengine::LoadTerrainFile(char* filename) {
	FILE *TFILE;
	TFILE = fopen(filename, "r");
	fscanf(TFILE, "%f %f %f %f %f", &left, &right, &top, &down, &highest);
	array<int, 2> position;
	space s;
	while (fscanf(TFILE, "%d %d", &position[0], &position[1]) != EOF) {
		while (1) {
			fscanf(TFILE, "%f %f", &s.floor, &s.roof);
			if (s.floor != -1 && s.roof != -1) {
				resizedMap[position].push_back(s);
			}
			else
				break;
		}
	}
	fclose(TFILE);
}

detect Phyengine::Collide(vec3 camera) {
	detect detector;
	detector.y = camera.y;
	detector.collide = false;
	float floor = -1000;
	for (float i = -0.5; i <= 0.5; i += 0.5) {
		for (float j = -0.5; j <= 0.5; j += 0.5) {
			int x = (int)round((camera.x - top) * mapScale + i);
			int z = (int)round((camera.z - right) * mapScale + j);
			array<int, 2> position = { x, z };
			int k;
			for (k = 0; k < resizedMap[position].size(); k++) {
				if (detector.y - (0.05 - shake.y) >= resizedMap[position][k].floor && detector.y + (0.05 - shake.y) <= resizedMap[position][k].roof) {
					if (resizedMap[position][k].floor + 0.15 - shake.y > floor)
						floor = resizedMap[position][k].floor + 0.15 - shake.y;
					break;
				}
			}
			if (k == resizedMap[position].size()) {
				//printf("collide : %d\n", resizedMap[position].size());
				detector.collide = true;
				gv = 0;
			}
		}
	}
	detector.y = floor;
	return detector;
}

float Phyengine::Gravity(float rawy) {
	gv += 0.0015 * speedScale;
	climbv = 0;
	return rawy - gv;
}

float Phyengine::SmoothClimb(float rawy) {
	climbv += 0.0015 * speedScale;
	return rawy + climbv;
}

coord Phyengine::Walk() {
	shake.x = amplitude * cos(2 * PI * f * t);
	if (shake.x < 0) {
		shake.x += amplitude;
		shake.y = shake.x * 3;
	}
	else {
		shake.x -= amplitude;
		shake.y = -shake.x * 3;
	}
	if (shake.x < amplitude / 5 && shake.x > 0)
		shake.x = amplitude / 5;
	else if (shake.x > -amplitude / 5 && shake.x < 0)
		shake.x = -amplitude / 5;
	t += 0.01 * speedScale;
	shake.y /= sceneScale;
	return shake;
}

void Phyengine::SetSpeed(float s) {
	speedScale = s;
}

map<array<int, 2>, vector<space>> Phyengine::GetTerrain(){
	return resizedMap;
}

float Phyengine::GetHighest(){
	return highest;
}

void Phyengine::PrintWidthHeight(){
	map<array<int, 2>, vector<space>>::reverse_iterator it = resizedMap.rbegin();
	printf("Done.\n");
	printf("%d %d\n", it->first[0], it->first[1]);
}

coord Phyengine::GetPersonPosition(float x, float z){
	coord position;
	position.x = (int)round((x - top) * mapScale);
	position.y = (int)round((z - right) * mapScale);
	return position;
}