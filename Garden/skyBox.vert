#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    skybox.vert
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Vertex shader for generating a full-screen quad and computing 3D cubemap texture coordinates.
 */

in vec2 position;
out vec3 texCoord;

uniform mat4 inversePVmatrix;

void main() {
    gl_Position = vec4(position, 1.0, 1.0);
    vec4 farPlaneCoord = inversePVmatrix * vec4(position, 1.0, 1.0);
    
    vec3 worldDir = farPlaneCoord.xyz / farPlaneCoord.w;
   
    texCoord = vec3(worldDir.x, -worldDir.z, -worldDir.y);
}