//----------------------------------------------------------------------------------------
/**
 * \file    Bird.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Bird class.
 */


#include <iostream>
#include "allObjects.h"

Bird::Bird(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/bird.obj")
{
}


void Bird::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    float radius = 2.5f; 
    float speed = 1.0f;
    float height = 2.0f;

    float offsetX = radius * cos(elapsedTime * speed);
    float offsetY = radius * sin(elapsedTime * speed);

    float facingAngle = (elapsedTime * speed) + glm::radians(90.0f);

    glm::vec3 centerPoint = position;
    centerPoint.z = height; 

    localModelMatrix = glm::translate(glm::mat4(1.0f), centerPoint + glm::vec3(offsetX, offsetY, 0.0f));
    localModelMatrix = glm::rotate(localModelMatrix, facingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}
