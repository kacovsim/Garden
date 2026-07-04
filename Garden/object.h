#pragma once

//----------------------------------------------------------------------------------------
/**
 * \file    object.h
 * \author  Jaroslav Sloup, Petr Felkel (Original Skeleton), Modified by Simona Kácová
 * \date    2026/05/15
 * \brief   Core data structures and base classes for the 3D scene graph and object representation.
 */

#include <vector> 
#include <string>
#include "pgr.h"

/**
 * @brief Custom object loader from an .obj file
 * @param path to the .obj file
 * @param vector for storing verticies
 * @param vector for storing UVs
 * @param vector for storing normals
 */
bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
);

bool loadModelWithAssimp(
	const std::string& filepath,
	std::vector<float>& outVertices,
	std::vector<unsigned int>& outIndices
);

/**
 * \brief Shader program related stuff (id, locations, ...).
 */
typedef struct _ShaderProgram {
	GLuint program;
	GLint PVMmatrixLocation;
	bool initialized;

	/**
	  * \brief Indices of the vertex shader inputs (locations)
	  */
	struct {
		GLint position;
		GLint normal;
		GLint texCoord;
		GLint Vmatrix;
		GLint Mmatrix;
		GLint Pmatrix;
		GLint normalMatrix;
		GLint PVMmatrix;
		GLint texSampler;
		GLint diffuseLocation;    
		GLint ambientLocation;   
		GLint specularLocation;   
		GLint shininessLocation;
		GLint useTextureLocation; 
		GLint texSamplerLocation; 
		GLint texScaleLocation;
		GLint timeLocation;
		GLint instanceMatrix;
		GLint shadowTintLocation;
		GLint sunSpeedLocation;
		GLint inversePVmatrix;
		GLint lampAmbientLocation;
		GLint lampDiffuseLocation;
		GLint lampSpecularLocation;
		GLint lampPositionLocation;
		GLint drawWireframeLocation;
		GLint enableFogLocation;
		GLint fogColorLocation;
		GLint fogDensityLocation;
		GLint fogTextureLocation;
		GLint flashlightEnabledLocation;
		GLint flashlightAmbientLocation;
		GLint flashlightDiffuseLocation;
		GLint flashlightSpecularLocation;
		GLint flashlightPositionLocation;
		GLint flashlightDirectionLocation;
		GLint flashlightCutOffLocation;
		GLint flashlightExponentLocation;
		GLint texOffsetLocation;
		GLint texSpriteSizeLocation;
		GLint isAnimatedLocation;
	} locations;

	_ShaderProgram() : program(0), initialized(false) {
		locations.position = -1;
		locations.normal = -1;
		locations.texCoord = -1;
		locations.PVMmatrix = -1;
		locations.texSampler = -1;
		locations.Vmatrix = -1;
		locations.Mmatrix = -1;
		locations.Pmatrix = -1;
		locations.normalMatrix = -1;
		locations.diffuseLocation = -1;
		locations.ambientLocation = -1;
		locations.specularLocation = -1;
		locations.shininessLocation = -1;
		locations.useTextureLocation = -1;
		locations.texSamplerLocation = -1;
		locations.timeLocation = -1;
		locations.texScaleLocation = -1;
		locations.instanceMatrix = -1;
		locations.shadowTintLocation = -1;
		locations.sunSpeedLocation = -1;
		locations.inversePVmatrix = -1;
		locations.enableFogLocation = -1;
		locations.fogColorLocation = -1;
		locations.fogDensityLocation = -1;
		locations.flashlightEnabledLocation = -1;
		locations.flashlightAmbientLocation = -1;
		locations.flashlightDiffuseLocation = -1;
		locations.flashlightSpecularLocation = -1;
		locations.flashlightPositionLocation = -1;
		locations.flashlightDirectionLocation = -1;
		locations.flashlightCutOffLocation = -1;
		locations.flashlightExponentLocation = -1;
		locations.fogTextureLocation = -1;
		locations.texOffsetLocation = -1;
		locations.texSpriteSizeLocation = -1;
		locations.isAnimatedLocation = -1;
	}

} ShaderProgram;

/**
 * \brief Geometry of an object (vertices, triangles).
 */
typedef struct _ObjectGeometry {
	GLuint        vertexBufferObject; 
	GLuint        elementBufferObject; 
	GLuint        vertexArrayObject;  
	unsigned int  numTriangles; 
	unsigned int  numVertices;
	unsigned int  numAtrributesPerVertex;
	unsigned int  numIndices;

	glm::vec3     ambient;
	glm::vec3     diffuse;
	glm::vec3     specular;
	float         shininess;
	GLuint        texture;

} ObjectGeometry;

class ObjectInstance;
/**
 * \brief Linear representation of the scene objects.  The objects themselves may represent the subtrees.
 */
typedef std::vector<ObjectInstance*> ObjectList;  


class ObjectInstance {

protected:


public:
	ObjectGeometry* geometry;
	GLuint          textureID;
	glm::mat4		localModelMatrix;
	glm::mat4		globalModelMatrix;
	glm::vec3 position;
	glm::vec3 direction;
	float rotation;
	float     speed;
	float viewAngle;
	float     size;

	ShaderProgram* shaderProgram;

	ObjectList children;
	/**
	 * \brief ObjectInstance constructor. Takes a pointer to the shader and must create object resources (VBO and VAO)
	 * \param shdrPrg pointer to the shader program for rendering objects data
	 */
	ObjectInstance(ShaderProgram* shdrPrg = nullptr) :
		geometry(nullptr),
		textureID(0),
		localModelMatrix(1.0f), 
		globalModelMatrix(1.0f),  
		shaderProgram(shdrPrg) {
	}
	~ObjectInstance() {}
  
	/**
	* \brief Recalculates the global matrix and updates all children.
	*   Derived classes should also call this method (using ObjectInstance::update()).
	* \param elapsedTime time value in seconds, such as 0.001*glutGet(GLUT_ELAPSED_TIME) (conversion milliseconds => seconds)
	* \param parentModelMatrix parent transformation in the scene-graph subtree
	*/
	virtual void update(const float elapsedTime, const glm::mat4* parentModelMatrix) {

		if (parentModelMatrix != nullptr)
			globalModelMatrix = *parentModelMatrix * localModelMatrix;
		else
			globalModelMatrix = localModelMatrix;

		for (ObjectInstance* child : children) {
			if (child != nullptr)
				child->update(elapsedTime, &globalModelMatrix);
		}
	}

	/**
	 * \brief Draw instance geometry and calls the draw() on child nodes.
	 */
	virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

		for (ObjectInstance* child : children) { 
			if (child != nullptr)
				child->draw(viewMatrix, projectionMatrix);
		}
	}

};
