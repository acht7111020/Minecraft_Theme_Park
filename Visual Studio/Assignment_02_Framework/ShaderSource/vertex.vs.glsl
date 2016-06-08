#version 420

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2texcoor;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mvp;
uniform mat4 V;
uniform mat4 M;
uniform int changemode;
uniform mat4 shadow_matrix0;
uniform mat4 shadow_matrix1;
uniform mat4 shadow_matrix2;
uniform vec3 cameraPosition;
uniform vec4 plane;

out vec3 vv3color;
out vec3 surfacePos;
out vec3 normal;
out vec4 shadow_coord0;
out vec4 shadow_coord1;
out vec4 shadow_coord2;
out float ClipSpacePosZ;

//water
out vec3 vv3pos;
out vec3 toCameraVector;
out vec4 clipSpace;

flat out mat4 tranViewMatrix;

void main()
{

	surfacePos = vec3(V * M * vec4(iv3vertex, 1)); //surfacePosition 
	normal = vec3( transpose(inverse(V*M)) * vec4(iv3normal, 1.0 ) );
	tranViewMatrix = transpose(inverse(V));
	shadow_coord0 = shadow_matrix0 * vec4(iv3vertex,1);
	shadow_coord1 = shadow_matrix1 * vec4(iv3vertex,1);
	shadow_coord2 = shadow_matrix2 * vec4(iv3vertex,1);
	clipSpace = um4mvp * vec4(iv3vertex, 1.0);
	gl_Position = clipSpace;
	if(changemode == 0)
		vv3color = vec3(iv2texcoor, 1);
	else
		vv3color = iv3normal;

	ClipSpacePosZ = gl_Position.z;

	
    vec4 worldPos = M * vec4(iv3vertex, 1.0);

    gl_ClipDistance[0] = dot(worldPos, plane);
    vv3pos = iv3vertex;
    toCameraVector = cameraPosition - worldPos.xyz;
}