#include "Phyengine.h"
#include<math.h>
#include<stdlib.h>

float PI = 3.14159;

float scale = 10;

Phyengine::Phyengine(){
	printf("phyengine say hi\n");
	shake.x = 0;
	shake.y = 0;
}

void Phyengine::InsertMap(float triX, float triY, float triZ){
    if(triZ > left)
        left = triZ;
    
    if(triZ < right)
        right = triZ;
    
    if(triX > down)
        down = triX;
    if(triX < top)
        top = triX;
    if(triY > highest)
        highest = triY;
    array<float, 2> triXZ = {triX, triZ};
    if(heightMap.find(triXZ) != heightMap.end()){
        if(triY > heightMap[triXZ]){
            heightMap[triXZ] = triY;
        }
    }
    else{
        heightMap[triXZ] = triY;
    }
}
void Phyengine::GenMap(){
    map<array<float, 2>, float>::iterator it;
    for (it = heightMap.begin(); it != heightMap.end(); it++)
    {
        int x = (int)round((it->first[0] - top) * scale);
        int z = (int)round((it->first[1] - right) * scale);
        array<int, 2> triXZ = {x, z};
        if(resizedMap.find(triXZ) != resizedMap.end()){
            if(it->second > resizedMap[triXZ])
                resizedMap[triXZ] = it->second;
        }
        else
            resizedMap[triXZ] = it->second;
    }
    //FillMap();
}

void Phyengine::FillMap(){
    /*for(int i = 0; i < MAP_HEIGHT; i++){
        for(int j = 0; j < MAP_WIDTH; j++){
            array<int, 2> pixel = {i, j};
            if(resizedMap[pixel] == 0){
                float sum = 0;
                for(int k = -1; k <= 1; k++){
                    for(int l = -1; l <= 1; l++){
                        array<int, 2> temp = {k, j};
                        sum += resizedMap[temp];
                    }
                }
                resizedMap[pixel] = sum /= 9;
            }
        }
    }*/
}

float Phyengine::Collision(float rawx, float rawy, float rawz){
    float height = rawy;
    for(float i = -0.5; i <= 0.5; i+=0.5){
        for(float j = -0.5; j <= 0.5; j+=0.5){
            int x = (int)round((rawx - top ) * scale + i);
            int z = (int)round((rawz - right) * scale + j);
            array<int, 2> position = {x, z};
            if(resizedMap[position] > height){
                height = resizedMap[position];
                falling = false;
            }
            else{
                if(!falling)
                    gv = 0;
                falling = true;
            }
        }
    }
    int x = (int)round((rawx - top ) * scale);
    int z = (int)round((rawz - right) * scale);
    array<int, 2> position = {x, z};
    
    //printf("%d %d: %f %f %f\n", x, z, rawy, height, resizedMap[position]);
    //printf("%f\n", height);
    return height;
}

float Phyengine::Gravity(float rawy){
    gv += 0.03;
    climbv = 0;
    return rawy - gv;
}

float Phyengine::SmoothClimb(float rawy){
    climbv += 0.03;
    return rawy + climbv;
}

coord Phyengine::Walk(){
    shake.x = amplitude * cos(2 * PI * f * t);
    if(shake.x < 0){
        shake.x += amplitude;
        shake.y = shake.x*3;
    }
    else{
        shake.x -= amplitude;
        shake.y = -shake.x*3;
    }
    if(shake.x < amplitude / 5 && shake.x > 0)
        shake.x = amplitude / 5;
    else if(shake.x > -amplitude / 5 && shake.x < 0)
        shake.x = -amplitude / 5;
    t+=0.02;
    return shake;
}

void Phyengine::MakeDataFile(){
    /*
    map<array<int, 2>, float>::iterator it;
    freopen("map.txt", "w", stdout);
    for(it = resizedMap.begin(); it != resizedMap.end(); it++){
        printf("%d %d: %f\n", it->first[0], it->first[1], it->second);
    }
    fclose(stdout);*/
}

void Phyengine::GenTerrain(){
    map<array<int, 2>, float>::reverse_iterator it = resizedMap.rbegin();
    int height = it->first[0];
    int width = it->first[1];
    printf("Done.\n");
    printf("%d %d\n", height, width);
}

float Phyengine::GetHighest(){
    return highest;
}

int Phyengine::GetMapHeight(){
    return resizedMap.rbegin()->first[0];
}

int Phyengine::GetMapWidth(){
    return resizedMap.rbegin()->first[1];
}

float Phyengine::GetPositionHeight(int x, int y){
    array<int, 2> pos = {x, y};
    //printf("%f\n", resizedMap[pos]);
    return resizedMap[pos];
}