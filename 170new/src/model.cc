//
// Created by phatt on 1/29/25.
//

module;

#include "std.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/vector3.h>

export module model;

import mesh;
import vertex_buffer.vertex_struct;
import texture;
import camera;
import shader_program;
import transformation;

export class AssimpGlmHelper {
public:
    static auto printMat4(const glm::mat4& m, int depth = 0) -> void {
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("-------------------------------------------\n");
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("%f %f %f %f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("%f %f %f %f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("%f %f %f %f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("%f %f %f %f\n", m[3][0], m[3][1], m[3][2], m[3][3]);
        for (int i = 0; i < depth; i++) std::cout << "\t";
        std::printf("-------------------------------------------\n");
    }

    static auto convertMatrix(const aiMatrix4x4 &aiMat) -> glm::mat4 {
        return (glm::mat4) {
            aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
            aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
            aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
            aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4 
        };
    }

    static auto printMat4(const aiMatrix4x4& mat) -> void {
        std::printf("-----------------------------------------\n");
        std::printf("%f %f %f %f\n", mat.a1, mat.a2, mat.a3, mat.a4);
        std::printf("%f %f %f %f\n", mat.b1, mat.b2, mat.b3, mat.b4);
        std::printf("%f %f %f %f\n", mat.c1, mat.c2, mat.c3, mat.c4);
        std::printf("%f %f %f %f\n", mat.d1, mat.d2, mat.d3, mat.d4);
        std::printf("-----------------------------------------\n");
    }   

    /// Convert Assimp matrix to GLM matrix
    static glm::mat4 convertMatrixToGLM(const aiMatrix4x4& from) {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }
};

export class Model {
private:
    std::vector<Mesh> meshes;
    std::string basePath;
    std::string filePath;

    // Cache for loading in the textures. The textures classes are copied
    // but not the data which is good because classes are cheap but loading
    // and duplicating data is expensive.
    std::unordered_map<std::string, Texture> loadedTexturesCache;
public:
    /// Creates a model by loading in a model from a file.
    explicit Model(const std::string& filePath)
    : basePath(filePath.substr(0, filePath.find_last_of('/') + 1))
    , filePath(filePath) {
        loadInModel(filePath);
    }

    ~Model() = default;

    /// Deletes the meshes explicitly.
    auto deleteResource() -> void {
        for (auto& mesh : meshes) {
            mesh.deleteResource();
        }
    }

    /// Draws the model with specified shader with respect to the camera's POV
    /// and the model's scale, rotation and translation vectors.
    auto draw(
        ShaderProgram& shader, 
        const Camera& camera,
        const Transformation& transformation
    ) -> void {
        for (auto& mesh : meshes) {
            mesh.draw(shader, camera, transformation);
        }
    }
private:
    auto loadInModel(const std::string& path) -> void {
        std::println("Loading in model: {}", path);

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

        if (scene == nullptr
        || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
        || scene->mRootNode == nullptr) {
            throw std::runtime_error(std::format(
                "Could not load in model: {}", importer.GetErrorString()
            ));
        }
        
        glm::mat4 rootTransform = AssimpGlmHelper::convertMatrixToGLM(scene->mRootNode->mTransformation);
        // std::cout << "Root transformation matrix.\n";
        // AssimpGlmHelper::printMat4(rootTransform);
        traverseNode(scene->mRootNode, scene, glm::mat4(1.0f), 0);
    }

    auto traverseNode(
             const aiNode *node, 
             const aiScene *scene, 
             const glm::mat4& parentTransform, 
             const int depth
    ) -> void {
        // TODO: Add transformations relative to parent node to the mesh class.
        // node->mTransformation;

        // Converted from ASSIMP format to GLM format.
        const glm::mat4 localTransform = AssimpGlmHelper::convertMatrixToGLM(node->mTransformation);
        // The parent's computed transformation is 
        // multiplied with local transformation.
        const glm::mat4 computedTransform = parentTransform * localTransform; 

        // std::cout << "Local transformation matrix.\n";
        // AssimpGlmHelper::printMat4(localTransform, depth);

        // std::cout << "Parent computed transformation matrix.\n";
        // AssimpGlmHelper::printMat4(parentTransform, depth);

        // std::cout << "Computed with the parent computed matrix.\n";
        // AssimpGlmHelper::printMat4(computedTransform, depth);

        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene, computedTransform));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            traverseNode(node->mChildren[i], scene, computedTransform, depth + 1);
        }
    }

    auto processMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& transform) -> Mesh {
        std::vector<Vertex> vertices;
        
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            const aiVector3D p = mesh->mVertices[i];
            Vertex v = { .position = { p.x, p.y, p.z } };  

            if (mesh->HasNormals()) {
                const aiVector3D n = mesh->mNormals[i];
                v.normal = { n.x, n.y, n.z };
            }

            if (mesh->mTextureCoords[0] != nullptr) {
                const aiVector3D uv = mesh->mTextureCoords[0][i];
                v.texUV = { uv.x, uv.y };
                
                if (mesh->HasTangentsAndBitangents()) {
                    const aiVector3D t = mesh->mTangents[i];
                    const aiVector3D bt = mesh->mBitangents[i];

                    v.tangent = { t.x, t.y, t.z };
                    v.bitangent = { bt.x, bt.y, bt.z };
                }
            } else {
                v.texUV = { 0.f, 0.f };
            }

            vertices.push_back(v);
        }


        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace f = mesh->mFaces[i];
            for (unsigned int j = 0; j < f.mNumIndices; j++) {
                indices.push_back(f.mIndices[j]);
            }
        }


        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

        std::vector<Texture> diffuseMaps = getMaterialTextures(material, aiTextureType_DIFFUSE);
        std::vector<Texture> specularMaps = getMaterialTextures(material, aiTextureType_SPECULAR);
        std::vector<Texture> metalnessMaps = getMaterialTextures(material, aiTextureType_METALNESS);

        std::vector<Texture> textures;
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

        
        return Mesh(vertices, indices, textures, transform);
    }

    auto getMaterialTextures(
        const aiMaterial* material,
        const aiTextureType aiTextureType
    ) -> std::vector<Texture> {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType); i++) {
            aiString fileName;
            material->GetTexture(aiTextureType, i, &fileName);
            
            const std::string path = basePath + std::string(fileName.C_Str());

            if (loadedTexturesCache.contains(path)) {
                auto loadedTexture = loadedTexturesCache[path];
                textures.push_back(loadedTexture);
                continue;
            }

            std::cout << "Texture file path: " << path << "\n";

            const texture::Type textureType = [&] {
                switch (aiTextureType) {
                    case aiTextureType_DIFFUSE: { return texture::Type::DiffuseMap; }
                    case aiTextureType_SPECULAR: { return texture::Type::SpecularMap; }
                    case aiTextureType_METALNESS: { return texture::Type::SpecularMap; }
                    default: throw std::runtime_error("Unsupported assimp texture type.");
                }
            }();

            const auto texture = Texture(path, texture::Dimension::$2D, textureType);
            loadedTexturesCache[path] = texture;
            textures.push_back(texture);
        }

        return textures;
    }

};

