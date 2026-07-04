//----------------------------------------------------------------------------------------
/**
 * \file    MediumBush.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the MediumBush class.
 */

#include "allObjects.h"

MediumBush::MediumBush(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/bush01.obj")
{}

void MediumBush::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::translate(glm::mat4(1.0f), position);
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}