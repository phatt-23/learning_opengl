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

        traverseNode(scene->mRootNode, scene);
    }

    auto traverseNode(const aiNode *node, const aiScene *scene) -> void {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            traverseNode(node->mChildren[i], scene);
        }
    }

    auto processMesh(const aiMesh* mesh, const aiScene* scene) -> Mesh {
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

        return Mesh(vertices, indices, textures);
    }

    auto getMaterialTextures(
        const aiMaterial* material,
        const aiTextureType aiTextureType
    ) -> std::vector<Texture> {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType); i++) {
            aiString fileName;
            material->GetTexture(
                aiTextureType,
                i,
                &fileName
            );
            
            const std::string path = basePath + std::string(fileName.C_Str());

            std::cout << "Texture file path: " << path << "\n";
            
            if (loadedTexturesCache.contains(path)) {
                auto loadedTexture = loadedTexturesCache[path];
                textures.push_back(loadedTexture);
                continue;
            }


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

