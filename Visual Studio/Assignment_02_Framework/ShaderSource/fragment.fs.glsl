#version 420

in vec3 vv3color;
in vec3 surfacePos;
in vec3 normal;
in vec4 shadow_coord;
flat in mat4 tranViewMatrix;

layout (binding = 0) uniform sampler2D object_tex; 
layout (binding = 1) uniform sampler2DShadow shadow_tex;
layout(location = 0) out vec4 fragColor;

uniform int changemode;

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
};

struct MaterialParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform MaterialParameters Material;
uniform LightSourceParameters LightSource[3];
uniform int open[3];
uniform mat4 V;
uniform mat4 M;
uniform vec3 cameraPosition;


vec4 ApplyLighting(LightSourceParameters light, vec3 surfacePos, vec3 N, vec3 surfaceToCamera , int spotlight){

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
	
	if(spotlight == 2){
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

	return linearColor;
}


void main()
{
	/* Light formula */

	vec3 surfaceToCamera  = normalize( cameraPosition - surfacePos);
	

	int i = 0;
	vec4 lightColor = vec4(0,0,0,1);
	for(i = 0 ; i < 3 ; i++){
		if(open[i] == 1) lightColor += ApplyLighting(LightSource[i], surfacePos, normalize(normal), surfaceToCamera , i);
	}

	if(changemode == 0){
		vec2 vv2cor = vec2(vv3color);
		//fragColor = vec4(texture(object_tex, vv3color).rgb, 1.0);
		//fragColor = vec4(vv3color, 1);
		vec4 texColor = texture(object_tex, vv2cor).rgba;
		if(texColor.a < 0.5)
			discard;
		float visibility = 0.5;
		if(textureProj(shadow_tex, shadow_coord) > 0.6)
			visibility = 1.0;
		fragColor = vec4(texture(object_tex, vv2cor).rgb, 1.0) * lightColor * visibility;
		//fragColor =  vec4( vec3(textureProj(shadow_tex, shadow_coord)) , 1);
		//fragColor = vec4(texture(object_tex, vv2cor).rgb, 1.0)  lightColor *;
		/*fragColor = lightColor;
		float grayColor = (0.2126*texture_color.r+0.7152*texture_color.g+0.0722*texture_color.b)*0.5;
		fragColor = vec4(grayColor, grayColor, grayColor, 1);*/
	}
	else
		fragColor = vec4(vv3color, 1) ;
}