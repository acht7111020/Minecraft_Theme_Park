#version 410 core
layout (location = 0) in vec2 pos;
layout (location = 1) in float posz;
out vec4 height;
uniform int shapeID;
void main()
{
    gl_Position =  vec4(pos.x, pos.y,0,1);
	if(shapeID != 0)
		height = vec4(vec3(posz),1);
	else
		height = vec4(posz, 0, 0, 1);
}