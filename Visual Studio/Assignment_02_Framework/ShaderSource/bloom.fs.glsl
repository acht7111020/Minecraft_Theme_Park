#version 410 core
in vec2 vv2cor;
out vec4 fragColor;
uniform sampler2D tex;
uniform int shape_id;

int fire_id = 24;
void main()
{
	if(shape_id == fire_id)
		fragColor = texture(tex, vv2cor);
	else
		fragColor = vec4(0, 0, 0, 1);

	//fragColor = vec4(vv2cor, 0.0, 1.0);
}