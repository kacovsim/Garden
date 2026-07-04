//----------------------------------------------------------------------------------------
/**
 * \file    SkyBox.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the SkyBox class for rendering a cubemap background environment.
 */

#include <iostream>
#include "allObjects.h"

void SkyBox::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glUseProgram(shaderProgram->program);

    glm::mat4 viewRotation = viewMatrix;
    viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 inversePVmatrix = glm::inverse(projectionMatrix * viewRotation);

    glUniformMatrix4fv(shaderProgram->locations.inversePVmatrix, 1, GL_FALSE, glm::value_ptr(inversePVmatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glUniform1i(shaderProgram->locations.texSampler, 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

SkyBox::SkyBox(ShaderProgram* shdrPrg) : initialized(false) {
    this->shaderProgram = shdrPrg;
    vbo = 0;
    vao = 0;
    textureID = 0;

    static const float screenCoords[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    GLuint targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    const char* suffixes[] = { "px", "nx", "ny", "py", "pz", "nz" };
    for (int i = 0; i < 6; i++) {
        std::string texName = "textures/" + std::string(suffixes[i]) + ".png";
        pgr::loadTexImage2D(texName, targets[i]);
    }

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenCoords), screenCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(shdrPrg->locations.position);
    glVertexAttribPointer(shdrPrg->locations.position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    initialized = true;
}

SkyBox::~SkyBox() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}