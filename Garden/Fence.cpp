//----------------------------------------------------------------------------------------
/**
 * \file    Fence.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Fence class.
 */


#include <iostream>
#include "allObjects.h"

#pragma warning( disable : 4305 )

Fence::Fence(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/fencePiece.obj")
{}


void Fence::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::translate(glm::mat4(1.0f), position);
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}