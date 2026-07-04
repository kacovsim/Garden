#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    skybox.frag
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Fragment shader for rendering a cubemap skybox with dynamic time-of-day tinting.
 */

in vec3 texCoord;
out vec4 fragColor;

uniform samplerCube skyboxSampler;
uniform vec3 shadowTint; 

void main() {
    vec4 texColor = texture(skyboxSampler, texCoord);
    
    float brightnessBoost = 1.54; 
    vec3 boostedTint = shadowTint * brightnessBoost;
    
    boostedTint = min(boostedTint, vec3(1.0));

    fragColor = vec4(texColor.rgb * boostedTint, texColor.a);
}