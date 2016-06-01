#pragma once

typedef __declspec(dllexport) struct _texture_data
{
	_texture_data() : width(0), height(0), data(0) {}
	unsigned int width;
	unsigned int height;
	unsigned char* data;
} texture_data;

__declspec(dllexport) texture_data load_png(const char* path);
__declspec(dllexport) texture_data load_jpg(const char* path);
__declspec(dllexport) void free_texture_data(texture_data data);