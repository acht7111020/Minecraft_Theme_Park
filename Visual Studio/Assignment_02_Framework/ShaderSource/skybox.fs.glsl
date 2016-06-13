#version 410 core                          
                                           
uniform samplerCube tex_cubemap;
uniform samplerCube nightMap;
uniform float timeFactor;
                                           
in VS_OUT                                  
{                                          
    vec3 tc;                            
} fs_in;                                   
                                           
layout (location = 0) out vec4 color;      
                                           
void main(void)                            
{                                          
    vec4 color1 = texture(tex_cubemap, fs_in.tc);
    vec4 color2 = texture(nightMap, fs_in.tc);
    color = mix(color1, color2, timeFactor);
}