#version 420

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2texcoor;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mvp;
uniform mat4 V;
uniform mat4 M;
uniform int changemode;
uniform mat4 shadow_matrix;

out vec3 vv3color;
out vec3 surfacePos;
out vec3 normal;
out vec4 shadow_coord;
flat out mat4 tranViewMatrix;

void main()
{

	surfacePos = vec3(V * M * vec4(iv3vertex, 1)); //surfacePosition 
	normal = vec3( transpose(inverse(V*M)) * vec4(iv3normal, 1.0 ) );
	tranViewMatrix = transpose(inverse(V));
	shadow_coord = shadow_matrix * vec4(iv3vertex,1);

	gl_Position = um4mvp * vec4(iv3vertex, 1.0);
	if(changemode == 0)
		vv3color = vec3(iv2texcoor, 1);
	else
		vv3color = iv3normal;

	
}