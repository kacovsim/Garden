//----------------------------------------------------------------------------------------
/**
 * \file    Lantern.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Lantern class.
 */


#include <iostream>
#include "allObjects.h"

Lantern::Lantern(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/lamp.obj")
{
    geometry->ambient = glm::vec3(0.7f, 0.7f, 0.7f);
}

void Lantern::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::translate(glm::mat4(1.0f), position);
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}
