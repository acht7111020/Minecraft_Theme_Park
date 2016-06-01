#version 410 core                                         


out VS_OUT                                                
{                                                         
    vec3    tc;                                           
} vs_out;                                                 

uniform mat4 view_matrix;                                 
uniform vec3 eye_position;                                            
void main(void)                                           
{                                                         
    vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),     
                               vec3( 1.0, -1.0, 1.0),     
                               vec3(-1.0,  1.0, 1.0),     
                               vec3( 1.0,  1.0, 1.0));    
//   vec3[36] vertices = vec3[36](
//        // Positions
//      vec3(1.0f,  1.0f, -1.0f),
//      vec3(-1.0f, -1.0f, -1.0f),
//      vec3(1.0f, -1.0f, -1.0f),
//      vec3(1.0f, -1.0f, -1.0f),
//      vec3(1.0f,  1.0f, -1.0f),
//      vec3(-1.0f,  1.0f, -1.0f),
//      
//      vec3(-1.0f, -1.0f,  1.0f),
//      vec3(-1.0f, -1.0f, -1.0f),
//      vec3(-1.0f,  1.0f, -1.0f),
//      vec3(-1.0f,  1.0f, -1.0f),
//      vec3(-1.0f,  1.0f,  1.0f),
//      vec3(-1.0f, -1.0f,  1.0f),
//      
//      vec3(1.0f, -1.0f, -1.0f),
//      vec3(1.0f, -1.0f,  1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(1.0f,  1.0f, -1.0f),
//      vec3(1.0f, -1.0f, -1.0f),
//      
//      vec3(-1.0f, -1.0f,  1.0f),
//      vec3(-1.0f,  1.0f,  1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(1.0f, -1.0f,  1.0f),
//      vec3(-1.0f, -1.0f,  1.0f),
//      
//      vec3(-1.0f,  1.0f, -1.0f),
//      vec3(1.0f,  1.0f, -1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(1.0f,  1.0f,  1.0f),
//      vec3(-1.0f,  1.0f,  1.0f),
//      vec3(-1.0f,  1.0f, -1.0f),
//      
//      vec3(-1.0f, -1.0f, -1.0f),
//      vec3(-1.0f, -1.0f,  1.0f),
//      vec3(1.0f, -1.0f, -1.0f),
//      vec3(1.0f, -1.0f, -1.0f),
//      vec3(-1.0f, -1.0f,  1.0f),
//      vec3(1.0f, -1.0f,  1.0f)
//    );
    vec4 tc = inverse(view_matrix) * vec4(vertices[gl_VertexID], 1.0);
    vs_out.tc = eye_position - tc.xyz / tc.w ;
    gl_Position = vec4(vertices[gl_VertexID], 1.0);
}                                                         
