#pragma once

#include "object.h"

class Field : public ObjectInstance
{
public:

	Field(ShaderProgram* shdrPrg = nullptr);
	~Field();

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
	void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

private:

	bool initialized;  ///< object has the shader with defined locations
};

