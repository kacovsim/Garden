#include <iostream>
#include "singlemesh.h"


void SingleMesh::update(float elapsedTime, const glm::mat4* parentModelMatrix) {
	// instance specific stuff

	// propagate the update to children
	ObjectInstance::update(elapsedTime, parentModelMatrix);
}

void SingleMesh::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	if (initialized && (shaderProgram != nullptr)) {
		glUseProgram(shaderProgram->program);

		glUniformMatrix4fv(shaderProgram->locations.PVMmatrix, 1, GL_FALSE, glm::value_ptr(globalModelMatrix));

		glBindVertexArray(geometry->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, geometry->numTriangles * 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	else {
		std::cerr << "SingleMesh::draw(): Can't draw, mesh not initialized properly!" << std::endl;
	}
}


/** Load one mesh using assimp library (vertices only, for more attributes see method extended version in Asteroids)
 * \param fileName [in] file to open/load
 * \param shader [in] vao will connect loaded data to shader
 * \param geometry
 */
bool SingleMesh::loadSingleMesh(const std::string& fileName, ShaderProgram* shader, ObjectGeometry** geometry) {
	Assimp::Importer importer;

	// unitize object in size (scale the model to fit into (-1..1)^3)
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	// load asset from the file - you can play with various processing steps
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate             // triangulate polygons (if any)
		| aiProcess_PreTransformVertices    // transforms scene hierarchy into one root with geometry-leafs only, for more see Doc
		| aiProcess_GenSmoothNormals        // calculate normals per vertex
		| aiProcess_JoinIdenticalVertices);

	// abort if the loader fails
	if (scn == NULL) {
		std::cerr << "SingleMesh::loadSingleMesh(): assimp error - " << importer.GetErrorString() << std::endl;
		*geometry = NULL;
		return false;
	}

	// some formats store whole scene (multiple meshes and materials, lights, cameras, ...) in one file, we cannot handle that in our simplified example
	if (scn->mNumMeshes != 1) {
		std::cerr << "SingleMesh::loadSingleMesh(): this simplified loader can only process files with only one mesh" << std::endl;
		*geometry = NULL;
		return false;
	}

	// in this phase we know we have one mesh in our loaded scene, we can directly copy its data to OpenGL ...
	const aiMesh* mesh = scn->mMeshes[0];

	*geometry = new ObjectGeometry;

	// vertex buffer object, store all vertex positions
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW);     // allocate memory for vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices); // store all vertices

	// copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
	unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
	}

	// copy our temporary index array to OpenGL and free the array
	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

	delete[] indices;

	// copy the material info to MeshGeometry structure
	const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
	aiColor4D color;
	aiString name;
	aiReturn retValue = AI_SUCCESS;

	// get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
	mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing, must be aiString type!

	if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS)
		color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject); // bind our element array buffer (indices) to vao
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);

	bool validInit = false;

	if ((shaderProgram != nullptr) && shaderProgram->initialized && (shaderProgram->locations.position != -1)) {

		glEnableVertexAttribArray(shader->locations.position);
		glVertexAttribPointer(shader->locations.position, 3, GL_FLOAT, GL_FALSE, 0, 0);

		CHECK_GL_ERROR();

		validInit = true;
	}

	glBindVertexArray(0);

	(*geometry)->numTriangles = mesh->mNumFaces;

	return validInit;
}


SingleMesh::SingleMesh(ShaderProgram* shdrPrg) : ObjectInstance(shdrPrg), initialized(false)
{
	const char* MODEL_FILE_NAME = "data/shape.obj";

	if (!loadSingleMesh(MODEL_FILE_NAME, shdrPrg, &geometry)) {
		if (geometry == nullptr) {
			std::cerr << "SingleMesh::SingleMesh(): geometry not initialized!" << std::endl;
		}
		else {
			std::cerr << "SingleMesh::SingleMesh(): shaderProgram struct not initialized!" << std::endl;
		}
	}
	else {
		if ((shaderProgram != nullptr) && shaderProgram->initialized && (shaderProgram->locations.PVMmatrix != -1)) {
			initialized = true;
		}
		else {
			std::cerr << "SingleMesh::SingleMesh(): shaderProgram struct not initialized!" << std::endl;
		}
	}
}

SingleMesh::~SingleMesh() {

	if (geometry != nullptr) {
		glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
		glDeleteBuffers(1, &(geometry->elementBufferObject));
		glDeleteBuffers(1, &(geometry->vertexBufferObject));

		delete geometry;
		geometry = nullptr;
	}

	initialized = false;
}
