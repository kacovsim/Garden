#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    UIshader.vert
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Vertex shader for rendering 2D UI elements, featuring optional texture coordinate animation.
 */

in vec2 position;
in vec2 texCoord;
out vec2 vTexCoord;
uniform mat4 PVM;

uniform float time;
uniform int isAnimated;

void main() {
    gl_Position = PVM * vec4(position, 0.0, 1.0);
if (isAnimated == 1) {
        vTexCoord = texCoord + vec2(sin(time * 2.0) * 0.05, 0.0);
    } else {
        vTexCoord = texCoord;
    }
}