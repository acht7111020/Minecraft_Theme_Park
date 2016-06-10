#version 410 core

uniform mat4 um4mvp;
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 iv2texcoor;

out vec2 vv2cor;
void main(void)
{
	vv2cor = iv2texcoor;
    gl_Position = um4mvp * vec4(position, 1);
}