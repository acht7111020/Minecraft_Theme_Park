#version 410 core
out vec4 outcolor;
in vec4 height;

void main()
{
    outcolor = height;
}