//#include <glew.h>
//#include <freeglut.h>
#define GLM_SWIZZLE

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tiny_obj_loader.h>
#include "fbxloader.h"
#include <IL/il.h>
#include <texture_loader.h>
#include "Lighting.h"
/*
	Bonus part (Embossion, Constant)
	https://github.com/yulu/GLtext

	Ripple : http://www.geeks3d.com/20110316/shader-library-simple-2d-effects-sphere-and-ripple-in-glsl/

	water color : https://github.com/kashimAstro/WaterColor

	Swirl : http://www.geeks3d.com/20110428/shader-library-swirl-post-processing-filter-in-glsl/

	Frost Glass : http://www.geeks3d.com/20101228/shader-library-frosted-glass-post-processing-shader-glsl/


*/

#define DEPTH_TEXTURE_SIZE 4096
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define Shader_Blur 4
#define Shader_Quantization 5
#define Shader_DoG 6
#define Shader_CombineBasic 7
#define Shader_RedBlue 8
#define Shader_Sharpness 9
#define Shader_Laplacian 10
#define Shader_Bloom 11
#define Shader_Magnifier 12
#define Shader_Pixel 13
#define Shader_Constant 14
#define Shader_Embossion 15
#define Shader_WaterColor 16
#define Shader_Swing 17
#define Shader_Ripple 18
#define Shader_FrostedGlass 19
#define Shader_CompareONOFF 20
#define Shader_Oilpainting 21
#define Shader_Ink 22

#define NUM_DRAWS 1

static const GLfloat window_positions[] =
{
	1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

GLuint          program_ShadowDepth;
GLuint          program_RenderScene;
GLuint			program_FrameBuffer;
GLuint          window_vao;
GLuint          buffer;
GLuint			window_buffer;
GLuint			FBO;
GLuint			depthRBO;
GLuint	FBODataTexture;
GLuint	NoiseTexture0, NoiseTexture1;

/*Image Processing*/
GLint  texLoc0, texLoc1;
int showSelect = 0, magflag = 0, comflag = 0;
float compareBarX = 0.5;
float image_size[2] = { 1200, 720 };
float mouse_position[2] = { 1200, 720 };

/*Global Control Block*/
struct MouseButton{
	GLfloat start[2];
	GLfloat end[2];
	int flag = 0;
} mouseLeft, mouseCompare;
float timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
int selectModel = 1, changing = 0; vec3 rotate_vector;
vec3 cameraEyes, cameraCenter, upVector;
mat4 mvp;
GLint um4mvp, changeMode;
mat4 CameraProjectionMatrix, CameraViewMatrix;
bool finish_without_update = false;

/*Lighting*/
struct MaterialParameters {
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float shininess;
};
GLint iLocModel, iLocView;
GLint iLocMDiffuse, iLocMAmbient, iLocMSpecular, iLocMShininess;
GLint iCameraPosition;
Lighting *pointlight, *directionlight, *spotlight;

/*Shadow*/
GLuint uniforms_light_mvp, shadow_tex, shadow_matrix, object_tex;
GLint Shader_now_Loc, Shader_Image_size, Shader_Compare, Shader_mouseposition, Shader_magflag, Shader_time, Shader_comflag;
GLuint          depth_fbo;
GLuint          depth_tex;

/*Load Objects*/
struct OBJ_SHADER {
	vector<float> positions;
	vector<float>  normals;
	vector<float>  texcoords;
	vector<unsigned int> indices;
	vector<int> materialID;
	vector<int> materialIterator;
	GLuint vao;
	GLuint buffer[3];
	GLuint ele_buffer;
	int indexSize;
};
vector<vector<OBJ_SHADER>> models;
vector<GLuint*> texIndex;
vector<fbx_handles> myFbx;
vector<vector<MaterialParameters> > LightingMaterials;

/*Indirect*/
struct DrawElementsIndirectCommand
{
	GLuint  count;
	GLuint  primCount;
	GLuint  first;
	GLint   baseVertex;
	GLuint  baseInstance;
};
GLuint              indirect_draw_buffer;
GLuint              draw_index_buffer;
vector<unsigned int> firsts;
vector<unsigned int> indexCounts;
vector<int> baseVertices;
int shapeCount;

/*skybox*/
GLuint skybox_program;
GLuint skybox_texture;
vector<string> faces;
GLuint skybox_vao;

mat4 Scale(GLfloat a, GLfloat b, GLfloat c){
	mat4 temp1 = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		0, 0, 0, 1,
	};

	return temp1;
}
mat4 Translate(GLfloat a, GLfloat b, GLfloat c){
	mat4 temp1 = {
	1, 0, 0, a,
	0, 1, 0, b,
	0, 0, 1, c,
	0, 0, 0, 1,
};

	return temp1;
}
mat4 Rotate(GLfloat a, GLfloat b, GLfloat c){
	mat4 temp1 = {
		1, 0, 0, 0,
		0, cos(a), -sin(a), 0,
		0, sin(a), cos(a), 0,
		0, 0, 0, 1
	};
	mat4 temp2 = {
		cos(b), 0, sin(b), 0,
		0, 1, 0, 0,
		-sin(b), 0, cos(b), 0,
		0, 0, 0, 1
	};
	mat4 temp3 = {
		cos(c), -sin(c), 0, 0,
		sin(c), cos(c), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return temp3*temp2*temp1;
}
void checkError(const char *functionName)
{
    GLenum error;
    while (( error = glGetError() ) != GL_NO_ERROR) {
        fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
    }
}

// Print OpenGL context related information.
void dumpInfo(void)
{
    printf("Vendor: %s\n", glGetString (GL_VENDOR));
    printf("Renderer: %s\n", glGetString (GL_RENDERER));
    printf("Version: %s\n", glGetString (GL_VERSION));
    printf("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
}

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

void shaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete errorLog;
	}
}

void LoadSkybox() {
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");

	glGenTextures(1, &skybox_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);

	for (int face = 0; face < 6; face++)
	{
		printf("%s\n", faces[face].c_str());
		texture_data tex = load_jpg(faces[face].c_str());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data);
		//if(tex.data != NULL)
		//	delete[] tex.data;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenVertexArrays(1, &skybox_vao);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void My_Init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/*skybox program init*/
	skybox_program = glCreateProgram();
	GLuint vertexShader3 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader3 = glCreateShader(GL_FRAGMENT_SHADER);

	char** vertexShaderSource3 = loadShaderSource("ShaderSource/skybox.vs.glsl");
	char** fragmentShaderSource3 = loadShaderSource("ShaderSource/skybox.fs.glsl");

	glShaderSource(vertexShader3, 1, vertexShaderSource3, NULL);
	glShaderSource(fragmentShader3, 1, fragmentShaderSource3, NULL);
	freeShaderSource(vertexShaderSource3);
	freeShaderSource(fragmentShaderSource3);
	glCompileShader(vertexShader3);
	glCompileShader(fragmentShader3);
	shaderLog(vertexShader3);
	shaderLog(fragmentShader3);
	glAttachShader(skybox_program, vertexShader3);
	glAttachShader(skybox_program, fragmentShader3);
	glLinkProgram(skybox_program);
	glUseProgram(skybox_program);
	LoadSkybox();

	/*shadow depth program init*/
	program_ShadowDepth = glCreateProgram();
	GLuint vertexShader0 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader0 = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource0 = loadShaderSource("ShaderSource/depth.vs.glsl");
	char** fragmentShaderSource0 = loadShaderSource("ShaderSource/depth.fs.glsl");
	glShaderSource(vertexShader0, 1, vertexShaderSource0, NULL);
	glShaderSource(fragmentShader0, 1, fragmentShaderSource0, NULL);
	freeShaderSource(vertexShaderSource0);
	freeShaderSource(fragmentShaderSource0);
	glCompileShader(vertexShader0);
	glCompileShader(fragmentShader0);
	shaderLog(vertexShader0);
	shaderLog(fragmentShader0);
	glAttachShader(program_ShadowDepth, vertexShader0);
	glAttachShader(program_ShadowDepth, fragmentShader0);
	glLinkProgram(program_ShadowDepth);
	uniforms_light_mvp = glGetUniformLocation(program_ShadowDepth, "mvp");

	/*render scene program init*/
    program_RenderScene = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("ShaderSource/vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("ShaderSource/fragment.fs.glsl");
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
	shaderLog(vertexShader);
    shaderLog(fragmentShader);
	glAttachShader(program_RenderScene, vertexShader);
	glAttachShader(program_RenderScene, fragmentShader);
	glLinkProgram(program_RenderScene);
	um4mvp = glGetUniformLocation(program_RenderScene, "um4mvp");
	changeMode = glGetUniformLocation(program_RenderScene, "changemode");
	iLocModel = glGetUniformLocation(program_RenderScene, "M");
	iLocView = glGetUniformLocation(program_RenderScene, "V");

	iLocMDiffuse = glGetUniformLocation(program_RenderScene, "Material.diffuse");
	iLocMAmbient = glGetUniformLocation(program_RenderScene, "Material.ambient");
	iLocMSpecular = glGetUniformLocation(program_RenderScene, "Material.specular");
	iLocMShininess = glGetUniformLocation(program_RenderScene, "Material.shininess");

	pointlight->LightingGetUniformLocation(program_RenderScene);
	directionlight->LightingGetUniformLocation(program_RenderScene);
	spotlight->LightingGetUniformLocation(program_RenderScene);
	spotlight->SpotLightingGetUniformLocation(program_RenderScene);

	iCameraPosition = glGetUniformLocation(program_RenderScene, "cameraPosition");
	shadow_tex = glGetUniformLocation(program_RenderScene, "shadow_tex");
	object_tex = glGetUniformLocation(program_RenderScene, "object_tex");
	shadow_matrix = glGetUniformLocation(program_RenderScene, "shadow_matrix");

	/*frame buffer program init*/
	program_FrameBuffer = glCreateProgram();
	GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource2 = loadShaderSource("ShaderSource/framebuffer.vs.glsl");
	char** fragmentShaderSource2 = loadShaderSource("ShaderSource/framebuffer.fs.glsl");
	glShaderSource(vertexShader2, 1, vertexShaderSource2, NULL);
	glShaderSource(fragmentShader2, 1, fragmentShaderSource2, NULL);
	freeShaderSource(vertexShaderSource2);
	freeShaderSource(fragmentShaderSource2);
	glCompileShader(vertexShader2);
	glCompileShader(fragmentShader2);
	shaderLog(vertexShader2);
	shaderLog(fragmentShader2);
	glAttachShader(program_FrameBuffer, vertexShader2);
	glAttachShader(program_FrameBuffer, fragmentShader2);
	glLinkProgram(program_FrameBuffer);
	Shader_now_Loc = glGetUniformLocation(program_FrameBuffer, "shader_now");
	Shader_Image_size = glGetUniformLocation(program_FrameBuffer, "img_size");
	Shader_Compare = glGetUniformLocation(program_FrameBuffer, "CompareBarX");
	Shader_mouseposition = glGetUniformLocation(program_FrameBuffer, "mouse_pos");
	Shader_magflag = glGetUniformLocation(program_FrameBuffer, "magflag");
	Shader_comflag = glGetUniformLocation(program_FrameBuffer, "comflag");
	Shader_time = glGetUniformLocation(program_FrameBuffer, "time");

	texLoc0 = glGetUniformLocation(program_FrameBuffer, "tex");
	texLoc1 = glGetUniformLocation(program_FrameBuffer, "noise_map");

	/*Frame buffer*/
	glGenVertexArrays(1, &window_vao);
	glBindVertexArray(window_vao);
	glGenBuffers(1, &window_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, window_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)* 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)* 4, (const GLvoid*)(sizeof(GL_FLOAT)* 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenFramebuffers(1, &FBO);

	//depth buffer
	glGenFramebuffers(1, &depth_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*alpha blend*/
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void My_LoadModels(char* filename, int flag)
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	vector<MaterialParameters> thieObjMat;
	fbx_handles myFbxTmp;
	
	std::string err;
	vector<OBJ_SHADER> objects;
	GLuint* subtextureINDEX;
	
	bool ret;
	if(flag == 1)
		ret = tinyobj::LoadObj(shapes, materials, err, filename);
	else{
		ret = LoadFbx(myFbxTmp, shapes, materials, err, filename);
		myFbx.push_back(myFbxTmp);
	}
	
	if(ret)
	{
		subtextureINDEX = (GLuint*)malloc(sizeof(GLuint)* materials.size());
		printf("limit size : %d\n", materials.size());
		int count = 0;
		// For Each Material
		for(int i = 0; i < materials.size(); i++)
		{
			// materials[i].diffuse_texname; // This is the Texture Path You Need
			ILuint ilTexName;
			ilGenImages(1, &ilTexName);
			ilBindImage(ilTexName);
			printf("i : %d\n", i);
			cout << materials[i].diffuse_texname.c_str() << endl;
			MaterialParameters submat;
			for (int z = 0; z < 3; z++){
				submat.ambient[z] = (materials[i].ambient[z] == 0) ? 1 : materials[i].ambient[z];
				submat.diffuse[z] = (materials[i].diffuse[z] == 0) ? 1 : materials[i].diffuse[z];
				submat.specular[z] = materials[i].specular[z];
			}
			submat.ambient[3] = 1; submat.diffuse[3] = 1; submat.specular[3] = 1;
			submat.shininess = (materials[i].shininess == 0) ? 1 : materials[i].shininess;
			thieObjMat.push_back(submat);
			if(ilLoadImage(materials[i].diffuse_texname.c_str()))
			{
				cout << materials[i].diffuse_texname.c_str() << endl;
				unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 4];
				// Image Width = ilGetInteger(IL_IMAGE_WIDTH)
				// Image Height = ilGetInteger(IL_IMAGE_HEIGHT)
				ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGBA, IL_UNSIGNED_BYTE, data);

				glGenTextures(1, &subtextureINDEX[i]);
				glBindTexture(GL_TEXTURE_2D, subtextureINDEX[i]);
								
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					
				//GLfloat *data = new GLfloat[1024 * 1024 * 4];
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				glGenerateMipmap(GL_TEXTURE_2D);

				delete[] data;
				ilDeleteImages(1, &ilTexName);
				count++;
			}
		}

		size_t positionOffset = 0;
		size_t normalOffset = 0;
		size_t indexOffset = 0;
		int baseVertex = 0;
		shapeCount = shapes.size();

		printf("count : %d, size : %d\n", count, shapes.size());
		// For Each Shape (or Mesh, Object)
		for (int i = 0; i < shapes.size(); i++)
		{
			
			OBJ_SHADER obj;

			obj.indices = shapes[i].mesh.indices;
			obj.materialIterator.push_back(0);
			obj.materialID.push_back(shapes[i].mesh.material_ids[0]);
			
			for (int k = 0; k < shapes[i].mesh.material_ids.size(); k++){
				if (obj.materialID[obj.materialID.size() -1 ] != shapes[i].mesh.material_ids[k]){
					int ite = k * 3;
					obj.materialIterator.push_back(ite);
					obj.materialID.push_back(shapes[i].mesh.material_ids[k]);
					
				}
			}
			
			obj.materialIterator.push_back(shapes[i].mesh.indices.size());
			
			
			glGenBuffers(1, &obj.buffer[0]);

			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[0]);
			glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size()*sizeof(float), &shapes[i].mesh.positions[0], GL_STATIC_DRAW);

			glGenBuffers(1, &obj.buffer[1]);
			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[1]);
			glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size()*sizeof(float), &shapes[i].mesh.texcoords[0], GL_STATIC_DRAW);

			glGenBuffers(1, &obj.buffer[2]);
			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[2]);
			glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size()*sizeof(float), &shapes[i].mesh.normals[0], GL_STATIC_DRAW);
			
			glGenBuffers(1, &obj.ele_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ele_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size()*sizeof(unsigned int), &shapes[i].mesh.indices[0], GL_STATIC_DRAW);

			glGenVertexArrays(1, &obj.vao);
			glBindVertexArray(obj.vao);
			
			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[0]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[1]);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, obj.buffer[2]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);
			
			obj.indexSize = shapes[i].mesh.indices.size();
			objects.push_back(obj);

			firsts.push_back(indexOffset);
			positionOffset += shapes[i].mesh.positions.size();
			normalOffset += shapes[i].mesh.normals.size();
			indexOffset += shapes[i].mesh.indices.size();
			baseVertices.push_back(baseVertex);
			indexCounts.push_back(shapes[i].mesh.indices.size());
			baseVertex += shapes[i].mesh.positions.size() / 3;
		}

		glGenBuffers(1, &indirect_draw_buffer);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
		glBufferData(GL_DRAW_INDIRECT_BUFFER,
			NUM_DRAWS * sizeof(DrawElementsIndirectCommand),
			NULL,
			GL_STATIC_DRAW);

		DrawElementsIndirectCommand * cmd = (DrawElementsIndirectCommand *)
			glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, NUM_DRAWS * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		for (int i = 0; i < NUM_DRAWS; i++)
		{
			cmd[i].first = firsts[i % shapeCount];
			cmd[i].count = shapes[i % shapeCount].mesh.indices.size();
			cmd[i].primCount = 1;
			cmd[i].baseVertex = baseVertices[i % shapeCount];
			cmd[i].baseInstance = i;
		}

		glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
	}
	models.push_back(objects);
	texIndex.push_back(subtextureINDEX);
	LightingMaterials.push_back(thieObjMat);
}
float g_fps(void(*func)(void), int n_frame)
{
	clock_t start, finish;
	int i;
	float fps;

	printf("Performing benchmark, please wait");
	start = clock();
	for (i = 0; i<n_frame; i++)
	{
		if ((i + 1) % 10 == 0)
			printf(".");
		func();
	}
	printf("done\n");
	finish = clock();

	fps = float(n_frame) / (finish - start)*CLOCKS_PER_SEC;
	return fps;
}
// GLUT callback. Called to draw the scene.
void My_Display()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// program_RenderScene :: Depth texture (for shader)
	vector<OBJ_SHADER> objects = models[0];
	GLuint* subtextureINDEX;
	vector<MaterialParameters> thisObjMat;
	mat4 newMVP;
	
	vec3 light_position = pointlight->GetLightPosition();
	light_position = vec3(light_position.x - 1000.0f, light_position.y, light_position.z);
	mat4 light_proj_matrix = frustum(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10000.0f);
	mat4 light_view_matrix = lookAt(light_position, vec3(1000.0f, light_position.y, light_position.z), vec3(0.0f, 1.0f, 0.0f));

	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	mat4 scale_bias_matrix = mat4(
		vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f));

	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_proj_matrix * light_view_matrix;

	glEnable(GL_DEPTH_TEST);
	glUseProgram(program_ShadowDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	for (int j = 0; j < objects.size(); j++){
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
		glBindVertexArray(objects[j].vao);
		glUniformMatrix4fv(uniforms_light_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[j].ele_buffer);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, objects[j].indexSize, GL_UNSIGNED_INT, 0);
		//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, NUM_DRAWS, 0);
	}

	objects = models[selectModel];
	for (int j = 0; j < objects.size(); j++){
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
		glBindVertexArray(objects[j].vao);

		mat4 M = transpose(Translate(0, 118, 0)) * Scale(10, 10, 10) * Rotate(1.5f, 0.0f, 0.0f);
		newMVP = light_vp_matrix * M;
		glUniformMatrix4fv(uniforms_light_mvp, 1, GL_FALSE, value_ptr(newMVP));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[j].ele_buffer);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElements(GL_TRIANGLES, objects[j].indexSize, GL_UNSIGNED_INT, 0);
		//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, NUM_DRAWS, 0);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.6f, 0.6f, 1.0f, 1.0f);
	glViewport(0, 0, image_size[0], image_size[1]);
	

	// Clear part
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static const GLfloat green[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const GLfloat one = 1.0f;

	glClearBufferfv(GL_COLOR, 0, green);
	glClearBufferfv(GL_DEPTH, 0, &one);

	// program_RenderScene :: Display skybox
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
	glUseProgram(skybox_program);
	glDisable(GL_DEPTH_TEST);

	float camera[3] = { cameraEyes.x, cameraEyes.y, cameraEyes.z };
	glUniform3fv(glGetUniformLocation(skybox_program, "eye_position"), 1, camera);
	glUniformMatrix4fv(glGetUniformLocation(skybox_program, "view_matrix"), 1, GL_FALSE, value_ptr(CameraProjectionMatrix * CameraViewMatrix));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	// program_RenderScene :: Render Scene
	glUseProgram(program_RenderScene);
	
	pointlight->LightingUniformGiven();
	directionlight->LightingUniformGiven();
	spotlight->LightingUniformGiven();
	spotlight->SpotLightingUniformGiven();

	glUniform3fv(iCameraPosition, 1, camera);
	glUniformMatrix4fv(iLocView, 1, GL_FALSE, value_ptr(CameraViewMatrix));	
	vector<tinyobj::shape_t> new_shapes;
	
	GetFbxAnimation(myFbx[selectModel-1], new_shapes, timer_cnt/100); // The Last Parameter is A Float in [0, 1], Specifying The Animation Location You Want to Retrieve
	for(int i = 0; i < new_shapes.size(); i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, models[selectModel][0].buffer[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, new_shapes[i].mesh.positions.size() * sizeof(float), &new_shapes[i].mesh.positions[0]);
	}

	glUniform1i(shadow_tex, 0);
	glUniform1i(object_tex, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glActiveTexture(GL_TEXTURE1);

	mat4 shadow_matrix_obj = shadow_sbpv_matrix ;

	objects = models[0];
	subtextureINDEX = texIndex[0];
	thisObjMat = LightingMaterials[0];
	for (int j = 0; j < objects.size(); j++){
		for (int k = objects[j].materialID.size()-1; k >= 0; k--){
			int matID = objects[j].materialID[k];
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
			glBindVertexArray(objects[j].vao);

			glBindTexture(GL_TEXTURE_2D, subtextureINDEX[matID]);
			mat4 M = Scale(5, 5, 5)* Rotate(1.5f, 0.0f, 0.0f);
			newMVP = mvp * M;
			glUniformMatrix4fv(iLocModel, 1, GL_FALSE, value_ptr(mat4(1)));
			glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
			glUniform1i(changeMode, changing);
			glUniformMatrix4fv(shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix_obj));

			glUniform4fv(iLocMAmbient, 1, thisObjMat[matID].ambient);
			glUniform4fv(iLocMDiffuse, 1, thisObjMat[matID].diffuse);
			glUniform4fv(iLocMSpecular, 1, thisObjMat[matID].specular);
			glUniform1f(iLocMShininess, thisObjMat[matID].shininess);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[j].ele_buffer);

			glDrawElements(GL_TRIANGLES, (objects[j].materialIterator[k + 1]), GL_UNSIGNED_INT, (void*)(objects[j].materialIterator[k] * sizeof(float)));
			//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, NUM_DRAWS, 0);
		}
	}

	objects = models[selectModel];
	subtextureINDEX = texIndex[selectModel];
	for (int j = 0; j < objects.size(); j++){
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
		glBindVertexArray(objects[j].vao);

		glBindTexture(GL_TEXTURE_2D, subtextureINDEX[objects[j].materialID[0]]);

		mat4 M = transpose(Translate(0, 118, 0)) * Scale(10, 10, 10) * Rotate(1.5f, 0.0f, 0.0f);
		newMVP = mvp * M;
		shadow_matrix_obj = shadow_sbpv_matrix * M;
		glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(newMVP));
		glUniformMatrix4fv(shadow_matrix, 1, GL_FALSE, value_ptr(shadow_matrix_obj));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[j].ele_buffer);

		glDrawElements(GL_TRIANGLES, objects[j].indexSize, GL_UNSIGNED_INT, 0);
		//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, NUM_DRAWS, 0);
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	
	glUseProgram(program_FrameBuffer);
	glBindVertexArray(window_vao);

	glUniform1i(texLoc0, 0);
	glUniform1i(texLoc1, 1);
	glActiveTexture(GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, FBODataTexture);
	glActiveTexture(GL_TEXTURE1);
	if (showSelect == 11 || showSelect == 16)
		glBindTexture(GL_TEXTURE_2D, NoiseTexture0);
	else
		glBindTexture(GL_TEXTURE_2D, NoiseTexture1);

	glUniform1i(Shader_now_Loc, showSelect);
	glUniform1i(Shader_magflag, magflag);
	glUniform1i(Shader_comflag, comflag);
	glUniform1f(Shader_Compare, compareBarX); 
	glUniform1f(Shader_time, timer_cnt);

	glUniform2fv(Shader_Image_size, 1, image_size);
	glUniform2fv(Shader_mouseposition, 1, mouse_position);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_TEXTURE_2D);
	if (finish_without_update)
		glFinish();
	else
		glutSwapBuffers();
}

void UpdateCamera(){
	/*cameraCenter = vec3(Rotate(rotate_vector.x, rotate_vector.y, rotate_vector.z) * vec4(1, 0, 0, 1));
	cameravector = vec3(1 + cameraEyes.x, 1 + cameraEyes.y, cameraEyes.z);
	CameraViewMatrix = lookAt(cameravector, (cameravector + cameraCenter), vec3(0.0f, 1.0f, 0.0f));
	mvp = CameraProjectionMatrix * CameraViewMatrix ;
	printf("%lf %lf %lf \n", cameravector.x, cameravector.y, cameravector.z);
	//printf("%lf %lf %lf \n", cameraCenter.x, cameraCenter.y, cameraCenter.z);*/

	vec3 cameraVector = cameraCenter - cameraEyes;
	//cameraEyes = vec3(Rotate(rotate_vector.x, rotate_vector.y, rotate_vector.z) * vec4(cameraEyes, 1) );
	cameraCenter = cameraEyes + vec3(Rotate(rotate_vector.x, rotate_vector.y, rotate_vector.z) * vec4(cameraVector, 0));
	//upVector = vec3(Rotate(rotate_vector.x, rotate_vector.y, rotate_vector.z) * vec4(upVector, 1) );
	CameraViewMatrix = lookAt(cameraEyes, cameraEyes + cameraVector, upVector);
	mvp = CameraProjectionMatrix * CameraViewMatrix;
	rotate_vector = vec3(0);
	printf("%lf %lf %lf \n", cameraEyes.x, cameraEyes.y, cameraEyes.z);
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;
	image_size[0] = width;
	image_size[1] = height;
	printf("%d %d", width, height);
	CameraProjectionMatrix = perspective(radians(90.0f), viewportAspect, 0.1f, 100000.0f);
	
	//mvp = ortho(-15 * viewportAspect, 15 * viewportAspect, -10.0f, 20.0f, 0.0f, 100.0f);
	UpdateCamera();

	glDeleteRenderbuffers(1, &depthRBO);
	glDeleteTextures(1, &FBODataTexture);
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

	glGenTextures(1, &FBODataTexture);
	glBindTexture(GL_TEXTURE_2D, FBODataTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBODataTexture, 0);
}

void My_Timer(int val)
{
	timer_cnt++;
	if (timer_cnt > 120) timer_cnt = 0;
	glutPostRedisplay();
	if(timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void onMouse(int who, int state, int x, int y)
{


	switch (who)
	{
	case GLUT_LEFT_BUTTON:
		if ((compareBarX*image_size[0] - 10) <= x && (compareBarX*image_size[0] + 10) >= x){
			mouseCompare.flag = 1;
			mouseLeft.flag = 0;
		}
		else{
			mouseLeft.flag = 1;
			mouseCompare.flag = 0;
		}
		break;
	case GLUT_MIDDLE_BUTTON:  break;
	case GLUT_RIGHT_BUTTON:
		break;
	default:                 
		break;
	}

	switch (state)
	{
	case GLUT_DOWN:

		break;
	case GLUT_UP:

		mouseLeft.start[0] = 0;
		mouseLeft.start[1] = 0;
		mouseCompare.start[0] = 0;
		mouseCompare.start[1] = 0;

		break;
	}


}

void onMouseMotion(int x, int y)
{
	if (magflag == 1){
		mouse_position[0] = (double)x / image_size[0];
		mouse_position[1] = 1 - (double)y / image_size[1];
	}
	else if (mouseLeft.flag == 1){
		mouseLeft.end[0] = (double)x / 500;
		mouseLeft.end[1] = (double)y / 500;
		float movex = -(mouseLeft.end[0] - mouseLeft.start[0]);
		float movey = -(mouseLeft.end[1] - mouseLeft.start[1]);
		if (mouseLeft.start[0] != 0 || mouseLeft.start[1] != 0){
			rotate_vector.z += movey;
			rotate_vector.y += movex;
			UpdateCamera();
		}
		mouseLeft.start[0] = mouseLeft.end[0];
		mouseLeft.start[1] = mouseLeft.end[1];
	}
	else{
		mouseCompare.end[0] = (double)x / image_size[0];
		float movex = (mouseCompare.end[0] - mouseCompare.start[0]);
		if (mouseCompare.start[0] != 0 || mouseCompare.start[1] != 0){
			compareBarX += movex;
		}
		mouseCompare.start[0] = mouseCompare.end[0];
	
	
	}
	
}

void My_Keyboard(unsigned char key, int x, int y)
{
	vec3 move;
	vec3 movement = cameraCenter - cameraEyes;
	switch (key) {
	case 'w':
		move = normalize(movement);
		cameraEyes.x += move.x * 10;
		cameraEyes.y += move.y * 10;
		cameraEyes.z += move.z * 10;
		cameraCenter.x += move.x * 10;
		cameraCenter.y += move.y * 10;
		cameraCenter.z += move.z * 10;
		UpdateCamera();
		break;
	case 's':
		move = normalize(movement);
		cameraEyes.x -= move.x * 10;
		cameraEyes.y -= move.y * 10;
		cameraEyes.z -= move.z * 10;

		cameraCenter.x -= move.x * 10;
		cameraCenter.y -= move.y * 10;
		cameraCenter.z -= move.z * 10;
		UpdateCamera();
		break;
	case 'd':
		move = normalize(movement);
		move = normalize(cross(move, vec3(move.x, 1, move.z)));
		cameraEyes.x += move.x * 10;
		cameraEyes.y += move.y * 10;
		cameraEyes.z += move.z * 10;

		cameraCenter.x += move.x * 10;
		cameraCenter.y += move.y * 10;
		cameraCenter.z += move.z * 10;
		UpdateCamera();
		break;
	case 'a':
		move = normalize(movement);
		move = normalize(cross(move, vec3(move.x, 1, move.z)));
		cameraEyes.x -= move.x * 10;
		cameraEyes.y -= move.y * 10;
		cameraEyes.z -= move.z * 10;

		cameraCenter.x -= move.x * 10;
		cameraCenter.y -= move.y * 10;
		cameraCenter.z -= move.z * 10;
		UpdateCamera();
		break;
	case 'x':
		move = normalize(cross(movement, vec3(movement.x, movement.y, 1)));
		cameraEyes.x -= move.x * 10;
		cameraEyes.y -= move.y * 10;
		cameraEyes.z -= move.z * 10;

		cameraCenter.x -= move.x * 10;
		cameraCenter.y -= move.y * 10;
		cameraCenter.z -= move.z * 10;
		UpdateCamera();
		break;
	case 'z':
		move = normalize(cross(movement, vec3(movement.x, movement.y, 1)));
		cameraEyes.x += move.x * 10;
		cameraEyes.y += move.y * 10;
		cameraEyes.z += move.z * 10;

		cameraCenter.x += move.x * 10;
		cameraCenter.y += move.y * 10;
		cameraCenter.z += move.z * 10;
		UpdateCamera();
		break;
	case 'f':
		finish_without_update = true;
		printf("%f fps\n", g_fps(My_Display, 100));
		finish_without_update = false;
		break;
	case 'n':
		(changing == 0) ? changing = 1 : changing = 0;
		break;
	case 'c':
		system("CLS");
		break;
	case 'h':
	case 'H':
		printf("\n\n====================================================================\n");
		printf("-----------There is help information. !!!-----------\n");
		printf("Press f to calculate FPS.\n");
		printf("Press n to change texture mode or normal mode.\n");
		printf("Press -> / <- to change the selecting animation.\n\n");
		printf("-----------Then you can move.-------------\n");
		printf("Press w to move forward, s to move backward\n");
		printf("Press d to move left side, a to move right side\n");
		printf("Press x to move up, z to move down\n\n");
		printf("-----------Mouse Event Function.-----------\n");
		printf("Press the left button and drag to rotate camera view.\n\n");
		printf("-----------Another help Functions.-----------\n");
		printf("Press h to get help information.\n");
		printf("Press c/C to clear the terminal.\n");
		printf("\n====================================================================\n\n");
		break;
	case '1':
		pointlight->ChangeOpenStatus();
		break;
	case '2':
		directionlight->ChangeOpenStatus();
		break;
	case '3':
		spotlight->ChangeOpenStatus();
		break;
	default:
		break;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		break;
	case GLUT_KEY_PAGE_UP:
		break;
	case GLUT_KEY_LEFT:
		selectModel--;
		if (selectModel < 1) selectModel = 3;
		break;
	case GLUT_KEY_RIGHT:
		selectModel++;
		if (selectModel > 3) selectModel = 1;
		UpdateCamera();
		break;
	default:
		break;
	}
}

void My_Menu(int id)
{
	
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	case Shader_Blur:
		showSelect = 0;
		break;
	case Shader_Quantization:
		showSelect = 1;
		break;
	case Shader_DoG:
		showSelect = 2;
		break;
	case Shader_CombineBasic:
		showSelect = 3;
		break;
	case Shader_RedBlue:
		showSelect = 4;
		break;
	case Shader_Sharpness:
		showSelect = 5;
		break;
	case Shader_Laplacian:
		showSelect = 6;
		break;
	case Shader_Bloom:
		showSelect = 7;
		break;
	case Shader_Magnifier:
		(magflag == 0) ? magflag = 1 : magflag = 0;
		break;
	case Shader_Pixel:
		showSelect = 8;
		break;
	case Shader_Constant:
		showSelect = 9;
		break;
	case Shader_Embossion:
		showSelect = 10;
		break;
	case Shader_WaterColor:
		showSelect = 11;
		break;
	case Shader_Swing:
		showSelect = 12;
		break;
	case Shader_Ripple:
		showSelect = 13;
		break;
	case Shader_FrostedGlass:
		showSelect = 14;
		break;
	case Shader_CompareONOFF:
		(comflag == 0) ? comflag = 1 : comflag = 0;
		break;
	case Shader_Oilpainting:
		showSelect = 15;
		break;
	case Shader_Ink:
		showSelect = 16;
		break;
	default:
		break;
	}
}
void initTextures(char* name, int flag)
{
	// load jpg
	texture_data tdata = load_jpg(name); // return width * height * 3 uchars
	if (tdata.data == 0)
	{
		// load failed
		return;
	}
	if (flag == 1){
		glGenTextures(1, &NoiseTexture0);
		glBindTexture(GL_TEXTURE_2D, NoiseTexture0);
	}
	else{
		glGenTextures(1, &NoiseTexture1);
		glBindTexture(GL_TEXTURE_2D, NoiseTexture1);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tdata.width, tdata.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata.data); // Use GL_RGB
	glGenerateMipmap(GL_TEXTURE_2D);
	free_texture_data(tdata);
	printf("%s %d %d",name, tdata.width, tdata.height);
	// load png
	/*tdata = load_png("nthu.png"); // return width * height * 4 uchars
	if (tdata.data == 0)
	{
		// load failed
		return;
	}
	glGenTextures(1, &texture_png);
	glBindTexture(GL_TEXTURE_2D, texture_png);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data); // Use GL_RGBA
	glGenerateMipmap(GL_TEXTURE_2D);
	free_texture_data(tdata);*/
}
void setLightingSource(){

	vec4 ambient = { 0.1f, 0.1f, 0.1f, 1 };
	vec4 diffuse = { 0.3f, 0.3f, 0.3f, 1 };
	vec4 specular = { 1, 1, 1, 1 };
	vec4 position = { -300.0f, 202.0f, -44.0f, 1 };
	pointlight = new Lighting(0, 1);
	pointlight->SetLight(value_ptr(ambient), value_ptr(diffuse), value_ptr(specular), value_ptr(position));

	ambient = { 0.4f, 0.4f, 0.4f, 1 };
	diffuse = { 0.5f, 0.5f, 0.5f, 1 };
	specular = { 1, 1, 1, 1 };
	position = { -300.0f, 404.0f, -44.0f, 0 };
	directionlight = new Lighting(1, 1);
	directionlight->SetLight(value_ptr(ambient), value_ptr(diffuse), value_ptr(specular), value_ptr(position));

	ambient = { 1.0f, 1.0f, 1.0f, 1 };
	diffuse = { 1.0f, 1.0f, 1.0f, 1 };
	specular = { 1, 1, 1, 1 };
	position = { -410.2f, 206.2f, 4.2f, 1 };
	spotlight = new Lighting(2, 0);
	spotlight->SetLight(value_ptr(ambient), value_ptr(diffuse), value_ptr(specular), value_ptr(position));
	spotlight->SetSpotLight(vec3(410.2f, -206.2f, -4.2f), 900.0f, 12.5f);

}
void initParas(){
	cameraEyes = { -410.20f, 206.2f, 4.2f };
	cameraCenter = { 93.59f, 206.2f, -0.2f };
	rotate_vector = { 0, 0, 0 };
	upVector = { 0, 1, 0 };
	initTextures("ImageProcessTexture/noise2.jpg", 1);
	initTextures("ImageProcessTexture/diffuse.jpg", 2);
	setLightingSource();
}
int main(int argc, char *argv[])
{
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 720);
	glutCreateWindow("Assignment 03 102062222"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
	glewInit();
	ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	dumpInfo();
	initParas();
	My_Init();
	My_LoadModels("sponza.obj", 1);
	//My_LoadModels("Volleyball.FBX",2);
	//My_LoadModels("Island/island.fbx", 2 );
	My_LoadModels("zombie_fury.FBX", 2 );
	My_LoadModels("zombie_walk.FBX", 2 );
	My_LoadModels("zombie_dead.FBX", 2);
	
	////////////////////

	// Create a menu and bind it to mouse right button.
	////////////////////////////
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int shader = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddSubMenu("Shader", shader);
	glutAddMenuEntry("Compare Bar on/off", Shader_CompareONOFF);
	glutAddMenuEntry("Magnifier Glass on/off", Shader_Magnifier);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(shader);
	glutAddMenuEntry("Blur", Shader_Blur);
	glutAddMenuEntry("Quantization", Shader_Quantization);
	glutAddMenuEntry("DoG", Shader_DoG);
	glutAddMenuEntry("Image Abstraction", Shader_CombineBasic);
	glutAddMenuEntry("-----Mid-level-----", 200);
	glutAddMenuEntry("Red-Blue Stereo", Shader_RedBlue);
	glutAddMenuEntry("Sharpness Filter", Shader_Sharpness);
	glutAddMenuEntry("Laplacian Filter", Shader_Laplacian);
	glutAddMenuEntry("Pixelation Effect", Shader_Pixel);
	glutAddMenuEntry("-----Advanced-----", 200);
	glutAddMenuEntry("Bloom Effect", Shader_Bloom);  
	glutAddMenuEntry("WaterColor Effect", Shader_WaterColor); 
	glutAddMenuEntry("-----Cool shader-----", 200);
	glutAddMenuEntry("Constant Effect", Shader_Constant);
	glutAddMenuEntry("Embossion Effect", Shader_Embossion);
	glutAddMenuEntry("Swing Effect", Shader_Swing);
	glutAddMenuEntry("Ripple Effect", Shader_Ripple);
	glutAddMenuEntry("Frosted Glass Effect", Shader_FrostedGlass); 
	glutAddMenuEntry("Oil Painting Effect", Shader_Oilpainting); 
	glutAddMenuEntry("Water Ink Effect", Shader_Ink);
	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	////////////////////////////

	// Register GLUT callback functions.
	///////////////////////////////
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMouseMotion);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 
	///////////////////////////////

	// Enter main event loop.
	//////////////
	glutMainLoop();
	//////////////
	return 0;
}