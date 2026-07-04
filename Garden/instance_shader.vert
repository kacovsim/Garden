#version 140

//----------------------------------------------------------------------------------------
/**
 * \file    instanced_shader.vert
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Vertex shader for rendering instanced and static geometry, implementing a dynamic day/night lighting cycle.
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

in mat4 instanceMatrix;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec2 v_texCoord;
flat out vec4 color_v;
uniform float texScale;

out vec3 v_eyePosition;
out vec3 v_eyeNormal;

uniform float time;        
uniform Material material; 

uniform vec3 shadowTint;
uniform float sunSpeed;

uniform mat4 Vmatrix;       
uniform mat4 Pmatrix;    

vec4 directionalLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

  vec3 ret = vec3(0.0);

  ret += material.ambient * light.ambient;

  vec3 V = normalize(-vertexPosition);


  vec3 L = normalize(light.position);
  float NdotL = dot(vertexNormal, L);

  if(NdotL > 0.0) {
    vec3 R = reflect(-L, vertexNormal);
    float RdotV = max(0.0, dot(R, V));

    ret += material.diffuse * light.diffuse * NdotL;
    ret += material.specular * light.specular * pow(RdotV, material.shininess);
  }

  return vec4(ret, 1.0);
}

vec4 pointLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {
  vec3 ret = vec3(0.0);
  
  vec3 lightDir = light.position - vertexPosition;
  float distance = length(lightDir);
  vec3 L = normalize(lightDir);
  
  float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.005 * distance * distance);

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

Light sun;
Light moon; 

void setupLights() {
  float sunAngle = time * sunSpeed; 
  vec4 sunWorldPos = vec4(cos(sunAngle), 0.0, sin(sunAngle), 0.0);
  
  float intensity = smoothstep(-0.4, 0.2, sunWorldPos.z); 

  vec3 dayLightColor = vec3(0.9, 0.9, 0.85);
  vec3 sunsetLightColor = vec3(1.0, 0.35, 0.05);

  float colorMixFactor = smoothstep(0.0, 0.4, sunWorldPos.z); 
  vec3 currentSunColor = mix(sunsetLightColor, dayLightColor, colorMixFactor);

  sun.ambient  = vec3(0.4);
  sun.diffuse  = currentSunColor * intensity;
  
  vec3 currentSpecularColor = mix(vec3(1.0, 0.6, 0.2), vec3(1.0, 1.0, 1.0), colorMixFactor);
  sun.specular = currentSpecularColor * intensity;
  
  sun.position = (Vmatrix * sunWorldPos).xyz;

  float moonAngle = sunAngle + 3.14159; 
  vec4 moonWorldPos = vec4(cos(moonAngle) * 15.0, 0.0, sin(moonAngle) * 15.0, 1.0);
  
  float moonIntensity = smoothstep(-0.4, 0.2, moonWorldPos.z);
  
  moon.ambient  = vec3(0.0); 
  moon.diffuse  = vec3(0.2, 0.3, 0.6) * moonIntensity;
  moon.specular = vec3(0.4, 0.5, 0.8) * moonIntensity;
  moon.position = (Vmatrix * moonWorldPos).xyz;
}

void main() {
setupLights();

    mat4 Mmatrix = instanceMatrix;

    mat3 normalMatrix = transpose(inverse(mat3(Mmatrix)));

    vec3 vertexPosition = (Vmatrix * Mmatrix * vec4(position, 1.0)).xyz;         
    vec3 vertexNormal   = normalize(mat3(Vmatrix) * normalMatrix * normal);

    v_eyePosition = vertexPosition;
    v_eyeNormal = vertexNormal;

    vec3 globalAmbientLight = shadowTint;
    vec4 outputColor = vec4(material.ambient * globalAmbientLight, 0.0);
    
    outputColor += directionalLight(sun, material, vertexPosition, vertexNormal);
    outputColor += pointLight(moon, material, vertexPosition, vertexNormal);
     
    color_v = outputColor;
    v_texCoord = texCoord;

    gl_Position = Pmatrix * Vmatrix * Mmatrix * vec4(position, 1.0f);
}