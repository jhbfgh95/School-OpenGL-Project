#pragma once

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stbi/stb_image.h>

#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <fstream>

#include "Model/mesh.h"
#include "ourshader/Shader.h"

class Model
{
public:
    std::vector<Mesh> meshes;
    std::string directory;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    Model(const std::string& path)
    {
        minBounds = glm::vec3(std::numeric_limits<float>::max());
        maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
        loadModel(path);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader);
        }
    }

private:
    std::vector<Texture> textures_loaded;

    void loadModel(const std::string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
            return;
        }

        directory = path.substr(0, path.find_last_of("/\\"));

        std::cout << "Model loaded: " << path << std::endl;
        std::cout << "Mesh count: " << scene->mNumMeshes << std::endl;
        std::cout << "Material count: " << scene->mNumMaterials << std::endl;

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        std::cout << "Node: " << node->mName.C_Str()
                  << " / Mesh count: " << node->mNumMeshes << std::endl;

        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        std::cout << "Processing Mesh: " << mesh->mName.C_Str() << std::endl;
        std::cout << "Vertices: " << mesh->mNumVertices << std::endl;
        std::cout << "Faces: " << mesh->mNumFaces << std::endl;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            vertex.Position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );
            minBounds = glm::min(minBounds, vertex.Position);
            maxBounds = glm::max(maxBounds, vertex.Position);

            if (mesh->HasNormals())
            {
                vertex.Normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                );
            }
            else
            {
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            std::vector<Texture> diffuseMaps = loadMaterialTextures(
                material,
                scene,
                aiTextureType_DIFFUSE,
                "texture_diffuse"
            );

            std::vector<Texture> baseColorMaps = loadMaterialTextures(
                material,
                scene,
                aiTextureType_BASE_COLOR,
                "texture_diffuse"
            );

            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());

            std::vector<Texture> specularMaps = loadMaterialTextures(
                material,
                scene,
                aiTextureType_SPECULAR,
                "texture_specular"
            );
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            // FBX / MMD / Toon 계열에서 diffuse 외 경로가 다른 타입으로 들어가는 경우도 있습니다.
            printMaterialInfo(material);
        }

        return Mesh(vertices, indices, textures);
    }

    std::vector<Texture> loadMaterialTextures(
        aiMaterial* mat,
        const aiScene* scene,
        aiTextureType type,
        const std::string& typeName
    )
    {
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            std::string texturePath = str.C_Str();

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (textures_loaded[j].path == texturePath)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromPath(texturePath, directory, scene);
                texture.type = typeName;
                texture.path = texturePath;

                textures.push_back(texture);
                textures_loaded.push_back(texture);

                std::cout << "Loaded texture: " << texturePath << std::endl;
            }
        }

        return textures;
    }

    unsigned int TextureFromPath(
        const std::string& path,
        const std::string& directory,
        const aiScene* scene
    )
    {
        if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.c_str()))
        {
            return TextureFromEmbeddedTexture(embeddedTexture, path);
        }

        std::string normalizedPath = normalizePath(path);
        std::string filenameOnly = fileNameFromPath(normalizedPath);
        std::vector<std::string> candidates = {
            normalizedPath,
            directory + "/" + normalizedPath,
            directory + "/" + filenameOnly,
            directory + "/../Textures/" + filenameOnly,
            "resource/Textures/" + filenameOnly,
            "../resource/Textures/" + filenameOnly,
            "resource/Models/" + filenameOnly,
            "../resource/Models/" + filenameOnly
        };

        for (const std::string& candidate : candidates)
        {
            if (fileExists(candidate))
            {
                return TextureFromFile(candidate);
            }
        }

        std::cout << "Texture failed to find: " << path << std::endl;
        return 0;
    }

    unsigned int TextureFromFile(const std::string& filename)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;

        stbi_set_flip_vertically_on_load(false);

        unsigned char* data = stbi_load(
            filename.c_str(),
            &width,
            &height,
            &nrComponents,
            0
        );

        if (data)
        {
            GLenum format = GL_RGB;

            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                format,
                width,
                height,
                0,
                format,
                GL_UNSIGNED_BYTE,
                data
            );

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load: " << filename << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    unsigned int TextureFromEmbeddedTexture(const aiTexture* texture, const std::string& path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = nullptr;

        if (texture->mHeight == 0)
        {
            data = stbi_load_from_memory(
                reinterpret_cast<const unsigned char*>(texture->pcData),
                static_cast<int>(texture->mWidth),
                &width,
                &height,
                &nrComponents,
                0
            );
        }
        else
        {
            width = static_cast<int>(texture->mWidth);
            height = static_cast<int>(texture->mHeight);
            nrComponents = 4;

            std::vector<unsigned char> pixels(width * height * 4);
            for (int i = 0; i < width * height; i++)
            {
                pixels[i * 4 + 0] = texture->pcData[i].r;
                pixels[i * 4 + 1] = texture->pcData[i].g;
                pixels[i * 4 + 2] = texture->pcData[i].b;
                pixels[i * 4 + 3] = texture->pcData[i].a;
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            setTextureParameters();

            std::cout << "Loaded embedded texture: " << path << std::endl;
            return textureID;
        }

        if (data)
        {
            GLenum format = textureFormat(nrComponents);

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            setTextureParameters();

            stbi_image_free(data);
            std::cout << "Loaded embedded texture: " << path << std::endl;
        }
        else
        {
            std::cout << "Embedded texture failed to load: " << path << std::endl;
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }

        return textureID;
    }

    GLenum textureFormat(int nrComponents)
    {
        if (nrComponents == 1)
            return GL_RED;
        if (nrComponents == 4)
            return GL_RGBA;
        return GL_RGB;
    }

    void setTextureParameters()
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    bool fileExists(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        return file.good();
    }

    std::string normalizePath(std::string path)
    {
        for (char& c : path)
        {
            if (c == '\\')
                c = '/';
        }
        return path;
    }

    std::string fileNameFromPath(const std::string& path)
    {
        size_t slash = path.find_last_of("/\\");
        if (slash == std::string::npos)
            return path;
        return path.substr(slash + 1);
    }

    void printMaterialInfo(aiMaterial* mat)
    {
        aiString name;
        mat->Get(AI_MATKEY_NAME, name);

        std::cout << "Material: " << name.C_Str() << std::endl;

        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++)
        {
            aiTextureType texType = static_cast<aiTextureType>(type);
            unsigned int count = mat->GetTextureCount(texType);

            if (count > 0)
            {
                std::cout << "  Texture type " << type << " count: " << count << std::endl;

                for (unsigned int i = 0; i < count; i++)
                {
                    aiString path;
                    mat->GetTexture(texType, i, &path);
                    std::cout << "    " << path.C_Str() << std::endl;
                }
            }
        }
    }
};
