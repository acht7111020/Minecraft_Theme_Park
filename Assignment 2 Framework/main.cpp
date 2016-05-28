#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tiny_obj_loader.h>
#include "fbxloader.h"
#include "texture.h"

#include <unordered_map>

#define PI 3.1415
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3


GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
float frame = 0;

using namespace glm;


struct indexGroup{
    GLint matid;
    int numIndices;
    GLuint ibo;
};

struct vbo {
    GLuint pos;
    GLuint nor;
    GLuint coord;
};

struct Crew {
    std::vector<indexGroup> indexGroups;
    
};

struct Model {
    std::vector<GLuint> texObjs;
    std::vector<GLuint> vaos;
    std::vector<vbo> vbos;
    fbx_handles myFbx;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<Crew> crews;
};

struct
{
    struct
    {
        GLint mv_matrix;
        GLint proj_matrix;
    } render;
    struct
    {
        GLint view_matrix;
    } skybox;
} uniforms;

Model My_LoadModels(string name);

std::vector<Model> models;
float roll_z = 0; // z
float pitch_x = 0; // x
float yaw_y = 270.0f; // y
vec3 eyeVector = vec3(100.0f, 200.0f, 0.0f);
vec2 mousePosition;
bool isMousePressed;
std::vector<Crew> crews;
mat4 mvp;
mat4 viewMatrix;
GLint um4mvp;
std::vector<GLuint> texObjs;
std::vector<GLuint> vaos;
std::vector<vbo> vbos;
std::vector<GLuint> ivbos;
GLuint program;
GLuint sampler;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
fbx_handles myFbx;
int zact = 1;
int counts = 0;


// skybox
GLuint skybox_program;
GLuint skybox_texture;
GLuint skybox_vao;
vector<string> faces;
GLuint skyboxVAO, skyboxVBO;


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
    printf("shader: %d\n", shader);
    if(isCompiled == GL_FALSE)
    {
        printf("not compiled\n");
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        
        // The maxLength includes the NULL character
        GLchar* errorLog = new GLchar[maxLength];
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        
        printf("%s\n", errorLog);
        delete[] errorLog;
    }
}


void My_Init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    glDepthFunc(GL_LEQUAL);
    
    // skybox shader
    skybox_program = glCreateProgram();
    GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    
    char** vertexShaderSource2 = loadShaderSource("skybox.vs.glsl");
    char** fragmentShaderSource2 = loadShaderSource("skybox.fs.glsl");
    
    glShaderSource(vertexShader2, 1, vertexShaderSource2, NULL);
    glShaderSource(fragmentShader2, 1, fragmentShaderSource2, NULL);
    freeShaderSource(vertexShaderSource2);
    freeShaderSource(fragmentShaderSource2);
    glCompileShader(vertexShader2);
    glCompileShader(fragmentShader2);
    shaderLog(vertexShader2);
    shaderLog(fragmentShader2);
    glAttachShader(skybox_program, vertexShader2);
    glAttachShader(skybox_program, fragmentShader2);
    glLinkProgram(skybox_program);
    glUseProgram(skybox_program);
    uniforms.skybox.view_matrix = glGetUniformLocation(skybox_program, "view_matrix");
    
    faces.push_back("right.jpg");
    faces.push_back("left.jpg");
    faces.push_back("bottom.jpg");
    faces.push_back("top.jpg");
    faces.push_back("back.jpg");
    faces.push_back("front.jpg");
    
    glGenTextures(1, &skybox_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    for(int face = 0; face < 6; face++)
    {
        Texture tga(faces[face]);
        if(tga.Data() != 0)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, tga.Width(), tga.Height() , 0, GL_RGB, GL_UNSIGNED_BYTE, tga.Data());
            tga.Release();
        }

    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenVertexArrays(1, &skybox_vao);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    
    // render program
    program = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
    char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
    
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
    freeShaderSource(vertexShaderSource);
    freeShaderSource(fragmentShaderSource);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    shaderLog(vertexShader);
    shaderLog(fragmentShader);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    um4mvp = glGetUniformLocation(program, "um4mvp");
    glUseProgram(program);
    
    glUniform1i(glGetUniformLocation(sampler, "tex"), 0);
    
    glEnable(GL_TEXTURE_2D);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    models.resize(4);
    models[0] = My_LoadModels("island_no_ocean.obj");
    //models[1] = My_LoadModels("island_no_ocean.obj");
    counts = 1;
    //models[2] = My_LoadModels("zombie_fury.FBX");
    //models[3] = My_LoadModels("zombie_walk.FBX");
}

Model My_LoadModels(string name)
{
    Model model;
    std::vector<GLuint> texObjs;
    std::vector<GLuint> vaos;
    std::vector<vbo> vbos;
    fbx_handles myFbx;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<Crew> crews;
    bool isFbx = false;
    
    std::string err;
    string extension = name.substr( name.find_last_of('.') + 1 );
    if (extension == "FBX") {
        isFbx = true;
    }
    else {
        isFbx = false;
    }
    
    bool ret = (isFbx) ? LoadFbx(myFbx, shapes, materials, err, name.c_str()) :
    tinyobj::LoadObj(shapes, materials, err, name.c_str());

    if(ret)
    {
        // For Each Material
        glActiveTexture(GL_TEXTURE0);
        texObjs.resize(materials.size());
        
        for(int i = 0; i < materials.size(); i++)
        {
            
            // materials[i].diffuse_texname; // This is the Texture Path You Need
            printf("material: %s\n", materials[i].diffuse_texname.c_str());
            Texture texture(materials[i].diffuse_texname.c_str());
            
            if(texture.Data() != 0 && i != 61)
            {
                
                //Image Width = texture.Width();
                //Image Height = texture.Height();
                //GLuint texObj;
                glGenTextures(1, &texObjs[i]);
                
                if(texObjs[i] != 0)
                    glBindTexture(GL_TEXTURE_2D, texObjs[i]);
                // TODO: Generate an OpenGL Texture and use texture.Data() as Input Here.
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.Width(), texture.Height(), 0, texture.GLFormat(), GL_UNSIGNED_BYTE, texture.Data());
                //texObjs.push_back(texObj);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                
                glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                glGenerateMipmap(GL_TEXTURE_2D);
                texture.Release();
            }
        }
        
        // For Each Shape (or Mesh, Object)
        vbos.resize(shapes.size());
        crews.resize(shapes.size());
        for(int i = 0; i < shapes.size(); i++)
        {
            Crew &crew = crews[i];
            
            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            // bind buffer
            GLuint buffer[3];
            glGenBuffers(3, buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
            glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() *  sizeof(float), &shapes[i].mesh.positions[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            vbos[i].pos = buffer[0];
            
            glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
            glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() *  sizeof(float), &shapes[i].mesh.normals[0], GL_STATIC_DRAW);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            vbos[i].nor = buffer[1];
            
            glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
            glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size() *  sizeof(float), &shapes[i].mesh.texcoords[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
                                  0);//offset
            vbos[i].coord = buffer[2];
            
            GLuint indexVBO;
            glGenBuffers(1, &indexVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size() * sizeof(unsigned int), &shapes[i].mesh.indices[0], GL_STATIC_DRAW);
            
            vaos.push_back(vao);
            ivbos.push_back(indexVBO);
            
            std::unordered_map<GLint, std::vector<unsigned int>> indexGroupMap;
            for (int j = 0; j < shapes[i].mesh.indices.size() / 3; j++)
            {
                GLint matid = shapes[i].mesh.material_ids[j];
                std::vector<unsigned int> &indexGroup = indexGroupMap[matid];
                indexGroup.push_back(shapes[i].mesh.indices[j * 3]);
                indexGroup.push_back(shapes[i].mesh.indices[j * 3 + 1]);
                indexGroup.push_back(shapes[i].mesh.indices[j * 3 + 2]);
            }
            
            
            int igIndex = 0;
            crew.indexGroups.resize(indexGroupMap.size());
            for (std::unordered_map<GLint, std::vector<unsigned int>>::iterator it = indexGroupMap.begin();
                 it != indexGroupMap.end();
                 it++)
            {
                
                crew.indexGroups[igIndex].matid = it->first;
                crew.indexGroups[igIndex].numIndices = it->second.size();
                glGenBuffers(1, &crew.indexGroups[igIndex].ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crew.indexGroups[igIndex].ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, it->second.size() * sizeof(unsigned int), &it->second[0], GL_STATIC_DRAW);
                igIndex++;
            }
            glBindVertexArray(0);
            
        }
        model.texObjs.assign(texObjs.begin(), texObjs.end());
        model.vaos.assign(vaos.begin(), vaos.end());
        model.vbos.assign(vbos.begin(), vbos.end());
        model.myFbx = myFbx;
        model.shapes.assign(shapes.begin(), shapes.end());
        model.crews.assign(crews.begin(), crews.end());
    }
    else {
        printf("Load model failed\n");
    }
    return model;
}

void UpdateView()
{
    
    //roll can be removed from here. because is not actually used in FPS camera
    mat4 matRoll  = mat4(1.0f);//identity matrix;
    mat4 matPitch = mat4(1.0f);//identity matrix
    mat4 matYaw   = mat4(1.0f);//identity matrix
    
    
    //roll, pitch and yaw are used to store our angles in our class
    printf("x: %f, y: %f, z: %f\n", pitch_x, yaw_y, roll_z);
    roll_z = mod(roll_z, 360.0f);
    pitch_x = mod(pitch_x, 360.0f);
    yaw_y = mod(yaw_y, 360.0f);
    mat4 mat = viewMatrix;
    //row major
    vec3 forward(mat[0][2], mat[1][2], mat[2][2]);
    vec3 side( mat[0][0], mat[1][0], mat[2][0]);
    vec3 up(mat[0][1], mat[1][1], mat[2][1]);
    
    matRoll  = rotate(matRoll,  radians(roll_z),  vec3(0.0f, 0.0f, 1.0f));
    matPitch = rotate(matPitch, radians(pitch_x), vec3(1.0f, 0.0f, 0.0f));
    matYaw   = rotate(matYaw,  radians(yaw_y),    vec3(0.0f, 1.0f, 0.0f));
    
    //order matters
    mat4 rotate = matRoll * matPitch * matYaw;
    
    mat4 t = mat4(1.0f);
    t = translate(t, -eyeVector);
    
    viewMatrix = rotate * t;
    
}

void Display_model(int id, vec3 pos)
{
    for(int i = 0; i < models[id].shapes.size(); i++){
        mat4 mvp = perspective( radians(65.0f),  1.0f, 3.0f, 3000.0f);
        
        mvp = mvp * viewMatrix * translate(mat4(), pos);
        //mvp = mvp * viewMatriid * translate(mat4(), vec3(0, 60, 0))* rotate(mat4(), radians(180.0f), vec3(0,0,1)) * rotate(mat4(), radians(90.0f), vec3(1,0,0));
        glUniformMatrix4fv(glGetUniformLocation(program, "um4mvp"), 1, GL_FALSE, value_ptr(mvp));
        
        glBindVertexArray(models[id].vaos[i]);
        
        for(int j = 0; j < models[id].crews[i].indexGroups.size(); j++){
            GLint matid = models[id].crews[i].indexGroups[j].matid;
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, models[id].crews[i].indexGroups[j].ibo);
            if (matid != -1)
                glBindTexture(GL_TEXTURE_2D, models[id].texObjs[matid]);
            glDrawElements(GL_TRIANGLES, models[id].crews[i].indexGroups[j].numIndices, GL_UNSIGNED_INT, 0);
        }
    }
    
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    
    glUseProgram(skybox_program);
    
    float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    
    glDisable(GL_DEPTH_TEST);
    mat4 mvp = perspective( radians(65.0f),  1.0f, 0.1f, 300.0f);
    mvp =  mvp * viewMatrix;
    
    glUniform3f(glGetUniformLocation(skybox_program, "eye_position"), eyeVector.x, eyeVector.y, eyeVector.z);
    glUniformMatrix4fv(uniforms.skybox.view_matrix, 1, GL_FALSE, value_ptr(mvp));
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_DEPTH_TEST);
    
    glUseProgram(program);
    Display_model(0, vec3(0));
    //Display_model(1, vec3(800, 0, 100));
    

    
    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    
    float viewportAspect = (float)width / (float)height;
    
}

void My_Timer(int val)
{
    timer_cnt++;
    frame = mod(frame, 1.0f);
    frame += 0.005;
    glutPostRedisplay();
    if(timer_enabled)
    {
        glutTimerFunc(timer_speed, My_Timer, val);
    }
}

void MouseMove(int x, int y)
{
    if (isMousePressed == false)
        return;
    //always compute delta
    //mousePosition is the last mouse position
    glm::vec2 mouse_delta = glm::vec2(x, y) - mousePosition;
    
    const float mouseX_Sensitivity = 0.7f;
    const float mouseY_Sensitivity = 0.7f;
    //note that yaw and pitch must be converted to radians.
    //this is done in UpdateView() by glm::rotate
    yaw_y   += mouseX_Sensitivity * mouse_delta.x;
    pitch_x += mouseY_Sensitivity * mouse_delta.y;
    
    
    mousePosition = glm::vec2(x, y);
    UpdateView();
}


void My_Mouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        isMousePressed = true;
        mousePosition.x = x;
        mousePosition.y = y;
        printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
    }
    else if(state == GLUT_UP)
    {
        isMousePressed = false;
        printf("Mouse %d is released at (%d, %d)\n", button, x, y);
    }
}

void My_Keyboard(unsigned char key, int x, int y)
{
    printf("Key %c is pressed at (%d, %d)\n", key, x, y);
    
    float dx = 0; //how much we strafe on x
    float dz = 0; //how much we walk on z
    float dy = 0;
    switch (key)
    {
        case 'w':
        {
            dz = 2;
            break;
        }
            
        case 's':
        {
            dz = -2;
            break;
        }
        case 'a':
        {
            dx = -2;
            break;
        }
            
        case 'd':
        {
            dx = 2;
            break;
        }
        case 'z':
        {
            dy = 2;
            break;
        }
        case 'x':
        {
            dy = -2;
            break;
        }
        case '1':
        {
            zact = 1;
            break;
        }
        case '2':
        {
            zact = 2;
            break;
        }
        case '3':
        {
            zact = 3;
            break;
        }
        default:
            break;
    }
    
    //get current view matrix
    mat4 mat = viewMatrix;
    //row major
    vec3 forward(mat[0][2], mat[1][2], mat[2][2]);
    vec3 side( mat[0][0], mat[1][0], mat[2][0]);
    vec3 up(mat[0][1], mat[1][1], mat[2][1]);
    
    const float speed = 2.0f;//how fast we move
    
    
    eyeVector += (-dz * forward + dx * side + dy * up) * speed;
    
    //update the view matrix
    UpdateView();
}

void My_SpecialKeys(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_F1:
            printf("F1 is pressed at (%d, %d)\n", x, y);
            break;
        case GLUT_KEY_PAGE_UP:
            printf("Page up is pressed at (%d, %d)\n", x, y);
            break;
        case GLUT_KEY_LEFT:
            printf("Left arrow is pressed at (%d, %d)\n", x, y);
            break;
        default:
            printf("Other special key is pressed at (%d, %d)\n", x, y);
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
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    // Initialize GLUT and GLEW, then create a window.
    ////////////////////
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Assignment 02 102062332"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
    dumpInfo();
    My_Init();
    viewMatrix =  lookAt(eyeVector, vec3(0.0f,0.0f,0.0f), vec3(0.0f,1.0f,0.0f));
    UpdateView();
    ////////////////////
    
    // Create a menu and bind it to mouse right button.
    ////////////////////////////
    int menu_main = glutCreateMenu(My_Menu);
    int menu_timer = glutCreateMenu(My_Menu);
    
    glutSetMenu(menu_main);
    glutAddSubMenu("Timer", menu_timer);
    glutAddMenuEntry("Exit", MENU_EXIT);
    
    glutSetMenu(menu_timer);
    glutAddMenuEntry("Start", MENU_TIMER_START);
    glutAddMenuEntry("Stop", MENU_TIMER_STOP);
    
    glutSetMenu(menu_main);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    ////////////////////////////
    
    // Register GLUT callback functions.
    ///////////////////////////////
    glutDisplayFunc(My_Display);
    glutReshapeFunc(My_Reshape);
    glutMouseFunc(My_Mouse);
    glutMotionFunc(MouseMove);
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