//----------------------------------------------------------------------------------------
/**
 * \file    StaticModel.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the StaticModel class for loading and rendering static 3D models.
 */

#include <iostream>
#include "allObjects.h"

#pragma warning( disable : 4305 )

void StaticModel::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}

void StaticModel::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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

        unsigned int numIndices = geometry->numTriangles * 3;

        glBindVertexArray(geometry->vertexArrayObject);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    else {
        std::cerr << "Failed to draw model" << std::endl;
    }
}

StaticModel::StaticModel(ShaderProgram* shdrPrg, const std::string& modelPath)
    : ObjectInstance(shdrPrg), initialized(false)
{
    geometry = new ObjectGeometry;

    std::vector<float> loadedVertices;
    std::vector<unsigned int> loadedIndices;

    if (!loadModelWithAssimp(modelPath, loadedVertices, loadedIndices)) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return;
    }

    geometry->numTriangles = loadedIndices.size() / 3;
    geometry->numVertices = loadedVertices.size() / 8;
    geometry->numAtrributesPerVertex = 8;
    geometry->elementBufferObject = 0;

    geometry->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    geometry->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    geometry->specular = glm::vec3(0.0f, 0.0f, 0.0f);
    geometry->shininess = 10.0f;

    GLsizei stride = 8 * sizeof(float);

    glGenVertexArrays(1, &geometry->vertexArrayObject);
    glBindVertexArray(geometry->vertexArrayObject);

    glGenBuffers(1, &geometry->vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, loadedVertices.size() * sizeof(float), loadedVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &geometry->elementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, loadedIndices.size() * sizeof(unsigned int), loadedIndices.data(), GL_STATIC_DRAW);


    if ((shaderProgram != nullptr) && shaderProgram->initialized) {
        glEnableVertexAttribArray(shaderProgram->locations.position);
        glVertexAttribPointer(shaderProgram->locations.position, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        if (shaderProgram->locations.normal != -1) {
            glEnableVertexAttribArray(shaderProgram->locations.normal);
            glVertexAttribPointer(shaderProgram->locations.normal, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        }

        if (shaderProgram->locations.texCoord != -1) {
            glEnableVertexAttribArray(shaderProgram->locations.texCoord);
            glVertexAttribPointer(shaderProgram->locations.texCoord, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        }

        initialized = true;
    }

    glBindVertexArray(0);
}

StaticModel::~StaticModel() {
    glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
    glDeleteBuffers(1, &(geometry->elementBufferObject));
    glDeleteBuffers(1, &(geometry->vertexBufferObject));

    delete geometry;
    geometry = nullptr;

    initialized = false;
}