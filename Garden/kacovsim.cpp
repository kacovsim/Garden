//----------------------------------------------------------------------------------------
/**
 * \file    kacovsim.cpp
 * \author  Jaroslav Sloup, Petr Felkel (Original Skeleton), Modified by Simona Kácová
 * \date    2026/05/15
 * \brief   Main application file for the PGR Garden scene. Handles rendering, input, and scene logic.
 */

 /**
  * \brief	\mainpage Documentation of the PGR Garden Application
  *
  * This application is built upon the skeleton for the BI-PGR course.
  * It features a 3D interactive garden environment utilizing OpenGL and the PGR framework.
  */

#include <iostream>
#include "pgr.h"
#include "object.h"
#include "allObjects.h"


#define SCENE_WIDTH  5.0f
#define SCENE_HEIGHT 5.0f
#define SCENE_DEPTH  5.0f

#define CAMERA_ELEVATION_MAX 65.0f

enum { KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_SHIFT, KEYS_COUNT };

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr char WINDOW_TITLE[] = "PGR: Garden";

bool keyMap[KEYS_COUNT];

ObjectList objects;

glm::mat4 currentViewMatrix;
glm::mat4 currentProjectionMatrix;

float targetElevationAngle = 0.0f;
float targetViewAngle = 90.0f;

Fairy* player = nullptr;
Bird* globalBird = nullptr;
Flashlight* playerFlashlight = nullptr;
SkyBox* skybox = nullptr;

FlowerInstanced* flowerField = nullptr;
RocksInstanced* rocksField = nullptr;
GrassInstanced* grassField = nullptr;
BellInstanced* bellField = nullptr;
DaisyInstanced* daisyField = nullptr;

int cameraMode = 0;

float player_speed = 0.005f;

float cameraElevationAngle = 0.0f;

int itemSelection = 0;

float sunSpeed = 0.01f;

int timeOfDayState = 0;

bool isFogEnabled = false;

bool isFlashlightEnabled = false;


ShaderProgram commonShaderProgram;
ShaderProgram instancedShaderProgram;
ShaderProgram skyboxShaderProgram;

GLuint iconTextures[6];
GLuint uiVao, uiVbo;
ShaderProgram uiShaderProgram;

GLuint fogTexture = 0;
GLint fogTextureLocation = -1;

GLuint explosionTextureID = 0;

/**
 * \brief Passes the transformation matrices to the shader.
 * \param shader Pointer to the active ShaderProgram.
 * \param modelMatrix The Model matrix of the object.
 * \param viewMatrix The global View matrix.
 * \param projectionMatrix The global Projection matrix.
 */
void setTransformUniforms(ShaderProgram* shader, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

	glm::mat4 PVM = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(shader->locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVM));
	glUniformMatrix4fv(shader->locations.Vmatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shader->locations.Mmatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	const glm::mat4 modelRotationMatrix = glm::mat4(
		modelMatrix[0],
		modelMatrix[1],
		modelMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	const glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));
	glUniformMatrix4fv(shader->locations.normalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

/**
 * \brief Passes material properties and texture data to the shader.
 * \param shader Pointer to the active ShaderProgram.
 * \param ambient Ambient color vector.
 * \param diffuse Diffuse color vector.
 * \param specular Specular color vector.
 * \param shininess Shininess exponent for specular highlights.
 * \param texture OpenGL texture ID.
 */
void setMaterialUniforms(ShaderProgram* shader, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture) {

	glUniform3fv(shader->locations.diffuseLocation, 1, glm::value_ptr(diffuse));
	glUniform3fv(shader->locations.ambientLocation, 1, glm::value_ptr(ambient));
	glUniform3fv(shader->locations.specularLocation, 1, glm::value_ptr(specular));
	glUniform1f(shader->locations.shininessLocation, shininess);

	if (texture != 0) {
		glUniform1i(shader->locations.useTextureLocation, 1);

		glUniform1i(shader->locations.texSampler, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	else {
		glUniform1i(shader->locations.useTextureLocation, 0);
	}
}

/**
 * \brief Loads, compiles, and links all required shader programs for the scene.
 *
 * \details This function initializes three distinct shader programs used in the application and
 * caches their attribute and uniform locations.
 * The compiled programs include:
 * - **Common Shader:** (`shader.vert` / `shader.frag`) Used for standard 3D object rendering.
 * Maps locations for PVM matrices, material properties, textures, fog, and multiple light sources (sun, static lamp, player flashlight).
 * - **Instanced Shader:** (`instance_shader.vert` / `shader.frag`) Used for drawing large quantities of objects (like grass or flowers) efficiently using hardware instancing.
 * Maps the `instanceMatrix` attribute alongside standard environment and lighting uniforms.
 * - **Skybox Shader:** (`skybox.vert` / `skybox.frag`) Used for rendering the background environment mapping.
 */
void loadShaderPrograms()
{
	GLuint shaders[] = {
	  pgr::createShaderFromFile(GL_VERTEX_SHADER, "shader.vert"),
	  pgr::createShaderFromFile(GL_FRAGMENT_SHADER,  "shader.frag"),
	  0
	};

	commonShaderProgram.program = pgr::createProgram(shaders);

	commonShaderProgram.locations.position = glGetAttribLocation(commonShaderProgram.program, "position");
	commonShaderProgram.locations.normal = glGetAttribLocation(commonShaderProgram.program, "normal");
	commonShaderProgram.locations.texCoord = glGetAttribLocation(commonShaderProgram.program, "texCoord");
	commonShaderProgram.locations.Vmatrix = glGetUniformLocation(commonShaderProgram.program, "Vmatrix");
	commonShaderProgram.locations.Mmatrix = glGetUniformLocation(commonShaderProgram.program, "Mmatrix");
	commonShaderProgram.locations.normalMatrix = glGetUniformLocation(commonShaderProgram.program, "normalMatrix");
	commonShaderProgram.locations.PVMmatrix = glGetUniformLocation(commonShaderProgram.program, "PVM");
	commonShaderProgram.locations.texSampler = glGetUniformLocation(commonShaderProgram.program, "texSampler");
	commonShaderProgram.locations.texScaleLocation = glGetUniformLocation(commonShaderProgram.program, "texScale");

	commonShaderProgram.locations.texOffsetLocation = glGetUniformLocation(commonShaderProgram.program, "texOffset");
	commonShaderProgram.locations.texSpriteSizeLocation = glGetUniformLocation(commonShaderProgram.program, "texSpriteSize");

	commonShaderProgram.locations.ambientLocation = glGetUniformLocation(commonShaderProgram.program, "material.ambient");
	commonShaderProgram.locations.diffuseLocation = glGetUniformLocation(commonShaderProgram.program, "material.diffuse");
	commonShaderProgram.locations.specularLocation = glGetUniformLocation(commonShaderProgram.program, "material.specular");
	commonShaderProgram.locations.shininessLocation = glGetUniformLocation(commonShaderProgram.program, "material.shininess");
	commonShaderProgram.locations.useTextureLocation = glGetUniformLocation(commonShaderProgram.program, "material.useTexture");
	commonShaderProgram.locations.timeLocation = glGetUniformLocation(commonShaderProgram.program, "time");

	commonShaderProgram.locations.shadowTintLocation = glGetUniformLocation(commonShaderProgram.program, "shadowTint");
	commonShaderProgram.locations.sunSpeedLocation = glGetUniformLocation(commonShaderProgram.program, "sunSpeed");

	commonShaderProgram.locations.lampAmbientLocation = glGetUniformLocation(commonShaderProgram.program, "lamp.ambient");
	commonShaderProgram.locations.lampDiffuseLocation = glGetUniformLocation(commonShaderProgram.program, "lamp.diffuse");
	commonShaderProgram.locations.lampSpecularLocation = glGetUniformLocation(commonShaderProgram.program, "lamp.specular");
	commonShaderProgram.locations.lampPositionLocation = glGetUniformLocation(commonShaderProgram.program, "lamp.position");

	commonShaderProgram.locations.enableFogLocation = glGetUniformLocation(commonShaderProgram.program, "enableFog");
	commonShaderProgram.locations.fogColorLocation = glGetUniformLocation(commonShaderProgram.program, "fogColor");
	commonShaderProgram.locations.fogDensityLocation = glGetUniformLocation(commonShaderProgram.program, "fogDensity");
	commonShaderProgram.locations.fogTextureLocation = glGetUniformLocation(commonShaderProgram.program, "fogSampler");


	commonShaderProgram.locations.flashlightEnabledLocation = glGetUniformLocation(commonShaderProgram.program, "flashlightEnabled");
	commonShaderProgram.locations.flashlightAmbientLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.ambient");
	commonShaderProgram.locations.flashlightDiffuseLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.diffuse");
	commonShaderProgram.locations.flashlightSpecularLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.specular");
	commonShaderProgram.locations.flashlightPositionLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.position");
	commonShaderProgram.locations.flashlightDirectionLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.spotDirection");
	commonShaderProgram.locations.flashlightCutOffLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.spotCosCutOff");
	commonShaderProgram.locations.flashlightExponentLocation = glGetUniformLocation(commonShaderProgram.program, "flashlight.spotExponent");



	commonShaderProgram.initialized = true;

	GLuint instancedShaders[] = {
		pgr::createShaderFromFile(GL_VERTEX_SHADER, "instance_shader.vert"),
		pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "shader.frag"),
		0
	};

	instancedShaderProgram.program = pgr::createProgram(instancedShaders);

	instancedShaderProgram.locations.position = glGetAttribLocation(instancedShaderProgram.program, "position");
	instancedShaderProgram.locations.normal = glGetAttribLocation(instancedShaderProgram.program, "normal");
	instancedShaderProgram.locations.texCoord = glGetAttribLocation(instancedShaderProgram.program, "texCoord");
	instancedShaderProgram.locations.texSampler = glGetUniformLocation(instancedShaderProgram.program, "texSampler");

	instancedShaderProgram.locations.instanceMatrix = glGetAttribLocation(instancedShaderProgram.program, "instanceMatrix");
	instancedShaderProgram.locations.Vmatrix = glGetUniformLocation(instancedShaderProgram.program, "Vmatrix");
	instancedShaderProgram.locations.Pmatrix = glGetUniformLocation(instancedShaderProgram.program, "Pmatrix");

	instancedShaderProgram.locations.diffuseLocation = glGetUniformLocation(instancedShaderProgram.program, "material.diffuse");
	instancedShaderProgram.locations.ambientLocation = glGetUniformLocation(instancedShaderProgram.program, "material.ambient");
	instancedShaderProgram.locations.specularLocation = glGetUniformLocation(instancedShaderProgram.program, "material.specular");
	instancedShaderProgram.locations.shininessLocation = glGetUniformLocation(instancedShaderProgram.program, "material.shininess");
	instancedShaderProgram.locations.useTextureLocation = glGetUniformLocation(instancedShaderProgram.program, "material.useTexture");
	instancedShaderProgram.locations.timeLocation = glGetUniformLocation(instancedShaderProgram.program, "time");

	instancedShaderProgram.locations.shadowTintLocation = glGetUniformLocation(instancedShaderProgram.program, "shadowTint");
	instancedShaderProgram.locations.sunSpeedLocation = glGetUniformLocation(instancedShaderProgram.program, "sunSpeed");

	instancedShaderProgram.locations.lampAmbientLocation = glGetUniformLocation(instancedShaderProgram.program, "lamp.ambient");
	instancedShaderProgram.locations.lampDiffuseLocation = glGetUniformLocation(instancedShaderProgram.program, "lamp.diffuse");
	instancedShaderProgram.locations.lampSpecularLocation = glGetUniformLocation(instancedShaderProgram.program, "lamp.specular");
	instancedShaderProgram.locations.lampPositionLocation = glGetUniformLocation(instancedShaderProgram.program, "lamp.position");

	instancedShaderProgram.locations.enableFogLocation = glGetUniformLocation(instancedShaderProgram.program, "enableFog");
	instancedShaderProgram.locations.fogColorLocation = glGetUniformLocation(instancedShaderProgram.program, "fogColor");
	instancedShaderProgram.locations.fogDensityLocation = glGetUniformLocation(instancedShaderProgram.program, "fogDensity");
	instancedShaderProgram.locations.fogTextureLocation = glGetUniformLocation(instancedShaderProgram.program, "fogSampler");

	instancedShaderProgram.locations.flashlightEnabledLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlightEnabled");
	instancedShaderProgram.locations.flashlightAmbientLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.ambient");
	instancedShaderProgram.locations.flashlightDiffuseLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.diffuse");
	instancedShaderProgram.locations.flashlightSpecularLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.specular");
	instancedShaderProgram.locations.flashlightPositionLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.position");
	instancedShaderProgram.locations.flashlightDirectionLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.spotDirection");
	instancedShaderProgram.locations.flashlightCutOffLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.spotCosCutOff");
	instancedShaderProgram.locations.flashlightExponentLocation = glGetUniformLocation(instancedShaderProgram.program, "flashlight.spotExponent");

	instancedShaderProgram.initialized = true;

	GLuint skyboxShaders[] = {
	pgr::createShaderFromFile(GL_VERTEX_SHADER, "skybox.vert"),
	pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "skybox.frag"),
	0
	};
	skyboxShaderProgram.program = pgr::createProgram(skyboxShaders);
	skyboxShaderProgram.locations.position = glGetAttribLocation(skyboxShaderProgram.program, "position");
	skyboxShaderProgram.locations.inversePVmatrix = glGetUniformLocation(skyboxShaderProgram.program, "inversePVmatrix");
	skyboxShaderProgram.locations.texSampler = glGetUniformLocation(skyboxShaderProgram.program, "skyboxSampler");
	skyboxShaderProgram.locations.shadowTintLocation = glGetUniformLocation(skyboxShaderProgram.program, "shadowTint");
	skyboxShaderProgram.initialized = true;
}

/**
 * \brief Delete all shader program objects.
 */
void cleanupShaderPrograms(void) {

	pgr::deleteProgramAndShaders(commonShaderProgram.program);
}

/**
 * \brief Initializes the HUD including 2D shaders, VAOs, and UI textures.
 */
void initHUD() {

	GLuint uiShaders[] = {
	  pgr::createShaderFromFile(GL_VERTEX_SHADER, "UIshader.vert"),
	  pgr::createShaderFromFile(GL_FRAGMENT_SHADER,  "UIshader.frag"),
	  0
	};
	uiShaderProgram.program = pgr::createProgram(uiShaders);
	uiShaderProgram.locations.position = glGetAttribLocation(uiShaderProgram.program, "position");
	uiShaderProgram.locations.texCoord = glGetAttribLocation(uiShaderProgram.program, "texCoord");
	uiShaderProgram.locations.PVMmatrix = glGetUniformLocation(uiShaderProgram.program, "PVM");
	uiShaderProgram.locations.texSampler = glGetUniformLocation(uiShaderProgram.program, "texSampler");
	uiShaderProgram.locations.timeLocation = glGetUniformLocation(uiShaderProgram.program, "time");
	uiShaderProgram.locations.isAnimatedLocation = glGetUniformLocation(uiShaderProgram.program, "isAnimated");


	float quadVertices[] = {
		0.0f, 1.0f,        0.0f, 1.0f,
		0.0f, 0.0f,        0.0f, 0.0f,
		1.0f, 1.0f,        1.0f, 1.0f,
		1.0f, 0.0f,        1.0f, 0.0f
	};

	glGenVertexArrays(1, &uiVao);
	glGenBuffers(1, &uiVbo);
	glBindVertexArray(uiVao);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(uiShaderProgram.locations.position);
	glVertexAttribPointer(uiShaderProgram.locations.position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(uiShaderProgram.locations.texCoord);
	glVertexAttribPointer(uiShaderProgram.locations.texCoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);


	iconTextures[0] = pgr::createTexture("UI/01.png");
	iconTextures[1] = pgr::createTexture("UI/02.png");
	iconTextures[2] = pgr::createTexture("UI/03.png");
	iconTextures[3] = pgr::createTexture("UI/04.png");
	iconTextures[4] = pgr::createTexture("UI/05.png");
	iconTextures[5] = pgr::createTexture("UI/heart.png");
}

/**
 * \brief Renders the HUD elements over the 3D scene.
 */
void drawHUD() {
	glUseProgram(uiShaderProgram.program);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	glm::mat4 ortho = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

	float time = 0.001f * static_cast<float>(glutGet(GLUT_ELAPSED_TIME));
	glUniform1f(uiShaderProgram.locations.timeLocation, time);

	float iconSize = 100.0f;
	float padding = 20.0f;
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(padding, padding, 0.0f));
	model = glm::scale(model, glm::vec3(iconSize, iconSize, 1.0f));

	glm::mat4 PVM = ortho * model;
	glUniformMatrix4fv(uiShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(PVM));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, iconTextures[itemSelection]);
	glUniform1i(uiShaderProgram.locations.texSampler, 0);

	glUniform1i(uiShaderProgram.locations.isAnimatedLocation, 0);

	glBindVertexArray(uiVao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glUniform1i(uiShaderProgram.locations.isAnimatedLocation, 1);

	glm::mat4 heartModel = glm::translate(glm::mat4(1.0f), glm::vec3(padding, height - iconSize - padding, 0.0f));
	heartModel = glm::scale(heartModel, glm::vec3(iconSize, iconSize, 1.0f));

	glm::mat4 heartPVM = ortho * heartModel;
	glUniformMatrix4fv(uiShaderProgram.locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(heartPVM));

	glBindTexture(GL_TEXTURE_2D, iconTextures[5]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

/**
 * \brief Renders the entire 3D scene for the current frame.
 *
 * \details This is the core rendering function called every frame by the display callback. It orchestrates
 * the visual state of the application and handles the rendering pipeline in several distinct phases:
 *
 * 1. **Camera Setup:** Computes the View and Projection matrices based on the active `cameraMode`:
 * - **Mode 0:** An orthographic camera that smoothly spins around the scene origin over time.
 * - **Mode 1:** A free perspective camera attached to the player (Fairy), utilizing mouse input for elevation and viewing angle.
 * - **Mode 2 & 3:** Fixed, static perspective cameras observing the scene from specific vantage points.
 *
 * 2. **Time of Day Simulation:** Calculates the sun's elevation based on elapsed time and the user-selected `timeOfDayState`.
 * It dynamically interpolates a global tint (`currentShadowTint`) transitioning smoothly between day, sunset, and night colors.
 *
 * 3. **Lighting Configuration:** Computes the world-to-eye space transformations for the scene's light sources (the static lamp and the player's flashlight).
 * These lighting parameters are uploaded to both the `commonShaderProgram` and the `instancedShaderProgram`.
 *
 * 4. **Environment Effects:** Uploads fog parameters (color, density, texture) to the shaders if the fog toggle is enabled.
 *
 * 5. **Skybox Rendering:** Renders the skybox behind all other geometry, provided that fog is currently disabled.
 *
 * 6. **Object Rendering:** Iterates through the global `objects` list, invoking the `draw` method on each entity.
 * If the first-person camera is active, the player avatar and its flashlight model are explicitly culled to prevent camera clipping.
 */
void drawScene(void)
{
	float targetRatio = 16.0f / 9.0f;
	float zoomFactor = 0.5f;

	float currentHeight = SCENE_HEIGHT * zoomFactor;

	float orthoWidth = currentHeight * targetRatio;

	const glm::mat4 orthoProjectionMatrix = glm::ortho(
		-orthoWidth, orthoWidth,
		-currentHeight, currentHeight,
		-10.0f * SCENE_DEPTH, 10.0f * SCENE_DEPTH
	);

	float time = 0.001f * static_cast<float>(glutGet(GLUT_ELAPSED_TIME));

	float spinSpeed = 0.1f;
	float radius = 2.0f;

	glm::vec3 spinningCameraPosition = glm::vec3(cos(time * spinSpeed) * radius, sin(time * spinSpeed) * radius, 1.0f);


	const glm::mat4 orthoViewMatrix = glm::lookAt(
		spinningCameraPosition,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	glm::mat4 viewMatrix = orthoViewMatrix;
	glm::mat4 projectionMatrix = orthoProjectionMatrix;

	if (cameraMode == 1) {

		float eyeHeight = 0.2f;
		float forwardOffset = 0.02f;

		glm::vec3 cameraPosition = player->position
			+ glm::vec3(0.0f, 0.0f, eyeHeight)
			+ (player->direction * forwardOffset);
		glm::vec3 cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cameraViewDirection = player->direction;

		glm::vec3 rotationAxis = glm::cross(cameraViewDirection, cameraUpVector);
		glm::mat4 cameraTransform = glm::rotate(glm::mat4(1.0f), glm::radians(cameraElevationAngle), rotationAxis);

		cameraUpVector = glm::vec3(cameraTransform * glm::vec4(cameraUpVector, 0.0f));
		cameraViewDirection = glm::vec3(cameraTransform * glm::vec4(cameraViewDirection, 0.0f));

		glm::vec3 cameraCenter = cameraPosition + cameraViewDirection;

		viewMatrix = glm::lookAt(
			cameraPosition,
			cameraCenter,
			cameraUpVector
		);

		projectionMatrix = glm::perspective(glm::radians(60.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10.0f);

		currentViewMatrix = viewMatrix;
		currentProjectionMatrix = projectionMatrix;
	}
	else if (cameraMode == 2) {

		glm::vec3 staticCamPosition = glm::vec3(0.0f, -2.1f, 0.3f);

		glm::vec3 staticCamTarget = glm::vec3(0.0f, 0.0f, 0.1f);

		glm::vec3 staticCamUp = glm::vec3(0.0f, 0.0f, 1.0f);

		viewMatrix = glm::lookAt(staticCamPosition, staticCamTarget, staticCamUp);


		projectionMatrix = glm::perspective(glm::radians(60.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 20.0f);

		currentViewMatrix = viewMatrix;
		currentProjectionMatrix = projectionMatrix;
	}
	else if (cameraMode == 3) {

		glm::vec3 staticCamPosition = glm::vec3(0.0f, 2.1f, 0.3f);

		glm::vec3 staticCamTarget = glm::vec3(0.0f, 0.0f, 0.1f);

		glm::vec3 staticCamUp = glm::vec3(0.0f, 0.0f, 1.0f);

		viewMatrix = glm::lookAt(staticCamPosition, staticCamTarget, staticCamUp);


		projectionMatrix = glm::perspective(glm::radians(60.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 20.0f);

		currentViewMatrix = viewMatrix;
		currentProjectionMatrix = projectionMatrix;
	}
	else {

		currentViewMatrix = viewMatrix;
		currentProjectionMatrix = projectionMatrix;
	}

	float sunAngle = time * sunSpeed;
	float elevation;

	switch (timeOfDayState) {
	case 0:
		elevation = sin(sunAngle);
		break;
	case 1:
		elevation = 0.1f;
		break;
	case 2:
		elevation = 1.0f;
		break;
	case 3:
		elevation = 0.0f;
		break;
	case 4:
		elevation = -1.0f;
		break;
	}

	glm::vec3 dayTint(0.65f, 0.65f, 0.65f);
	glm::vec3 sunsetTint(1.0f, 0.45f, 0.55f);
	glm::vec3 nightTint(0.1f, 0.1f, 0.3f);

	glm::vec3 currentShadowTint;

	if (elevation >= 0.4f) {
		currentShadowTint = dayTint;
	}
	else if (elevation >= 0.0f) {
		float factor = elevation / 0.4f;
		factor = factor * factor * (3.0f - 2.0f * factor);
		currentShadowTint = glm::mix(sunsetTint, dayTint, factor);
	}
	else if (elevation >= -0.4f) {

		float factor = (elevation + 0.4f) / 0.4f;

		factor = factor * factor * (3.0f - 2.0f * factor);

		currentShadowTint = glm::mix(nightTint, sunsetTint, factor);
	}
	else {
		currentShadowTint = nightTint;
	}

	glm::vec4 flashWorldPos = glm::vec4(playerFlashlight->position, 1.0f);
	flashWorldPos.z += 0.05f;
	glm::vec3 flashEyePos = glm::vec3(viewMatrix * flashWorldPos);

	glm::vec4 flashWorldDir = glm::vec4(player->direction, 0.0f);
	glm::vec3 flashEyeDir = glm::normalize(glm::vec3(viewMatrix * flashWorldDir));

	glUseProgram(commonShaderProgram.program);

	glUniform1i(commonShaderProgram.locations.flashlightEnabledLocation, isFlashlightEnabled ? 1 : 0);
	if (isFlashlightEnabled) {
		glUniform3f(commonShaderProgram.locations.flashlightAmbientLocation, 0.05f, 0.05f, 0.05f);
		glUniform3f(commonShaderProgram.locations.flashlightDiffuseLocation, 0.8f, 0.8f, 0.7f);
		glUniform3f(commonShaderProgram.locations.flashlightSpecularLocation, 1.0f, 1.0f, 1.0f);
		glUniform3fv(commonShaderProgram.locations.flashlightPositionLocation, 1, glm::value_ptr(flashEyePos));
		glUniform3fv(commonShaderProgram.locations.flashlightDirectionLocation, 1, glm::value_ptr(flashEyeDir));
		glUniform1f(commonShaderProgram.locations.flashlightCutOffLocation, cos(glm::radians(20.0f))); // 20 degree cone
		glUniform1f(commonShaderProgram.locations.flashlightExponentLocation, 2.0f);
	}

	glUniform1f(commonShaderProgram.locations.timeLocation, time);
	glUniform3fv(commonShaderProgram.locations.shadowTintLocation, 1, glm::value_ptr(currentShadowTint));
	glUniform1f(commonShaderProgram.locations.sunSpeedLocation, sunSpeed);

	glm::vec4 lampWorldPosition = glm::vec4(2.5f, -1.9f, 0.0f, 1.0f);

	glm::vec3 lampEyePosition = glm::vec3(viewMatrix * lampWorldPosition);

	glUniform3f(commonShaderProgram.locations.lampAmbientLocation, 0.1f, 0.1f, 0.05f);
	glUniform3f(commonShaderProgram.locations.lampDiffuseLocation, 0.5f, 0.4f, 0.2f);
	glUniform3f(commonShaderProgram.locations.lampSpecularLocation, 0.5f, 0.45f, 0.25f);

	glUniform1i(commonShaderProgram.locations.enableFogLocation, isFogEnabled ? 1 : 0);
	glUniform3f(commonShaderProgram.locations.fogColorLocation, 0.55f, 0.75f, 0.95f);
	glUniform1f(commonShaderProgram.locations.fogDensityLocation, 0.25f);
	glUniform1i(commonShaderProgram.locations.fogTextureLocation, 1);

	glUniform3fv(commonShaderProgram.locations.lampPositionLocation, 1, glm::value_ptr(lampEyePosition));

	glUniform2f(commonShaderProgram.locations.texOffsetLocation, 0.0f, 0.0f);
	glUniform2f(commonShaderProgram.locations.texSpriteSizeLocation, 1.0f, 1.0f);

	glUseProgram(instancedShaderProgram.program);

	glUniform1i(instancedShaderProgram.locations.flashlightEnabledLocation, isFlashlightEnabled ? 1 : 0);
	if (isFlashlightEnabled) {
		glUniform3f(instancedShaderProgram.locations.flashlightAmbientLocation, 0.05f, 0.05f, 0.05f);
		glUniform3f(instancedShaderProgram.locations.flashlightDiffuseLocation, 0.8f, 0.8f, 0.7f);
		glUniform3f(instancedShaderProgram.locations.flashlightSpecularLocation, 1.0f, 1.0f, 1.0f);
		glUniform3fv(instancedShaderProgram.locations.flashlightPositionLocation, 1, glm::value_ptr(flashEyePos));
		glUniform3fv(instancedShaderProgram.locations.flashlightDirectionLocation, 1, glm::value_ptr(flashEyeDir));
		glUniform1f(instancedShaderProgram.locations.flashlightCutOffLocation, cos(glm::radians(20.0f)));
		glUniform1f(instancedShaderProgram.locations.flashlightExponentLocation, 2.0f);
	}

	glUniform1f(instancedShaderProgram.locations.timeLocation, time);
	glUniform3fv(instancedShaderProgram.locations.shadowTintLocation, 1, glm::value_ptr(currentShadowTint));
	glUniform1f(instancedShaderProgram.locations.sunSpeedLocation, sunSpeed);

	glUniform3f(instancedShaderProgram.locations.lampAmbientLocation, 0.1f, 0.1f, 0.05f);
	glUniform3f(instancedShaderProgram.locations.lampDiffuseLocation, 0.5f, 0.4f, 0.2f);
	glUniform3f(instancedShaderProgram.locations.lampSpecularLocation, 0.5f, 0.45f, 0.25f);
	glUniform3fv(instancedShaderProgram.locations.lampPositionLocation, 1, glm::value_ptr(lampEyePosition));

	glUniform1i(instancedShaderProgram.locations.enableFogLocation, isFogEnabled ? 1 : 0);
	glUniform3f(instancedShaderProgram.locations.fogColorLocation, 0.55f, 0.75f, 0.95f);
	glUniform1f(instancedShaderProgram.locations.fogDensityLocation, 0.25f);
	glUniform1i(instancedShaderProgram.locations.fogTextureLocation, 1);

	if (isFogEnabled) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fogTexture);

		glActiveTexture(GL_TEXTURE0);
	}

	if (skybox != nullptr && !isFogEnabled) {
		glDepthFunc(GL_LEQUAL);

		glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix));

		glUseProgram(skyboxShaderProgram.program);

		glUniform3fv(skyboxShaderProgram.locations.shadowTintLocation, 1, glm::value_ptr(currentShadowTint));

		skybox->draw(skyboxViewMatrix, projectionMatrix);

		glDepthFunc(GL_LESS);
	}


	for (ObjectInstance* object : objects) {
		if (object != nullptr)
			if (cameraMode == 1 && object == player || cameraMode == 1 && object == playerFlashlight) {
				continue;
			}
		object->draw(viewMatrix, projectionMatrix);

	}

}


// -----------------------  Window callbacks ---------------------------------

/**
 * \brief Draw the window contents.
 */
void displayCb() {
	glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawScene();

	if (cameraMode == 1) drawHUD();

	glutSwapBuffers();
}

/**
 * \brief Window was reshaped.
 * \param newWidth New window width
 * \param newHeight New window height
 */
void reshapeCb(int newWidth, int newHeight) {
	if (newHeight == 0) newHeight = 1;

	float targetRatio = 16.0f / 9.0f;
	float windowRatio = (float)newWidth / (float)newHeight;

	int viewportWidth, viewportHeight;
	int viewportX = 0, viewportY = 0;

	if (windowRatio > targetRatio) {
		viewportHeight = newHeight;
		viewportWidth = (int)(newHeight * targetRatio);
		viewportX = (newWidth - viewportWidth) / 2;
	}
	else {
		viewportWidth = newWidth;
		viewportHeight = (int)(newWidth / targetRatio);
		viewportY = (newHeight - viewportHeight) / 2;
	}

	glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
};

// -----------------------  Keyboard ---------------------------------

/**
 * \brief Handle mouse movement over the window (with no button pressed).
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void passiveMouseMotionCb(int mouseX, int mouseY) {
	int currentWidth = glutGet(GLUT_WINDOW_WIDTH);
	int currentHeight = glutGet(GLUT_WINDOW_HEIGHT);

	int centerX = currentWidth / 2;
	int centerY = currentHeight / 2;

	if (mouseX != centerX || mouseY != centerY) {
		const float cameraElevationAngleDelta = 0.5f * (centerY - mouseY);

		if (fabs(targetElevationAngle + cameraElevationAngleDelta) < CAMERA_ELEVATION_MAX) {
			targetElevationAngle += cameraElevationAngleDelta;
		}

		const float cameraViewAngleDelta = 0.5f * (centerX - mouseX);
		targetViewAngle += cameraViewAngleDelta;

		glutWarpPointer(centerX, centerY);
	}
}

/**
 * \brief Handle right-click context menu options.
 * \param value The selected menu entry index
 */
void menuCb(int value) {
	switch (value) {
	case 1:
		cameraMode = 0;
		glutPassiveMotionFunc(NULL);
		glutSetCursor(GLUT_CURSOR_INHERIT);
		break;
	case 2:
		cameraMode = 1;
		glutPassiveMotionFunc(passiveMouseMotionCb);
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		break;
	case 3:
		cameraMode = 2;
		glutPassiveMotionFunc(NULL);
		glutSetCursor(GLUT_CURSOR_INHERIT);
		break;
	case 4:
		cameraMode = 3;
		glutPassiveMotionFunc(NULL);
		glutSetCursor(GLUT_CURSOR_INHERIT);
		break;
	case 5: timeOfDayState = 0; break;
	case 6: timeOfDayState = 1; break;
	case 7: timeOfDayState = 2; break;
	case 8: timeOfDayState = 3; break;
	case 9: timeOfDayState = 4; break;
	}
	glutPostRedisplay();
}

/**
 * \brief Handle the key pressed event.
 * Called whenever a key on the keyboard was pressed. The key is given by the "keyPressed"
 * parameter, which is an ASCII character. It's often a good idea to have the escape key (ASCII value 27)
 * to call glutLeaveMainLoop() to exit the program.
 * \param keyPressed ASCII code of the key
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void keyboardCb(unsigned char keyPressed, int mouseX, int mouseY) {

	if (keyPressed == 27) {
		glutLeaveMainLoop();
		exit(EXIT_SUCCESS);
	}

	if (keyPressed == 'c') {
		cameraMode += 1;
		if (cameraMode == 4) cameraMode = 0;
	}
	if (cameraMode == 1) {

		glutPassiveMotionFunc(passiveMouseMotionCb);
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	}
	else {
		glutPassiveMotionFunc(NULL);
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

	if (keyPressed == 'w' || keyPressed == 'W') {
		keyMap[KEY_UP] = true;
	}
	if (keyPressed == 's' || keyPressed == 'S') {
		keyMap[KEY_DOWN] = true;
	}
	if (keyPressed == 'd' || keyPressed == 'D') {
		keyMap[KEY_RIGHT] = true;
	}
	if (keyPressed == 'a' || keyPressed == 'A') {
		keyMap[KEY_LEFT] = true;
	}
	if (keyPressed == 'q' || keyPressed == 'Q') {
		itemSelection += 1;
		if (itemSelection == 5) itemSelection = 0;
	}
	if (keyPressed == 't' || keyPressed == 'T') {
		timeOfDayState++;
		if (timeOfDayState > 4) timeOfDayState = 0;

		switch (timeOfDayState) {
		case 0: std::cout << "Time: Auto" << std::endl; break;
		case 1: std::cout << "Time: Morning" << std::endl; break;
		case 2: std::cout << "Time: Noon" << std::endl; break;
		case 3: std::cout << "Time: Evening" << std::endl; break;
		case 4: std::cout << "Time: Midnight" << std::endl; break;
		}
	}
	if (keyPressed == 'f' || keyPressed == 'F') {
		isFogEnabled = !isFogEnabled;
	}
	if (keyPressed == 'l' || keyPressed == 'L') {
		isFlashlightEnabled = !isFlashlightEnabled;
	}
}

// Called whenever a key on the keyboard was released. The key is given by
// the "keyReleased" parameter, which is in ASCII. 
/**
 * \brief Handle the key released event.
 * \param keyReleased ASCII code of the released key
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void keyboardUpCb(unsigned char keyReleased, int mouseX, int mouseY) {
	if (keyReleased == 'w' || keyReleased == 'W') {
		keyMap[KEY_UP] = false;
	}
	if (keyReleased == 's' || keyReleased == 'S') {
		keyMap[KEY_DOWN] = false;
	}
	if (keyReleased == 'd' || keyReleased == 'D') {
		keyMap[KEY_RIGHT] = false;
	}
	if (keyReleased == 'a' || keyReleased == 'A') {
		keyMap[KEY_LEFT] = false;
	}
}

//
/**
 * \brief Handle the non-ASCII key pressed event (such as arrows or F1).
 *  The special keyboard callback is triggered when keyboard function (Fx) or directional
 *  keys are pressed.
 * \param specKeyPressed int value of a predefined glut constant such as GLUT_KEY_UP
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void specialKeyboardCb(int specKeyPressed, int mouseX, int mouseY) {
	if (specKeyPressed == GLUT_KEY_SHIFT_L) {
		keyMap[KEY_SHIFT] = true;
	}
}

void specialKeyboardUpCb(int specKeyReleased, int mouseX, int mouseY) {
	if (specKeyReleased == GLUT_KEY_SHIFT_L) {
		keyMap[KEY_SHIFT] = false;
	}
}

/**
 * \brief Spawns an explosion effect in the scene at a given location.
 * \param position A glm::vec3 containing the world coordinates for the explosion.
 */
void spawnExplosion(glm::vec3 position) {
	Explosion* boom = new Explosion(&commonShaderProgram);
	boom->position = position;
	boom->position.z += 0.2f;
	boom->size = 1.0f;
	boom->rotation = 0.0f;
	boom->textureID = explosionTextureID;
	objects.push_back(boom);
	std::cout << "hit" << std::endl;
}

// -----------------------  Mouse ---------------------------------
// three events - mouse click, mouse drag, and mouse move with no button pressed

// 
/**
 * \brief React to mouse button press and release (mouse click).
 * When the user presses and releases mouse buttons in the window, each press
 * and each release generates a mouse callback (including release after dragging).
 *
 * \param buttonPressed button code (GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON)
 * \param buttonState GLUT_DOWN when pressed, GLUT_UP when released
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void mouseCb(int buttonPressed, int buttonState, int mouseX, int mouseY) {
	if (buttonPressed == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		float winY = (float)viewport[3] - (float)mouseY;

		glm::vec3 rayStart = glm::unProject(
			glm::vec3(mouseX, winY, 0.0f),
			currentViewMatrix,
			currentProjectionMatrix,
			glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3])
		);

		glm::vec3 rayEnd = glm::unProject(
			glm::vec3(mouseX, winY, 1.0f),
			currentViewMatrix,
			currentProjectionMatrix,
			glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3])
		);

		glm::vec3 rayDir = glm::normalize(rayEnd - rayStart);

		if (globalBird != nullptr) {
			float elapsedTime = 0.001f * static_cast<float>(glutGet(GLUT_ELAPSED_TIME));
			float radius = 2.5f;
			float speed = 1.0f;
			float height = 2.0f;

			float offsetX = radius * cos(elapsedTime * speed);
			float offsetY = radius * sin(elapsedTime * speed);

			glm::vec3 actualBirdPos = globalBird->position;
			actualBirdPos.z = height;
			actualBirdPos += glm::vec3(offsetX, offsetY, 0.0f);

			glm::vec3 oc = rayStart - actualBirdPos;
			float b = glm::dot(oc, rayDir);
			float hitRadius = 0.3f;
			float c = glm::dot(oc, oc) - (hitRadius * hitRadius);

			if (b * b - c > 0 && b < 0.0f) {
				spawnExplosion(actualBirdPos);
			}
		}

		if (rayDir.z != 0.0f) {
			float t = -rayStart.z / rayDir.z;

			if (t > 0.0f && cameraMode == 1) {
				glm::vec3 intersection = rayStart + t * rayDir;

				if (intersection.x <= 2.75f && intersection.x >= -2.75f && intersection.y <= 2.75f && intersection.y >= -2.7f) {
					if (flowerField != nullptr && itemSelection == 0) {
						flowerField->addInstance(glm::vec3(intersection.x, intersection.y, intersection.z - 0.02f));
					}

					if (rocksField != nullptr && itemSelection == 4) {
						rocksField->addInstance(glm::vec3(intersection.x, intersection.y, intersection.z));
					}

					if (grassField != nullptr && itemSelection == 3) {
						grassField->addInstance(glm::vec3(intersection.x, intersection.y, intersection.z));
					}

					if (bellField != nullptr && itemSelection == 1) {
						bellField->addInstance(glm::vec3(intersection.x, intersection.y, intersection.z + 0.05f));
					}

					if (daisyField != nullptr && itemSelection == 2) {
						daisyField->addInstance(glm::vec3(intersection.x, intersection.y, intersection.z));
					}
				}
				glutPostRedisplay();

			}
		}
	}
}

/**
 * \brief Handle mouse dragging (mouse move with any button pressed).
 *        This event follows the glutMouseFunc(mouseCb) event.
 * \param mouseX mouse (cursor) X position
 * \param mouseY mouse (cursor) Y position
 */
void mouseMotionCb(int mouseX, int mouseY) {
}



// -----------------------  Timer ---------------------------------

/**
 * \brief Callback responsible for the scene update.
 */
void timerCb(int)
{

	cameraElevationAngle = glm::mix(cameraElevationAngle, targetElevationAngle, 0.3f);
	player->viewAngle = glm::mix(player->viewAngle, targetViewAngle, 0.3f);

	player->direction = glm::vec3(
		cos(glm::radians(player->viewAngle)),
		sin(glm::radians(player->viewAngle)),
		0.0f
	);

	if (keyMap[KEY_SHIFT] == true) {
		player_speed = 0.02f;
	}
	else {
		player_speed = 0.005f;
	}

	glm::vec3 nextPosition = player->position;

	if (keyMap[KEY_RIGHT] == true && cameraMode == 1)
		nextPosition -= glm::vec3(-player->direction.y, player->direction.x, 0.0f) * player_speed;

	if (keyMap[KEY_LEFT] == true && cameraMode == 1)
		nextPosition += glm::vec3(-player->direction.y, player->direction.x, 0.0f) * player_speed;

	if (keyMap[KEY_UP] == true && cameraMode == 1)
		nextPosition += player->direction * player_speed;

	if (keyMap[KEY_DOWN] == true && cameraMode == 1)
		nextPosition -= player->direction * player_speed;

	const float minBoundary = -2.55f;
	const float maxBoundary = 2.55f;

	nextPosition.x = glm::clamp(nextPosition.x, minBoundary, maxBoundary);
	nextPosition.y = glm::clamp(nextPosition.y, minBoundary, maxBoundary);

	player->position = nextPosition;

	playerFlashlight->position = player->position;
	playerFlashlight->direction = player->direction;
	playerFlashlight->viewAngle = player->viewAngle;

	const glm::mat4 sceneRootMatrix = glm::mat4(1.0f);
	float elapsedTime = 0.001f * static_cast<float>(glutGet(GLUT_ELAPSED_TIME));

	for (ObjectInstance* object : objects) {
		if (object != nullptr)
			object->update(elapsedTime, &sceneRootMatrix);
	}

	for (auto it = objects.begin(); it != objects.end(); ) {
		Explosion* exp = dynamic_cast<Explosion*>(*it);
		if (exp != nullptr && exp->isDead) {
			delete* it;
			it = objects.erase(it);
		}
		else {
			++it;
		}
	}

	glutTimerFunc(16, timerCb, 0);

	glutPostRedisplay();
}


// -----------------------  Application ---------------------------------

/**
 * \brief Helper to instantiate and place a small tree model.
 * \param coorX X-coordinate
 * \param coorY Y-coordinate
 * \param coorZ Z-coordinate
 * \param t_size Scale factor of the tree
 * \param t_rotate Rotation in degrees around the Z axis
 */
void placeSmallTree(float coorX, float coorY, float coorZ, float t_size, float t_rotate) {
	auto smallTree = new Tree00(&commonShaderProgram);
	smallTree->size = t_size;
	smallTree->position = glm::vec3(coorX, coorY, coorZ);
	smallTree->rotation = t_rotate;
	smallTree->textureID = pgr::createTexture("textures/tree_texture.png");
	objects.push_back(smallTree);
}

/**
 * \brief Helper to instantiate and place a big tree model.
 * \param coorX X-coordinate
 * \param coorY Y-coordinate
 * \param coorZ Z-coordinate
 * \param t_size Scale factor of the tree
 */
void placeBigTree(float coorX, float coorY, float coorZ, float t_size) {
	auto bigTree = new Tree01(&commonShaderProgram);
	bigTree->size = t_size;
	bigTree->position = glm::vec3(coorX, coorY, coorZ);
	bigTree->textureID = pgr::createTexture("textures/tree_texture01.png");
	objects.push_back(bigTree);
}

/**
 * \brief Helper to instantiate and place a large bush model.
 * \param coorX X-coordinate
 * \param coorY Y-coordinate
 * \param coorZ Z-coordinate
 * \param t_size Scale factor of the bush
 * \param t_rotate Rotation in degrees around the Z axis
 * \param t_path Path to the texture image
 */
void placeLargeBush(float coorX, float coorY, float coorZ, float t_size, float t_rotate, std::string t_path) {
	LargeBush* largeBush = new LargeBush(&commonShaderProgram);
	largeBush->size = t_size;
	largeBush->position = glm::vec3(coorX, coorY, coorZ);
	largeBush->rotation = t_rotate;
	largeBush->textureID = pgr::createTexture(t_path);
	objects.push_back(largeBush);
}

/**
 * \brief Helper to instantiate and place a medium bush model.
 * \param coorX X-coordinate
 * \param coorY Y-coordinate
 * \param coorZ Z-coordinate
 * \param t_size Scale factor of the bush
 * \param t_rotate Rotation in degrees around the Z axis
 * \param t_path Path to the texture image
 */
void placeMediumBush(float coorX, float coorY, float coorZ, float t_size, float t_rotate, std::string t_path) {
	MediumBush* mediumBush = new MediumBush(&commonShaderProgram);
	mediumBush->size = t_size;
	mediumBush->position = glm::vec3(coorX, coorY, coorZ);
	mediumBush->rotation = t_rotate;
	mediumBush->textureID = pgr::createTexture(t_path);
	objects.push_back(mediumBush);
}

//				y
//                ^
//              3 |
//                |
//              2 |
//                |
//              1 |
//                |
//<---+---+---+---+---+---+---+---> x
//   -3  -2  -1   0   1   2   3
//             -1 |
//                |
//             -2 |
//                |
//             -3 |
//				  -

/**
 * \brief Generates the perimeter fencing for the scene using multiple Fence objects and a Gate.
 */
void placeAllFence() {

	float coor = 2.45f;

	for (int i = 0; i <= 7; i++) {
		auto fence = new Fence(&commonShaderProgram);
		fence->size = 0.2f;
		fence->position = glm::vec3(coor, -2.75f, 0.0f);
		coor -= 0.7f;
		fence->rotation = 0.0f;
		fence->textureID = pgr::createTexture("textures/fence.png");
		objects.push_back(fence);
	}

	coor = 2.45f;

	for (int i = 0; i <= 7; i++) {

		if (i == 4) {
			auto gate = new Gate(&commonShaderProgram);
			gate->size = 0.2f;
			gate->position = glm::vec3(coor, 2.8f, 0.0f);
			coor -= 0.7f;
			gate->rotation = 0.0f;
			gate->textureID = pgr::createTexture("textures/fence.png");
			objects.push_back(gate);
			continue;
		}

		auto fence = new Fence(&commonShaderProgram);
		fence->size = 0.2f;
		fence->position = glm::vec3(coor, 2.8f, 0.0f);
		coor -= 0.7f;
		fence->rotation = 180.0f;
		fence->textureID = pgr::createTexture("textures/fence.png");
		objects.push_back(fence);
	}

	coor = 2.45f;

	for (int i = 0; i <= 7; i++) {
		auto fence = new Fence(&commonShaderProgram);
		fence->size = 0.2f;
		fence->position = glm::vec3(2.8f, coor, 0.0f);
		coor -= 0.7f;
		fence->rotation = 90.0f;
		fence->textureID = pgr::createTexture("textures/fence.png");
		objects.push_back(fence);
	}

	coor = -2.4f;

	for (int i = 0; i <= 7; i++) {
		auto fence = new Fence(&commonShaderProgram);
		fence->size = 0.2f;
		fence->position = glm::vec3(-2.8f, coor, 0.0f);
		coor += 0.7f;
		fence->rotation = 270.0f;
		fence->textureID = pgr::createTexture("textures/fence.png");
		objects.push_back(fence);
	}
}

/**
 * \brief Populates the scene with predefined tree structures.
 */
void placeAllTrees() {

	placeSmallTree(2.4f, 2.2f, 0.0f, 0.07f, 80.0f);

	placeSmallTree(2.2f, 0.4f, 0.0f, 0.09f, 120.0f);

	placeSmallTree(-2.3f, -2.2f, 0.0f, 0.07f, 80.0f);

	placeSmallTree(-2.2f, -0.4f, 0.0f, 0.09f, 120.0f);

	placeSmallTree(-2.5f, 1.2f, 0.0f, 0.07f, 150.0f);

	placeSmallTree(2.5f, -1.2f, 0.0f, 0.07f, 200.0f);

	placeBigTree(1.5f, 2.5f, 0.0f, 0.1f);

	placeBigTree(-1.5f, -2.5f, 0.0f, 0.1f);

	placeBigTree(-2.3f, 2.3f, 0.0f, 0.1f);

	placeBigTree(2.3f, -2.3f, 0.0f, 0.1f);

}

/**
 * \brief Populates the scene with predefined bush models.
 */
void placeAllBushes() {

	placeLargeBush(2.4f, 2.4f, 0.10f, 2.7f, 0.0f, "textures/green1.png");

	placeLargeBush(2.5f, 0.4f, 0.05f, 2.5f, 300.0f, "textures/green0.png");

	placeMediumBush(1.35f, 2.2f, 0.05f, 0.1f, 330.0f, "textures/green0.png");

	placeLargeBush(-2.4f, -2.4f, 0.10f, 2.5f, 0.0f, "textures/green1.png");

	placeLargeBush(-2.5f, -0.4f, 0.05f, 2.5f, 300.0f, "textures/green0.png");

	placeMediumBush(-1.35f, -2.4f, 0.05f, 0.1f, 330.0f, "textures/green0.png");

	placeLargeBush(-2.4f, 2.45f, 0.10f, 2.7f, 260.0f, "textures/green1.png");

	placeLargeBush(2.4f, -2.45f, 0.10f, 2.7f, 260.0f, "textures/green0.png");

	placeMediumBush(-2.4f, 1.35f, 0.05f, 0.1f, 280.0f, "textures/green0.png");

	placeMediumBush(2.4f, -1.35f, 0.05f, 0.1f, 0.0f, "textures/green1.png");
}

/**
 * \brief Initialize application data, OpenGL environment, scene models, textures, and instanced fields.
 */
void initApplication() {

	loadShaderPrograms();
	initHUD();
	glEnable(GL_DEPTH_TEST);

	fogTexture = pgr::createTexture("textures/fog_texture.png");

	explosionTextureID = pgr::createTexture("textures/boom.png");

	auto newField = new Field(&commonShaderProgram);
	newField->size = 2.9f;
	newField->position = glm::vec3(0.01f, -0.025f, 0.0f);
	newField->textureID = pgr::createTexture("textures/grass_texture.png");
	objects.push_back(newField);

	auto newFairy = new Fairy(&commonShaderProgram);
	newFairy->size = 0.03f;
	newFairy->position = glm::vec3(0.0f, 0.0f, 0.1f);
	newFairy->textureID = pgr::createTexture("textures/fairy.png");
	newFairy->viewAngle = 90.0f;
	newFairy->direction = glm::vec3(cos(glm::radians(newFairy->viewAngle)), sin(glm::radians(newFairy->viewAngle)), 0.0f);
	player = newFairy;
	objects.push_back(newFairy);

	placeAllTrees();
	placeAllBushes();
	placeAllFence();

	Lantern* lantern = new Lantern(&commonShaderProgram);
	lantern->size = 0.4f;
	lantern->position = glm::vec3(2.5f, -1.9f, 0.0f);
	lantern->rotation = 0.0f;
	lantern->textureID = pgr::createTexture("textures/lampOn.png");
	objects.push_back(lantern);

	Bench* bench = new Bench(&commonShaderProgram);
	bench->size = 0.75f;
	bench->position = glm::vec3(0.0f, -2.1f, 0.0f);
	bench->rotation = 0.0f;
	bench->textureID = pgr::createTexture("textures/brown.png");
	objects.push_back(bench);

	Bird* bird = new Bird(&commonShaderProgram);
	bird->size = 0.003f;
	bird->position = glm::vec3(0.0f, 0.0f, 0.0f);
	bird->rotation = 0.0f;
	bird->textureID = pgr::createTexture("textures/bird.png");
	globalBird = bird;
	objects.push_back(bird);

	Flashlight* flashlight = new Flashlight(&commonShaderProgram);
	flashlight->size = 0.03f;
	flashlight->position = newFairy->position;
	flashlight->viewAngle = newFairy->viewAngle;
	flashlight->direction = glm::vec3(cos(glm::radians(newFairy->viewAngle)), sin(glm::radians(newFairy->viewAngle)), 0.0f);
	flashlight->textureID = pgr::createTexture("textures/rocks.png");
	playerFlashlight = flashlight;
	objects.push_back(flashlight);

	flowerField = new FlowerInstanced(&instancedShaderProgram);
	flowerField->textureID = pgr::createTexture("textures/flower00.png");
	objects.push_back(flowerField);

	rocksField = new RocksInstanced(&instancedShaderProgram);
	rocksField->textureID = pgr::createTexture("textures/rocks.png");
	objects.push_back(rocksField);

	grassField = new GrassInstanced(&instancedShaderProgram);
	grassField->textureID = pgr::createTexture("textures/green0.png");
	objects.push_back(grassField);

	bellField = new BellInstanced(&instancedShaderProgram);
	bellField->textureID = pgr::createTexture("textures/flower01.png");
	objects.push_back(bellField);

	daisyField = new DaisyInstanced(&instancedShaderProgram);
	daisyField->textureID = pgr::createTexture("textures/flower02.png");
	objects.push_back(daisyField);

	skybox = new SkyBox(&skyboxShaderProgram);

}

/**
 * \brief Delete all OpenGL objects and application data.
 */
void finalizeApplication(void) {
	cleanupShaderPrograms();
}


/**
 * \brief Entry point of the application.
 * \param argc number of command line arguments
 * \param argv array with argument strings
 * \return 0 if OK, <> elsewhere
 */
int main(int argc, char** argv) {

	glutInit(&argc, argv);

	glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	{
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutCreateWindow(WINDOW_TITLE);

		glutDisplayFunc(displayCb);
		glutReshapeFunc(reshapeCb);
		glutKeyboardFunc(keyboardCb);
		glutKeyboardUpFunc(keyboardUpCb);
		glutSpecialFunc(specialKeyboardCb); 
		glutSpecialUpFunc(specialKeyboardUpCb); 
		glutMouseFunc(mouseCb);

		glutTimerFunc(16, timerCb, 0);

		int timeMenu = glutCreateMenu(menuCb);
		glutAddMenuEntry("Auto", 5);
		glutAddMenuEntry("Morning", 6);
		glutAddMenuEntry("Noon", 7);
		glutAddMenuEntry("Evening", 8);
		glutAddMenuEntry("Midnight", 9);

		int cameraMenu = glutCreateMenu(menuCb);
		glutAddMenuEntry("Spinning Camera", 1);
		glutAddMenuEntry("Free Camera", 2);
		glutAddMenuEntry("Static Camera 1", 3);
		glutAddMenuEntry("Static Camera 2", 4);

		glutCreateMenu(menuCb);
		glutAddSubMenu("Camera Modes", cameraMenu);
		glutAddSubMenu("Time of Day", timeMenu);

		glutAttachMenu(GLUT_RIGHT_BUTTON);

	}

	if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR))
		pgr::dieWithError("pgr init failed, required OpenGL not supported?");

	initApplication();

	glutCloseFunc(finalizeApplication);

	glutMainLoop();


	return EXIT_SUCCESS;
}