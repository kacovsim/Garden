//----------------------------------------------------------------------------------------
/**
 * \file    Bench.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Bench class.
 */

#include <iostream>
#include "allObjects.h"


Bench::Bench(ShaderProgram* shdrPrg)
    : StaticModel(shdrPrg, "models/bench.obj")
{
}


void Bench::update(float elapsedTime, const glm::mat4* parentModelMatrix) {

    localModelMatrix = glm::translate(glm::mat4(1.0f), position);
    localModelMatrix = glm::rotate(localModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

    ObjectInstance::update(elapsedTime, parentModelMatrix);
}