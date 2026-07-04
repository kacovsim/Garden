#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    UIshader.frag
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Fragment shader for rendering basic 2D textured UI elements.
 */

in vec2 vTexCoord;
out vec4 color;
uniform sampler2D texSampler;

void main() {
    color = texture(texSampler, vTexCoord);
}