#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    shader.frag
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Fragment shader for per-pixel lighting calculations, texturing, and distance-based fog evaluation.
 */

struct Material {           
  vec3  ambient;            
  vec3  diffuse;            
  vec3  specular;           
  float shininess;          
  bool  useTexture;         
};


struct Light {         
  vec3  ambient;       
  vec3  diffuse;
  vec3  specular;      
  vec3  position;     
  vec3  spotDirection; 
  float spotCosCutOff;
  float spotExponent; 
};

out vec4 fragmentColor;

in vec2 v_texCoord;
uniform sampler2D texSampler;

flat in vec4 color_v; 

in vec3 v_eyePosition;
in vec3 v_eyeNormal;

out vec4 color_f;

uniform bool enableFog;
uniform vec3 fogColor; 
uniform float fogDensity;
uniform sampler2D fogSampler;

uniform Material material; 
uniform Light lamp;  

uniform Light flashlight;
uniform bool flashlightEnabled;

vec4 pointLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {
  vec3 ret = vec3(0.0);
  
  vec3 lightDir = light.position - vertexPosition;
  float distance = length(lightDir);
  vec3 L = normalize(lightDir);
  
  float attenuation = 1.0 / (1.0 + 0.22 * distance + 0.20 * distance * distance);

  vec3 V = normalize(-vertexPosition);
  float NdotL = dot(vertexNormal, L);

  ret += material.ambient * light.ambient;

  if(NdotL > 0.0) {
    vec3 R = reflect(-L, vertexNormal);
    float RdotV = max(0.0, dot(R, V));

    ret += material.diffuse * light.diffuse * NdotL;
    ret += material.specular * light.specular * pow(RdotV, material.shininess);
  }

  return vec4(ret * attenuation, 1.0);
}

vec4 spotLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {
  vec3 ret = vec3(0.0);

  ret += material.ambient * light.ambient;

  vec3 V = normalize(-vertexPosition);
  if(dot(V, vertexNormal) < 0.0) {
    return vec4(ret, 1.0);
  }

  vec3 L = normalize(light.position - vertexPosition);
  float NdotL = dot(vertexNormal, L);

  if(NdotL < 0.0) {
    return vec4(ret, 1.0);
  }

  float spotCoef = max(0.0, dot(-L, normalize(light.spotDirection)));
  if(spotCoef > light.spotCosCutOff) {
    vec3 R = reflect(-L, vertexNormal);
    float RdotV = max(0.0, dot(R, V));

    ret += material.diffuse * light.diffuse * NdotL;
    ret += material.specular * light.specular * pow(RdotV, material.shininess);
    
    ret *= pow(spotCoef, light.spotExponent);
  }

  return vec4(ret, 1.0);
}

void main() {

vec3 normal = normalize(v_eyeNormal);
  vec4 lampContribution = pointLight(lamp, material, v_eyePosition, normal);
  
  vec4 totalLight = color_v + lampContribution;

  if (flashlightEnabled) {
      totalLight += spotLight(flashlight, material, v_eyePosition, normal);
  }

  vec4 baseColor;
  if(material.useTexture) {
    baseColor = totalLight * texture(texSampler, v_texCoord);
  } else {
    baseColor = totalLight;
  }

  if (enableFog) {
    float dist = length(v_eyePosition);
    
    float f = exp(-fogDensity * dist);
    f = clamp(f, 0.0, 1.0); 

    vec2 fogTexCoord = gl_FragCoord.xy * 0.002; 
    vec3 fogTexColor = texture(fogSampler, fogTexCoord).rgb;

    vec3 finalFogColor = fogColor * fogTexColor;

    vec3 finalRgb = mix(finalFogColor, baseColor.rgb, f);
    color_f = vec4(finalRgb, baseColor.a); 
  } else {
    color_f = baseColor;
  }
}