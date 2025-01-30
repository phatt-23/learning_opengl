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
    std::set<std::string> loadedTexturesFilePaths;

public:
    /// Creates a model by loading in a model from a file.
    explicit Model(const std::string& filePath)
    : basePath(filePath.substr(0, filePath.find_last_of("/") + 1))
    , filePath(filePath) {
        loadInModel(filePath);
    }

    ~Model() = default;

    /// Deletes the meshes explicitely.
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
    auto loadInModel(const std::string& filePath) -> void {
        std::println("Loading in model: {}", filePath);

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate); // | ,iProcess_FlipUVs);
        
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
        // std::println("Traversing node: {} with {} meshes and {} children.", std::size_t(node), node->mNumMeshes, node->mNumChildren);
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // std::println("Processing mesh number {}", i);
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            // std::println("Got reference to the mesh.");
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            // std::println("From node {} Traversing child node {}", std::size_t(node), std::size_t(node->mChildren[i]));
            traverseNode(node->mChildren[i], scene);
        }
    }

    auto processMesh(const aiMesh* mesh, const aiScene* scene) -> Mesh {
        std::vector<Vertex> vertices;
        
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // std::println("Getting position vector");
            const aiVector3D p = mesh->mVertices[i];
            Vertex v = { .position = { p.x, p.y, p.z } };  

            if (mesh->HasNormals()) {
                // std::println("Getting normal vector");
                const aiVector3D n = mesh->mNormals[i];
                v.normal = { n.x, n.y, n.z };
            }

            if (mesh->mTextureCoords[0] != nullptr) {
                // std::println("Getting texture OV coordinates");
                const aiVector3D uv = mesh->mTextureCoords[0][i];
                v.texUV = { uv.x, uv.y };
                
                if (mesh->HasTangentsAndBitangents()) {
                    // std::println("Getting tangent and bitangent vectors");
                    const aiVector3D t = mesh->mTangents[i];
                    const aiVector3D bt = mesh->mBitangents[i];

                    v.tangent = { t.x, t.y, t.z };
                    v.bitangent = { bt.x, bt.y, bt.z };
                }
            } else {
                // std::println("No texture UVs. Setting to zero");
                v.texUV = { 0.f, 0.f };
            }

            vertices.push_back(v);
        }

        // std::println("Getting indices");
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            // std::println("Getting face number {}", i);
            aiFace f = mesh->mFaces[i];
            for (unsigned int j = 0; j < f.mNumIndices; j++) {
                // std::println("From face {} Getting index {}", i, j);
                indices.push_back(f.mIndices[j]);
            }
        }

        // std::println("Getting material at index {}", mesh->mMaterialIndex);
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);

        // std::println("Getting matrial textures for diffuse maps.");
        std::vector<Texture> diffuseMaps = getMaterialTextures(
            material, 
            aiTextureType_DIFFUSE
        );

        // std::println("Getting matrial textures for specular maps.");
        std::vector<Texture> specularMaps = getMaterialTextures(
            material, 
            aiTextureType_SPECULAR
        );

        // std::println("Merging textures to one vector.");
        std::vector<Texture> textures;
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // std::println("Returning processed mesh.");
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
            
            const std::string filePath = basePath + std::string(fileName.C_Str());
            
            if (loadedTexturesFilePaths.contains(filePath)) {
                continue;
            }

            loadedTexturesFilePaths.insert(filePath);

            const texture::Type textureType = [&] {
                switch (aiTextureType) {
                    case aiTextureType_DIFFUSE: return texture::Type::DiffuseMap;
                    case aiTextureType_SPECULAR: return texture::Type::SpecularMap;
                    default: throw std::runtime_error("Unsupported assimp texture type.");
                }
            }();

            const auto texture = Texture(filePath, texture::Dimension::$2D, textureType);
            textures.push_back(texture);
        }

        return textures;
    }

};

