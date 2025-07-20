#include <unordered_set>
#include <utility>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Instancing.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<engine/graphics/OpenGL.hpp>
#include <glad/glad.h>

namespace engine::resources {

void ResourcesController::initialize() {
    load_shaders();
    load_models();
    load_textures();
    load_skyboxes();
    load_instancing();
    load_bloom();
}

void ResourcesController::load_shaders() {
    if (!exists(m_shaders_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the shaders from", m_shaders_path.string());
        return;
    }
    for (const auto &shader_path: std::filesystem::directory_iterator(m_shaders_path)) {
        const auto name = shader_path.path()
                                     .stem()
                                     .string();
        shader(name, shader_path);
    }
}

void ResourcesController::load_models() {
    if (!exists(m_models_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the models from", m_models_path.string());
        return;
    }
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("models")) {
        throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                "No configuration for models in the config.json, please provide the resources config. See the example in the README.md");
    }
    for (const auto &model_entry: config["resources"]["models"].items()) { model(model_entry.key()); }
}

void ResourcesController::load_textures() {
    if (!exists(m_textures_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the textures from", m_textures_path.string());
        return;
    }
    for (const auto &texture_entry: std::filesystem::directory_iterator(m_textures_path)) {
        texture(texture_entry.path()
                             .stem()
                             .string(), texture_entry.path());
    }
}

void ResourcesController::load_skyboxes() {
    if (!exists(m_skyboxes_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the skyboxes from", m_skyboxes_path.string());
        return;
    }
    for (const auto &sky_boxes_entry: std::filesystem::directory_iterator(m_skyboxes_path)) {
        skybox(sky_boxes_entry.path()
                              .stem()
                              .string(), sky_boxes_entry.path());
    }
}

void ResourcesController::load_instancing() {
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("instancing")) {
        throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                "No configuration for instancing in the config.json, please provide the resources config. See the example in the README.md");
    }
    for (const auto &instancing_entry: config["resources"]["instancing"].items()) { instancing(instancing_entry.key()); }
}

void ResourcesController::load_bloom() { bloom(); }

/**
 * @class AssimpSceneProcessor
 * @brief Processes the meshes in an Assimp scene.
 */
class AssimpSceneProcessor {
public:
    /**
     * @brief Processes the meshes in the scene.
     * @returns The meshes in the scene.
     */
    std::vector<Mesh> process_meshes();

    explicit AssimpSceneProcessor(ResourcesController *resources_controller, const aiScene *scene,
                                  std::filesystem::path model_path) : m_scene(scene)
                                                                  , m_model_path(std::move(model_path))
                                                                  , m_resources_controller(resources_controller) {}

private:
    void process_node(const aiNode *node);

    void process_mesh(aiMesh *mesh);

    std::vector<Texture *> process_materials(const aiMaterial *material);

    void process_material_type(std::vector<Texture *> &textures, const aiMaterial *material, aiTextureType type);

    static TextureType assimp_texture_type_to_engine(aiTextureType type);

    std::vector<Mesh> m_meshes;
    const aiScene *m_scene;
    std::filesystem::path m_model_path;
    ResourcesController *m_resources_controller;
};

Model *ResourcesController::model(
        const std::string &name) {
    auto &result = m_models[name];
    if (!result) {
        auto &config = util::Configuration::config();
        if (!config["resources"]["models"].contains(name)) {
            throw util::EngineError(util::EngineError::Type::ConfigurationError, std::format(
                                            "No model ({}) specify in config.json. Please add the model to the config.json.",
                                            name));
        }
        std::filesystem::path model_path = m_models_path /
                                           std::filesystem::path(
                                                   config["resources"]["models"][name]["path"].get<
                                                       std::string>());
        Assimp::Importer importer;
        int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                    aiProcess_CalcTangentSpace;
        if (config["resources"]["models"][name].value<bool>("flip_uvs", false)) { flags |= aiProcess_FlipUVs; }

        spdlog::info("load_model(name={}, path={})", name, model_path.string());
        const aiScene *scene =
                importer.ReadFile(model_path, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                    std::format("Assimp error while reading model: {} from path {}.",
                                                model_path.string(), name));
        }
        AssimpSceneProcessor scene_processor(this, scene, model_path);
        std::vector<Mesh> meshes = scene_processor.process_meshes();
        result = std::make_unique<Model>(Model(std::move(meshes), model_path,
                                               name));
    }
    return result.get();
}

Texture *ResourcesController::texture(const std::string &name,
                                      const std::filesystem::path &path,
                                      TextureType type, bool flip_uvs) {
    auto &result = m_textures[name];
    if (!result) {
        spdlog::info("load_texture(path={})", path.string());
        result = std::make_unique<Texture>(Texture(graphics::OpenGL::generate_texture(path, flip_uvs), type, path,
                                                   path.stem()));
    }
    return result.get();
}

Skybox *ResourcesController::skybox(const std::string &name,
                                    const std::filesystem::path &path,
                                    bool flip_uvs) {
    auto &result = m_sky_boxes[name];
    if (!result) {
        spdlog::info("load_skybox(path={})", path.string());
        result = std::make_unique<Skybox>(Skybox(graphics::OpenGL::init_skybox_cube(),
                                                 graphics::OpenGL::load_skybox_textures(path, flip_uvs),
                                                 path, name));
    }
    return result.get();
}

Instancing *ResourcesController::instancing(const std::string &name) {
    auto &result = m_instancing[name];
    if (!result) {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        auto &config = util::Configuration::config();
        if (!config["resources"]["instancing"].contains(name)) {
            throw util::EngineError(util::EngineError::Type::ConfigurationError, std::format(
                                            "No instancing ({}) specify in config.json. Please add the instancing to the config.json.",
                                            name));
        }
        uint32_t amount = config["resources"]["instancing"][name]["amount"].get<uint32_t>();
        std::string model_name = name;
        std::vector<glm::mat4> model_matrices(amount);
        srand(static_cast<unsigned int>(platform->current()));
        for (unsigned int i = 0; i < (amount - 1) / 50; i++) {
            float z_start = -120.0f, x_start = -5.0f * i + 200.0f, y_start = 50.0f;
            for (int j = 0; j < 50; j++) {
                glm::mat4 model = glm::mat4(1.0f);
                float displacement = (rand() % 400) / 100.0f - 2.0f;
                float x = x_start + displacement;
                displacement = (rand() % 400) / 100.0f - 2.0f;
                float y = y_start + displacement * 0.4f;
                displacement = (rand() % 400) / 100.0f - 2.0f;
                float z = z_start + displacement;
                z_start += 5.0f;
                model = glm::translate(model, glm::vec3(x, y, z));

                float scale = static_cast<float>((rand() % 20) / 10.00 + 1.0);
                model = glm::scale(model, glm::vec3(scale));

                model_matrices[i * 50 + j] = model;
            }
        }
        float z_start = -120.0f, x_start = -5.0f * ((amount - 1) / 50) + 200.0f, y_start = 50.0f;
        for (int j = ((amount - 1) / 50) * 50; j < amount; j++) {
            glm::mat4 model = glm::mat4(1.0f);
            float displacement = (rand() % 400) / 100.0f - 2.0f;
            float x = x_start + displacement;
            displacement = (rand() % 400) / 100.0f - 2.0f;
            float y = y_start + displacement * 0.4f;
            displacement = (rand() % 400) / 100.0f - 2.0f;
            float z = z_start + displacement;
            z_start += 5.0f;
            model = glm::translate(model, glm::vec3(x, y, z));

            float scale = static_cast<float>((rand() % 20) / 10.00 + 1.0);
            model = glm::scale(model, glm::vec3(scale));

            model_matrices[j] = model;
        }

        unsigned int buffer;
        CHECKED_GL_CALL(glGenBuffers, 1, &buffer);
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, buffer);
        CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &model_matrices[0], GL_STATIC_DRAW);

        unsigned int size_meshes = m_models[model_name]->meshes().size();
        for (unsigned int i = 0; i < size_meshes; i++) {
            unsigned int VAO = m_models[model_name]->meshes()[i].m_vao;
            CHECKED_GL_CALL(glBindVertexArray, VAO);
            CHECKED_GL_CALL(glEnableVertexAttribArray, 3);
            CHECKED_GL_CALL(glVertexAttribPointer, 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) 0);
            CHECKED_GL_CALL(glEnableVertexAttribArray, 4);
            CHECKED_GL_CALL(glVertexAttribPointer, 4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
            CHECKED_GL_CALL(glEnableVertexAttribArray, 5);
            CHECKED_GL_CALL(glVertexAttribPointer, 5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
            CHECKED_GL_CALL(glEnableVertexAttribArray, 6);
            CHECKED_GL_CALL(glVertexAttribPointer, 6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

            CHECKED_GL_CALL(glVertexAttribDivisor, 3, 1);
            CHECKED_GL_CALL(glVertexAttribDivisor, 4, 1);
            CHECKED_GL_CALL(glVertexAttribDivisor, 5, 1);
            CHECKED_GL_CALL(glVertexAttribDivisor, 6, 1);

            CHECKED_GL_CALL(glBindVertexArray, 0);
        }

        spdlog::info("load_instancing(name={})", name);
        result = std::make_unique<Instancing>(Instancing(model_matrices, &(*(m_models[model_name])), amount));
    }
    return result.get();
}

Shader *ResourcesController::shader(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_shaders[name];
    if (!result) {
        spdlog::info("load_shader(path={})", path.string());
        result = std::make_unique<Shader>(ShaderCompiler::compile_from_file(name, path));
    }
    return result.get();
}

Bloom *ResourcesController::bloom() {
    auto &result = m_blooms["bloom"];
    if (!result) {
        auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
        unsigned int hdrFBO;
        CHECKED_GL_CALL(glGenFramebuffers, 1, &hdrFBO);
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, hdrFBO);

        unsigned int colorBuffers[2];
        CHECKED_GL_CALL(glGenTextures, 2, colorBuffers);
        for (unsigned int i = 0; i < 2; i++) {
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, platform->window()->width(), platform->window()->height(), 0, GL_RGBA, GL_FLOAT, NULL);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
        }
        unsigned int rboDepth;
        CHECKED_GL_CALL(glGenRenderbuffers, 1, &rboDepth);
        CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rboDepth);
        CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, platform->window()->width(), platform->window()->height());
        CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { spdlog::info("Framebuffer not complete!"); }
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);

        unsigned int pingpongFBO[2];
        unsigned int pingpongColorbuffers[2];
        CHECKED_GL_CALL(glGenFramebuffers, 2, pingpongFBO);
        CHECKED_GL_CALL(glGenTextures, 2, pingpongColorbuffers);
        for (unsigned int i = 0; i < 2; i++) {
            CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, pingpongFBO[i]);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, pingpongColorbuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, platform->window()->width(), platform->window()->height(), 0, GL_RGBA, GL_FLOAT, NULL);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { spdlog::info("Framebuffer not complete!"); }
        }

        spdlog::info("load_bloom");
        result = std::make_unique<Bloom>(Bloom(pingpongFBO, pingpongColorbuffers, colorBuffers, hdrFBO));
    }
    return result.get();
}

std::vector<Mesh> AssimpSceneProcessor::process_meshes() {
    m_meshes.clear();
    process_node(m_scene->mRootNode);
    return std::move(m_meshes);
}

void AssimpSceneProcessor::process_node(const aiNode *node) {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = m_scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh);
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i) { process_node(node->mChildren[i]); }
}

void AssimpSceneProcessor::process_mesh(aiMesh *mesh) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position
              .x = mesh->mVertices[i].x;
        vertex.Position
              .y = mesh->mVertices[i].y;
        vertex.Position
              .z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.Normal
                  .x = mesh->mNormals[i].x;
            vertex.Normal
                  .y = mesh->mNormals[i].y;
            vertex.Normal
                  .z = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords
                  .x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords
                  .y = mesh->mTextureCoords[0][i].y;

            vertex.Tangent
                  .x = mesh->mTangents[i].x;
            vertex.Tangent
                  .y = mesh->mTangents[i].y;
            vertex.Tangent
                  .z = mesh->mTangents[i].z;

            vertex.Bitangent
                  .x = mesh->mBitangents[i].x;
            vertex.Bitangent
                  .y = mesh->mBitangents[i].y;
            vertex.Bitangent
                  .z = mesh->mBitangents[i].z;
        }
        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) { indices.push_back(face.mIndices[j]); }
    }

    auto material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture *> textures = process_materials(material);
    m_meshes.emplace_back(Mesh(vertices, indices, std::move(textures)));
}

std::vector<Texture *> AssimpSceneProcessor::process_materials(const aiMaterial *material) {
    std::vector<Texture *> textures;
    auto ai_texture_types = {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
            aiTextureType_NORMALS,
            aiTextureType_HEIGHT,
    };

    for (auto ai_texture_type: ai_texture_types) { process_material_type(textures, material, ai_texture_type); }
    return textures;
}

void AssimpSceneProcessor::process_material_type(std::vector<Texture *> &textures, const aiMaterial *material,
                                                 aiTextureType type) {
    auto material_count = material->GetTextureCount(type);
    for (uint32_t i = 0; i < material_count; ++i) {
        aiString ai_texture_path_string;
        material->GetTexture(type, i, &ai_texture_path_string);
        std::filesystem::path texture_path = m_model_path.parent_path() / ai_texture_path_string.C_Str();
        Texture *texture = m_resources_controller->texture(texture_path.string(), texture_path,
                                                           assimp_texture_type_to_engine(type));
        textures.emplace_back(texture);
    }
}

TextureType AssimpSceneProcessor::assimp_texture_type_to_engine(aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType::Diffuse;
        case aiTextureType_SPECULAR: return TextureType::Specular;
        case aiTextureType_HEIGHT: return TextureType::Height;
        case aiTextureType_NORMALS: return TextureType::Normal;
        default: RG_SHOULD_NOT_REACH_HERE("Engine currently doesn't support the aiTextureType: {}",
                                          static_cast<int>(type));
    }
}

}// namespace engine
