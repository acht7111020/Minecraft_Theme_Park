#version 410 core
in vec2 vv2cor;
out vec4 fragColor;
uniform sampler2D tex;
void main()
{
	fragColor = texture(tex, vv2cor);

	if(fragColor.x >= 1.0)
		fragColor = vec4(0.0,0.0,0.0,1.0);
}