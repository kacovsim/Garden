//----------------------------------------------------------------------------------------
/**
 * \file    field.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Implementation of the Field class.
 */


#include <iostream>
#include "allObjects.h"




void Field::update(float elapsedTime, const glm::mat4* parentModelMatrix) {
	localModelMatrix = glm::translate(glm::mat4(1.0f), position);
	localModelMatrix = glm::scale(localModelMatrix, glm::vec3(size, size, size));

	ObjectInstance::update(elapsedTime, parentModelMatrix);
}

void Field::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	if(initialized && (shaderProgram != nullptr)) {
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
		glUniform1f(shaderProgram->locations.texScaleLocation, 2.0f);

		unsigned int numIndices = geometry->numTriangles * 3;

		glBindVertexArray(geometry->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	else {
		std::cerr << "Field can't draw" << std::endl;
	}
}

Field::Field(ShaderProgram* shdrPrg) : ObjectInstance(shdrPrg), initialized(false)
{
	geometry = new ObjectGeometry;

	geometry->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	geometry->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	geometry->specular = glm::vec3(0.0f, 0.0f, 0.0f);
	geometry->shininess = 10.0f;

	GLsizei stride = 8 * sizeof(float);

	static constexpr float vertices[] = {
	  -1.000000,-1.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,
	  1.000000,-1.000000,0.000000,0.000000,0.000000,1.000000,1.000000,0.000000,
	  1.000000,1.000000,0.000000,0.000000,0.000000,1.000000,1.000000,1.000000,
	  -1.000000,1.000000,0.000000,-0.000000,0.000000,1.000000,0.000000,1.000000,
	};

	static constexpr unsigned int indices[] = {
	  0, 1, 2,
	  0, 2, 3,
	};

	geometry->numTriangles = 2;
	geometry->numVertices = 4;
	geometry->numAtrributesPerVertex = 4;
	geometry->elementBufferObject = 0;

	glGenVertexArrays(1, &geometry->vertexArrayObject);
	glBindVertexArray(geometry->vertexArrayObject);

	glGenBuffers(1, &geometry->vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &geometry->elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	if ((shaderProgram != nullptr) && shaderProgram->initialized && (shaderProgram->locations.position != -1) && (shaderProgram->locations.PVMmatrix != -1)) {
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
	else {
		std::cerr << "Field not initilized" << std::endl;
	}
	glBindVertexArray(0);
}

Field::~Field() {
	glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
	glDeleteBuffers(1, &(geometry->elementBufferObject));
	glDeleteBuffers(1, &(geometry->vertexBufferObject));

	delete geometry;
	geometry = nullptr;

	initialized = false;
}
