#pragma once
//----------------------------------------------------------------------------------------
/**
 * \file    allObjects.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   File with all object class declarations
 */

#include "object.h"

void setTransformUniforms(ShaderProgram* shader, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

void setMaterialUniforms(ShaderProgram* shader, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess, GLuint texture);

class Field : public ObjectInstance
{
public:

	Field(ShaderProgram* shdrPrg = nullptr);
	~Field();

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
	void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

private:

	bool initialized; 
};

class StaticModel : public ObjectInstance {
public:
	StaticModel(ShaderProgram* shdrPrg, const std::string& modelPath);
	virtual ~StaticModel();

	virtual void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
	virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

protected:
	bool initialized;
};

class Tree00 : public StaticModel
{
public:

	Tree00(ShaderProgram* shdrPrg = nullptr);
	~Tree00() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;

};

class Tree01 : public StaticModel
{
public:

	Tree01(ShaderProgram* shdrPrg = nullptr);
	~Tree01() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class LargeBush : public StaticModel {
public:
	LargeBush(ShaderProgram* shdrPrg = nullptr);
	~LargeBush() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class MediumBush : public StaticModel {
public:
	MediumBush(ShaderProgram* shdrPrg = nullptr);
	~MediumBush() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Fence : public StaticModel {
public:
	Fence(ShaderProgram* shdrPrg = nullptr);
	~Fence() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Gate : public StaticModel {
public:
	Gate(ShaderProgram* shdrPrg = nullptr);
	~Gate() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Lantern : public StaticModel {
public:
	Lantern(ShaderProgram* shdrPrg = nullptr);
	~Lantern() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Flashlight : public StaticModel {
public:
	Flashlight(ShaderProgram* shdrPrg = nullptr);
	~Flashlight() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Bench : public StaticModel {
public:
	Bench(ShaderProgram* shdrPrg = nullptr);
	~Bench() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};

class Bird : public StaticModel {
public:
	Bird(ShaderProgram* shdrPrg = nullptr);
	~Bird() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
};


class Fairy : public ObjectInstance
{
public:

	Fairy(ShaderProgram* shdrPrg = nullptr);
	~Fairy();

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;
	void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

private:

	bool initialized;  
};


class InstancedModel : public ObjectInstance {
public:

	InstancedModel(ShaderProgram* shdrPrg, const std::string& modelPath, float scale);
	virtual ~InstancedModel();

	virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;
	virtual void addInstance(const glm::vec3& position);

protected:
	bool initialized;
	int instanceCount;
	GLuint instanceVBO;
	float defaultScale;
	std::vector<glm::mat4> modelMatrices;
};

class FlowerInstanced : public InstancedModel {
public:
	FlowerInstanced(ShaderProgram* shdrPrg = nullptr);
	~FlowerInstanced() = default;
};

class RocksInstanced : public InstancedModel {
public:
	RocksInstanced(ShaderProgram* shdrPrg = nullptr);
	~RocksInstanced() = default;
};

class GrassInstanced : public InstancedModel {
public:
	GrassInstanced(ShaderProgram* shdrPrg = nullptr);
	~GrassInstanced() = default;
};

class BellInstanced : public InstancedModel {
public:
	BellInstanced(ShaderProgram* shdrPrg = nullptr);
	~BellInstanced() = default;
};


class DaisyInstanced : public InstancedModel {
public:
	DaisyInstanced(ShaderProgram* shdrPrg = nullptr);
	~DaisyInstanced() = default;
};


class SkyBox : public ObjectInstance {
public:
	SkyBox(ShaderProgram* shdrPrg);
	~SkyBox();

	void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

private:
	bool initialized;
	GLuint vbo, vao;
};

class Explosion : public StaticModel {
public:
	Explosion(ShaderProgram* shdrPrg = nullptr);
	~Explosion() = default;

	void update(float elapsedTime, const glm::mat4* parentModelMatrix) override;

	void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	int currentFrame;
	int numCols = 7;
	int numRows = 1;

	float age;
	float maxLifetime;
	bool isDead;
};