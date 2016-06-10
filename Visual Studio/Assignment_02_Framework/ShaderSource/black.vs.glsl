#version 410 core

uniform mat4 um4mvp;
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 iv2texcoor;

out vec2 vv2cor;
void main(void)
{
	gl_Position = vec4(position, 0.0, 1.0);
	vv2cor = iv2texcoor;
}