#version 420

in vec3 vv3color;
in vec3 surfacePos;
in vec3 normal;
in vec4 shadow_coord0;
in vec4 shadow_coord1;
in vec4 shadow_coord2;
in vec4 oriVertex;
in float ClipSpacePosZ;

in vec3 vv3pos;
in vec4 clipSpace;
in vec3 toCameraVector;

flat in mat4 tranViewMatrix;

layout (binding = 0) uniform sampler2DShadow shadow_tex0; 
layout (binding = 1) uniform sampler2DShadow shadow_tex1; 
layout (binding = 2) uniform sampler2DShadow shadow_tex2; 
layout (binding = 3) uniform sampler2D object_tex;
layout (binding = 4) uniform sampler2D reflectionTexture;
layout (binding = 5) uniform sampler2D refractionTexture;
layout (binding = 6) uniform sampler2D dudvMap;
layout (binding = 7) uniform sampler2D normalMap;

uniform float moveFactor;
uniform int shape_id;
uniform int changemode;
const float tiling = 4.0;
const float waveStrength = 0.04;

layout(location = 0) out vec4 fragColor;

struct LightSourceParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 halfVector;
	vec4 spotDirection;
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	int open;
};

struct MaterialParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform MaterialParameters Material;
uniform LightSourceParameters LightSource[5];
uniform mat4 V;
uniform mat4 M;
uniform vec3 cameraPosition;
uniform float fogDensity;
uniform mat4 shadow_matrix[3];
uniform int water_id;
vec2 reflectCoord;
vec2 refractCoord;
vec2 distortedTexCoords;

vec3 WaterLighting(LightSourceParameters light, vec3 surfacePos, vec3 N, vec3 surfaceToCamera , int spotlight){

	vec3 trans_l = vec3( V * light.position); //lightPosition
	vec3 surfaceToLight ; 
	// compute the distance to the light source to a varying variable

	if(light.position.w == 0){
		// direction light
		surfaceToLight  = normalize(trans_l);	//surfaceToLight
		
	}
	else{
		// point light
		surfaceToLight  = normalize(trans_l - surfacePos);
	}
  
	//vec3 R = normalize(reflect(-surfaceToLight ,N));
	vec3 H = normalize(surfaceToLight + surfaceToCamera);

	// Specular 
    // vec4 specular = pow(max(dot(R,surfaceToCamera),0.0), 100.0) * Material.specular * light.specular;
	vec4 specular = pow(max(dot(N, H),0.0), 100.0)  * light.specular;

    //linear color 
	vec4 linearColor =  ( specular )  ;

	return vec3(linearColor);
}

vec3 ApplyLighting(LightSourceParameters light, vec3 surfacePos, vec3 N, vec3 surfaceToCamera , int spotlight){

	vec3 trans_l = vec3( V * light.position); //lightPosition
	vec3 surfaceToLight ; 
	// compute the distance to the light source to a varying variable
	float attenuation;
	float Seff = 1;

	if(light.position.w == 0){
		// direction light
		surfaceToLight  = normalize(trans_l);	//surfaceToLight
		attenuation = 1.0;
		
	}
	else{
		// point light
		surfaceToLight  = normalize(trans_l - surfacePos);
		float dist = length(trans_l - surfacePos);
		attenuation = 1.0 / ( 0.1 + 0.00005 * dist + 0.00000005*pow(dist, 2));
	}
  
	//vec3 R = normalize(reflect(-surfaceToLight ,N));
	vec3 H = normalize(surfaceToLight + surfaceToCamera);

	// calculate Spotlight effect Term:
	
	if(spotlight == 2 || spotlight == 3 || spotlight == 4){
		vec3 D = -normalize( vec3( tranViewMatrix * light.spotDirection ) ) ;
		//if( dot(D, surfaceToLight) > 0.9999)
		//	Seff = 1;
		//else
		//	Seff = 0;
		
		if( degrees(acos(dot( D, surfaceToLight  ))) < light.spotCutoff ) {
			Seff = pow(max(dot(D, surfaceToLight ), 0.0), light.spotExponent);
			//Seff = 5;
		}
		else{
			Seff = 0;
		}
	}
	
	// Diffuse
	vec4 diffuse = max(dot(N,surfaceToLight), 0.0) * Material.diffuse * light.diffuse;

	// Ambient
	vec4 ambient =  Material.ambient * light.ambient;

	// Specular 
    // vec4 specular = pow(max(dot(R,surfaceToCamera),0.0), 100.0) * Material.specular * light.specular;
	vec4 specular = pow(max(dot(N, H),0.0), 100.0) * Material.specular * light.specular;

    //linear color 
	vec4 linearColor = attenuation * (ambient + diffuse + specular ) * vec4( Seff, Seff, Seff, 1) ;

	return vec3(linearColor);
}
float CalcShadowFactor(int CascadeIndex)
{ 
	/*vec4 shadowTexcoord = shadow_matrix[CascadeIndex] * oriVertex;
	shadowTexcoord = shadowTexcoord / shadowTexcoord.w;
	shadowTexcoord = 0.5 * shadowTexcoord + 0.5; 
	float realDepth = shadowTexcoord.z;
	float Depth;
	if(CascadeIndex== 0){
		Depth = textureProj( shadow_tex0, shadowTexcoord);
	}
	else if(CascadeIndex == 1){
		Depth = textureProj( shadow_tex1, shadowTexcoord);
	}else{
		Depth = textureProj( shadow_tex2, shadowTexcoord);
	}*/

	float Depth, factor, z;
	if( CascadeIndex== 0){
		z = shadow_coord0.z; 
		Depth = textureProj( shadow_tex0, shadow_coord0); 
		//factor = 0.4;
	}
	else if(CascadeIndex == 1){
		z = shadow_coord1.z; 
		Depth = textureProj( shadow_tex1, shadow_coord1); 
		//factor = 0.6;
	}else{
		z = shadow_coord2.z; 
		Depth = textureProj( shadow_tex2, shadow_coord2); 
		//factor = 0.8;
	}

    if (Depth < 0.6 + 0.00001) 
        return 0.4;
    else 
        return 1.0; 
} 
float shaodowMulti(){
	/*float sum = 1.0;
	float visibility = 0.6;
	if(textureProj(shadow_tex0, shadow_coord0) > 0.6)
		visibility = 1.0;
	sum *= visibility;*/

	float ShadowFactor = 0.1;
	//float gCascadeEndClipSpace[3] = {500, 1000, 1600};
	float gCascadeEndClipSpace[3] = {500, 800, 1000};
    for (int i = 0 ; i < 3 ; i++) {
        if (ClipSpacePosZ <= gCascadeEndClipSpace[i]) {
            return CalcShadowFactor(i);
            //return ShadowFactor;
        }
    }
	return ShadowFactor;
}

float WaterEffect(vec2 totalDistortion, vec3 normalwater){
	
    /*
	vec2 distortion1 = (texture(dudvMap, vec2(coord.x + moveFactor, coord.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(dudvMap, vec2(-coord.x + moveFactor, coord.y + moveFactor)).rg * 2.0 - 1.0) * waveStrength;
    vec2 totalDistortion = distortion1 + distortion2;
	*/
    vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;
    reflectCoord = vec2(ndc.x, -ndc.y);
    refractCoord = vec2(ndc.x, ndc.y);

    reflectCoord += totalDistortion;
    refractCoord += totalDistortion;
    
    reflectCoord.x = clamp(reflectCoord.x, 0.001, 0.999);
    reflectCoord.y = clamp(reflectCoord.y, -0.999, -0.001);
    refractCoord = clamp(refractCoord, 0.001, 0.999);
    
    vec3 viewVector = normalize(toCameraVector);
    float refractFactor = dot(viewVector, vec3(0, 1, 0));
    refractFactor = pow(refractFactor, 4.0);
	
	return refractFactor ;
}

float rand(vec2 n)
{
  return 0.5 + 0.5 * 
     fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

float FogColor(){
	
	float fogFactor ;
	float dist = length(vec4(surfacePos, 1));
	if(dist < 100) return 1;
	fogFactor = 2.0/pow(exp(dist * fogDensity),0.05);
	fogFactor = clamp(fogFactor, 0, 1);
	return fogFactor;
}

void main()
{
	/* Light formula */

	vec3 surfaceToCamera  = normalize( cameraPosition - surfacePos);
	

	int i = 0;
	vec3 lightColor = vec3(0);
	for(i = 0 ; i < 5 ; i++){
		if(LightSource[i].open == 1) lightColor += ApplyLighting(LightSource[i], surfacePos, normalize(normal), surfaceToCamera , i);
	}

	if(changemode == 0){
		
		vec2 vv2cor = vv3color.xy;
		vec4 color = texture(object_tex, vv2cor);

		
		if(shape_id == water_id){
			vec2 coord = vec2(vv3pos.x/2.0 + 0.5, vv3pos.z/2.0 + 0.5) * tiling;
			distortedTexCoords = texture(dudvMap, vec2(coord.x + moveFactor, coord.y)).rg*0.1;
			distortedTexCoords = coord + vec2(distortedTexCoords.x, distortedTexCoords.y+moveFactor);
			vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;
			/*calculate water normal*/
			vec4 normalMapColor = texture(normalMap, distortedTexCoords);
			vec3 normalwater = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
			normalwater = normalize(normalwater);

			float refractFactor = WaterEffect(totalDistortion, normalwater);
			
			lightColor = vec3(0);
			
			lightColor += WaterLighting(LightSource[0], surfacePos, normalwater, surfaceToCamera , 0);
			
			vec4 reflectColor = texture(reflectionTexture, reflectCoord);
			vec4 refractColor = texture(refractionTexture, refractCoord);
			fragColor =  mix(reflectColor, refractColor, refractFactor);
			if(rand(vv3pos.xz) > 0.75)
				fragColor = mix(fragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(lightColor, 0.0);
			else
				fragColor = mix(fragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2);
		}
		else{
			
			if(color.a < 0.5)
				discard;
			float visibility = shaodowMulti();
			//fragColor = color * vec4(lightColor,1) ;//* vec4(vec3(visibility), 1);

			fragColor = color * vec4(lightColor,1) * vec4(vec3(visibility), 1);
			//fragColor =  vec4( vec3(textureProj(shadow_tex0, shadow_coord0) ) , 1);
		}
		//fragColor =  vec4( vec3(visibility) , 1);
		//fragColor = vec4(texture(object_tex, vv2cor).rgb, 1.0)  lightColor *;

	}
	else
		fragColor = vec4(vv3color, 1) ;

	vec4 fogColor = vec4(0.5, 0.5, 0.5, 1.0);
	float fogFactor = FogColor();
	fragColor = mix(fogColor, fragColor, fogFactor);
}