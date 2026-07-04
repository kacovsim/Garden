//----------------------------------------------------------------------------------------
/**
 * \file    Flashlight.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Flashlight class.
 */


#include <iostream>
#include "allObjects.h"

Flashlight::Flashlight(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/flashlight.obj")
{
}


void Flashlight::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::mat4(1.0f);
    localModelMatrix = glm::translate(localModelMatrix, position);

    float angle = std::atan2(direction.y, direction.x);
    localModelMatrix = glm::rotate(localModelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 localOffset = glm::vec3(0.05f, -0.02f, 0.1f);
    localModelMatrix = glm::translate(localModelMatrix, localOffset);

    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}
