//----------------------------------------------------------------------------------------
/**
 * \file    fairy.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Fairy class.
 */


#include <iostream>
#include "allObjects.h"

#pragma warning( disable : 4305 )


void Fairy::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::mat4(1.0f);
    localModelMatrix = glm::translate(glm::mat4(1.0f), position);

    float angle = std::atan2(direction.y, direction.x);
    float offset = glm::radians(90.0f);

    localModelMatrix = glm::rotate(localModelMatrix, angle + offset, glm::vec3(0.0f, 0.0f, 1.0f));

    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}

void Fairy::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	if (initialized && (shaderProgram != nullptr)) {
		glUseProgram(shaderProgram->program);

		setTransformUniforms(shaderProgram, globalModelMatrix, viewMatrix, projectionMatrix);

		setMaterialUniforms(
            shaderProgram,
			geometry->ambient,
			geometry->diffuse,
			geometry->specular,
			geometry->shininess,
			textureID
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1f(shaderProgram->locations.texScaleLocation, 1.0f);

        unsigned int numVertices = geometry->numTriangles * 3;
        glBindVertexArray(geometry->vertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, numVertices);

		glBindVertexArray(0);
	}
	else {
		std::cerr << "Can't draw fairy" << std::endl;
	}
}

Fairy::Fairy(ShaderProgram* shdrPrg) : ObjectInstance(shdrPrg), initialized(false)
{
    geometry = new ObjectGeometry;

    geometry->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    geometry->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    geometry->specular = glm::vec3(0.0f, 0.0f, 0.0f);
    geometry->shininess = 10.0f;

    std::vector<glm::vec3> obj_vertices;
    std::vector<glm::vec2> obj_uvs;
    std::vector<glm::vec3> obj_normals;

    bool loaded = loadOBJ("models/fairy.obj", obj_vertices, obj_uvs, obj_normals);

    if (!loaded) {
        std::cerr << "Failed to load fairy.obj!" << std::endl;
        return;
    }

    geometry->numTriangles = obj_vertices.size() / 3;

    glGenVertexArrays(1, &(geometry->vertexArrayObject));
    glBindVertexArray(geometry->vertexArrayObject);

    GLuint vboPositions;
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(glm::vec3), &obj_vertices[0], GL_STATIC_DRAW);

    if (shaderProgram->locations.position != -1) {
        glEnableVertexAttribArray(shaderProgram->locations.position);
        glVertexAttribPointer(shaderProgram->locations.position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    GLuint vboUVs;
    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glBufferData(GL_ARRAY_BUFFER, obj_uvs.size() * sizeof(glm::vec2), &obj_uvs[0], GL_STATIC_DRAW);

    if (shaderProgram->locations.texCoord != -1) {
        glEnableVertexAttribArray(shaderProgram->locations.texCoord);
        glVertexAttribPointer(shaderProgram->locations.texCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, obj_normals.size() * sizeof(glm::vec3), &obj_normals[0], GL_STATIC_DRAW);

    if (shaderProgram->locations.normal != -1) {
        glEnableVertexAttribArray(shaderProgram->locations.normal);
        glVertexAttribPointer(shaderProgram->locations.normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    glBindVertexArray(0);
    initialized = true;
}

Fairy::~Fairy() {
	glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
	glDeleteBuffers(1, &(geometry->vertexBufferObject));

	delete geometry;
	geometry = nullptr;

	initialized = false;
}
