//----------------------------------------------------------------------------------------
/**
 * \file    InstancedModel.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the InstancedModel class for loading and rendering instanced 3D models.
 */

#include "allObjects.h"
#include <iostream>

InstancedModel::InstancedModel(ShaderProgram* shdrPrg, const std::string& modelPath, float scale)
    : ObjectInstance(shdrPrg), initialized(false), instanceCount(0), instanceVBO(0), defaultScale(scale)
{
    geometry = new ObjectGeometry;

    std::vector<float> loadedVertices;
    std::vector<unsigned int> loadedIndices;

    if (!loadModelWithAssimp(modelPath, loadedVertices, loadedIndices)) {
        std::cerr << "Failed to load the instanced model: " << modelPath << std::endl;
        return;
    }

    geometry->numTriangles = loadedIndices.size() / 3;
    geometry->numIndices = loadedIndices.size();
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
    }

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), modelMatrices.data(), GL_DYNAMIC_DRAW);

    if (shaderProgram != nullptr) {
        GLint instanceLoc = shaderProgram->locations.instanceMatrix;

        if (instanceLoc != -1) {
            std::size_t vec4Size = sizeof(glm::vec4);
            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(instanceLoc + i);
                glVertexAttribPointer(instanceLoc + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
                glVertexAttribDivisor(instanceLoc + i, 1);
            }
        }
    }

    glBindVertexArray(0);
    initialized = true;
}

InstancedModel::~InstancedModel() {
    glDeleteBuffers(1, &instanceVBO);
    if (geometry != nullptr) {
        delete geometry;
        geometry = nullptr;
    }
    initialized = false;
}

void InstancedModel::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (initialized && shaderProgram != nullptr) {
        glUseProgram(shaderProgram->program);

        glUniformMatrix4fv(shaderProgram->locations.Vmatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(shaderProgram->locations.Pmatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        setMaterialUniforms(shaderProgram, geometry->ambient, geometry->diffuse, geometry->specular, geometry->shininess, textureID);

        glBindVertexArray(geometry->vertexArrayObject);
        glDrawElementsInstanced(GL_TRIANGLES, geometry->numIndices, GL_UNSIGNED_INT, 0, instanceCount);
        glBindVertexArray(0);
    }
    else {
        std::cerr << "InstancedModel::draw() failed - model not initialized." << std::endl;
    }
}

void InstancedModel::addInstance(const glm::vec3& position) {
    glm::mat4 model = glm::mat4(1.0f);
    float zRot = static_cast<float>(rand() % 360);

    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(zRot), glm::vec3(0.0f, 1.0f, 0.0f));

    model = glm::scale(model, glm::vec3(defaultScale));

    modelMatrices.push_back(model);
    instanceCount = modelMatrices.size();

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), modelMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}