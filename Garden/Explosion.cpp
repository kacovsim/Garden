//----------------------------------------------------------------------------------------
/**
 * \file    Explosion.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Explosion class.
 */


#include <iostream>
#include "allObjects.h"

extern glm::mat4 currentViewMatrix;

Explosion::Explosion(ShaderProgram* shdrPrg) : StaticModel(shdrPrg, "models/quad.obj") {
    age = -1.0f;
    maxLifetime = 0.5f;
    isDead = false;
}

void Explosion::update(float elapsedTime, const glm::mat4* parentModelMatrix) {
    if (age < 0.0f) {
        age = elapsedTime;
    }

    float currentLifetime = elapsedTime - age;

    float lifePercentage = currentLifetime / maxLifetime;
    int totalFrames = numCols * numRows;

    currentFrame = (int)(lifePercentage * totalFrames);


    if (currentFrame >= totalFrames) {
        currentFrame = totalFrames - 1;
    }

    if (currentLifetime >= maxLifetime) {
        isDead = true;
    }

    localModelMatrix = glm::translate(glm::mat4(1.0f), position);


    glm::mat4 viewRotation = glm::mat4(glm::mat3(currentViewMatrix));
    glm::mat4 inverseViewRotation = glm::inverse(viewRotation);
    localModelMatrix = localModelMatrix * inverseViewRotation;

    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}

void Explosion::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    float frameWidth = 1.0f / numCols;
    float frameHeight = 1.0f / numRows;

    int col = currentFrame % numCols;
    int row = currentFrame / numCols;

    row = (numRows - 1) - row;

    glUseProgram(shaderProgram->program);
    glUniform2f(shaderProgram->locations.texOffsetLocation, col * frameWidth, row * frameHeight);
    glUniform2f(shaderProgram->locations.texSpriteSizeLocation, frameWidth, frameHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    StaticModel::draw(viewMatrix, projectionMatrix);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glUniform2f(shaderProgram->locations.texOffsetLocation, 0.0f, 0.0f);
    glUniform2f(shaderProgram->locations.texSpriteSizeLocation, 1.0f, 1.0f);
}