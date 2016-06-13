#include"Camera.h"
#include<math.h>

#define PI 3.1415926

Camera::Camera(char * filename){
	LoadData(filename);
}

void Camera::LoadData(char * filename){
	FILE* p;
	p = fopen(filename, "r");
	Roller_data tmp, prev;
	fscanf(p, "%f%f%f%f%f%f", &tmp.eye.x, &tmp.eye.y, &tmp.eye.z, &tmp.track.x, &tmp.track.y, &tmp.track.z);
	b_data.push_back(tmp);
	prev = tmp;
	int x = 1;
	while (fscanf(p, "%f%f%f%f%f%f", &tmp.eye.x, &tmp.eye.y, &tmp.eye.z, &tmp.track.x, &tmp.track.y, &tmp.track.z) != EOF){
		while (tmp.track.x - prev.track.x > 180)
			tmp.track.x -= 360;
		while (tmp.track.x - prev.track.x < -180)
			tmp.track.x += 360;
		while (tmp.track.y - prev.track.y > 180)
			tmp.track.y -= 360;
		while (tmp.track.y - prev.track.y < -180)
			tmp.track.y += 360;
		while (tmp.track.z - prev.track.z > 180)
			tmp.track.z -= 360;
		while (tmp.track.z - prev.track.z < -180)
			tmp.track.z += 360;
		b_data.push_back(tmp);
		prev = tmp;
	}
	fclose(p);
}

vec3 Camera::Bezier(float t, vec3 P0, vec3 P1, vec3 P2) {
	vec3 v;
	v.x = (P0.x)*(1 - t)*(1 - t) + 2 * (P1.x)*t*(1 - t) + (P2.x)*t*t;
	v.y = (P0.y)*(1 - t)*(1 - t) + 2 * (P1.y)*t*(1 - t) + (P2.y)*t*t;
	v.z = (P0.z)*(1 - t)*(1 - t) + 2 * (P1.z)*t*(1 - t) + (P2.z)*t*t;
	return v;
}

Roller_data Camera::RollerPlay(int mode, float speedScale){
	Roller_data RD;
	float d = sqrt(pow(b_data[b_index + 2].eye.x - b_data[b_index].eye.x, 2) + pow(b_data[b_index + 2].eye.y - b_data[b_index].eye.y, 2) + pow(b_data[b_index + 2].eye.z - b_data[b_index].eye.z, 2));
	b_timer += accelerate / d * speedScale;
	if (b_timer > 1) {
		b_timer -= round(b_timer);
		b_index = b_index + 2;
	}
	if (b_index + 2 >= b_data.size()){
		RD.eye.x = -10000;
		return RD;
	}
	RD.eye = Bezier(b_timer, b_data[b_index].eye, b_data[b_index + 1].eye, b_data[b_index + 2].eye);
	RD.track = Bezier(b_timer, b_data[b_index].track, b_data[b_index + 1].track, b_data[b_index + 2].track);

	if (accelerate < 0.3)
		accelerate = 0.3;
	printf("%f\n", RD.track.x);
	accelerate += RD.track.x * 0.0002;
	if (mode == 0){
		if (b_index == 6)
			accelerate -= 0.02;
		else if (b_index >= 8 && b_index <= 12)
			accelerate += 0.01;
		else if (b_index == 28)
			accelerate -= 0.02;
		else if (b_index == 30)
			accelerate += 0.015;
		else if (b_index == 108)
			accelerate -= 0.01;
		else if (b_index == 110)
			accelerate -= 0.02;
		else if (b_index >= 114 && b_index <= 116)
			accelerate += 0.02;
		else if (b_index >= 118 && b_index <= 120)
			accelerate += 0.01;
		else if (b_index >= 134 && b_index <= 136)
			accelerate += 0.01;
		else if (b_index >= 138 && b_index <= 140)
			accelerate -= 0.02;
		else if (b_index >= 144)
			accelerate += 0.02;
		else if (b_index >= 152 && b_index <= 154)
			accelerate += 0.02;
	}
	else{
		if (b_index >= 0 && b_index <= 6)
			accelerate -= 0.02;
	}

	printf("%d\n", b_index);
	return RD;
}

void Camera::Restart(){
	b_index = 0;
	b_timer = 0;
	accelerate = 0;
}

void Camera::SetSpeed(float a){
	accelerate = a;
}

float Camera::GetSpeed(){
	return accelerate;
}

int Camera::GetBIndex(){
	return b_index;
}