//----------------------------------------------------------------------------------------
/**
 * \file    ObjLoader.cpp
 * \author  Simona Kácová
 * \date    2026/05/15
 * \brief   Objects loaders freom .obj file, custom and using assimp
 */


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "pgr.h"
#include "object.h"


 /**
  * \brief Custom loader for parsing basic .obj files.
  *
  * \details Reads vertex, texture coordinate, normal, and face data from a standard .obj file
  * and outputs them into distinct vectors.
  *
  * \param path Path to the .obj file to load.
  * \param out_vertices Output vector to store the parsed vertex positions.
  * \param out_uvs Output vector to store the parsed texture coordinates.
  * \param out_normals Output vector to store the parsed normal vectors.
  * \return True if the file was successfully loaded and parsed, false otherwise.
  */
bool loadOBJ(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals
) {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE* file;

    errno_t err = fopen_s(&file, path, "r");
    if (err != 0 || file == NULL) {
        std::cerr << "You put wrong file path into the custom obj loader, you loser. Shame on you and your ancestors." << std::endl;
        return false;
    }

    while (1) {
        char lineHeader[128];

        int res = fscanf_s(file, "%s", lineHeader, (unsigned int)sizeof(lineHeader));
        if (res == EOF)
            break; 

        // parse vertices
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        // parse texture coordinates
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        // parse normals
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        // parse faces
        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];


            int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

            if (matches != 9) {
                std::cerr << "Not an .obj file" << std::endl;
                fclose(file);
                return false;
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }


    for (unsigned int i = 0; i < vertexIndices.size(); i++) {

        unsigned int vertexIndex = vertexIndices[i] - 1;
        unsigned int uvIndex = uvIndices[i] - 1;
        unsigned int normalIndex = normalIndices[i] - 1;

        glm::vec3 vertex = temp_vertices[vertexIndex];
        glm::vec2 uv = temp_uvs[uvIndex];
        glm::vec3 normal = temp_normals[normalIndex];

        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }

    fclose(file);
    return true;
}

/**
 * \brief Loads a 3D model utilizing the Open Asset Import Library (Assimp).
 *
 * \details Imports the model file, extracts its first mesh, and packs the vertices,
 * normals, and texture coordinates into a single interleaved float vector. It also extracts
 * the corresponding face indices for indexed rendering.
 *
 * \param filepath Path to the 3D model file.
 * \param outVertices Output vector containing interleaved vertex data (position, normal, UV).
 * \param outIndices Output vector containing the mesh face indices.
 * \return True if the model was successfully loaded and processed, false otherwise.
 */
bool loadModelWithAssimp(const std::string& filepath, std::vector<float>& outVertices, std::vector<unsigned int>& outIndices) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR ASIMP" << importer.GetErrorString() << std::endl;
        return false;
    }

    aiMesh* mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

        outVertices.push_back(mesh->mVertices[i].x);
        outVertices.push_back(mesh->mVertices[i].y);
        outVertices.push_back(mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            outVertices.push_back(mesh->mNormals[i].x);
            outVertices.push_back(mesh->mNormals[i].y);
            outVertices.push_back(mesh->mNormals[i].z);
        }
        else {
            outVertices.push_back(0.0f); outVertices.push_back(0.0f); outVertices.push_back(0.0f);
        }

        if (mesh->mTextureCoords[0]) {
            outVertices.push_back(mesh->mTextureCoords[0][i].x);
            outVertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else {
            outVertices.push_back(0.0f); outVertices.push_back(0.0f);
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            outIndices.push_back(face.mIndices[j]);
        }
    }

    return true;
}