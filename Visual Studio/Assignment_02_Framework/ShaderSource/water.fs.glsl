#version 410 core

//uniform sampler2D reflectionTexture;
//uniform sampler2D refractionTexture;
uniform sampler2D tex;
out vec4 color;

in VS_OUT {
    vec2 texcoord;
} fs_in;

void main(){
//    vec4 reflectColor = texture(reflectionTexture, fs_in.texcoord);
//    vec4 refractColor = texture(refractionTexture, fs_in.texcoord);
//    color = mix(reflectColor, refractColor, 0.5);
    color = texture(tex, fs_in.texcoord);
    color = mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.5);
}