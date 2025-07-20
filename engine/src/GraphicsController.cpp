
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Bloom.hpp>
#include <engine/resources/Skybox.hpp>

namespace engine::graphics {

void GraphicsController::initialize() {
    const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

    auto platform = engine::core::Controller::get<platform::PlatformController>();
    auto handle = platform->window()
                          ->handle_();
    m_perspective_params.FOV = glm::radians(m_camera.Zoom);
    m_perspective_params.Width = static_cast<float>(platform->window()
                                                            ->width());
    m_perspective_params.Height = static_cast<float>(platform->window()
                                                             ->height());
    m_perspective_params.Near = 0.1f;
    m_perspective_params.Far = 100.f;

    m_ortho_params.Bottom = 0.0f;
    m_ortho_params.Top = static_cast<float>(platform->window()
                                                    ->height());
    m_ortho_params.Left = 0.0f;
    m_ortho_params.Right = static_cast<float>(platform->window()
                                                      ->width());
    m_ortho_params.Near = 0.1f;
    m_ortho_params.Far = 500.0f;
    platform->register_platform_event_observer(
            std::make_unique<GraphicsPlatformEventObserver>(this));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
    RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");
}

void GraphicsController::terminate() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
    m_graphics->perspective_params()
              .Width = static_cast<float>(width);
    m_graphics->perspective_params()
              .Height = static_cast<float>(height);

    m_graphics->orthographic_params()
              .Right = static_cast<float>(width);
    m_graphics->orthographic_params()
              .Top = static_cast<float>(height);
}

std::string_view GraphicsController::name() const { return "GraphicsController"; }

void GraphicsController::begin_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicsController::end_gui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
    CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
    CHECKED_GL_CALL(glBindVertexArray, 0);
    CHECKED_GL_CALL(glDepthFunc, GL_LESS);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}

void GraphicsController::draw_instancing(const resources::Shader *shader, const resources::Instancing *instancing) {
    glm::mat4 view = m_camera.view_matrix();
    shader->use();
    shader->set_mat4("view", view);
    shader->set_mat4("projection", projection_matrix<>());
    shader->set_int("texture_diffuse1", 0);
    int amount = instancing->get_amount();
    auto model = instancing->get_model();
    if (model->meshes()[0].m_textures.size() > 0) {
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->meshes()[0].m_textures[0]->id());
    }
    for (unsigned int i = 0; i < model->meshes().size(); i++) {
        CHECKED_GL_CALL(glBindVertexArray, model->meshes()[i].m_vao);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(model->meshes()[i].m_num_indices), GL_UNSIGNED_INT, 0, amount);
        CHECKED_GL_CALL(glBindVertexArray, 0);
    }

    for (unsigned int i = 0; i < amount; i++) {
        shader->set_mat4("model", instancing->get_model_matrix(i));
        model->draw(shader);
    }
}

void render_quad(resources::Bloom *bloom) {
    uint32_t quadVAO = bloom->get_quadVAO(), quadVBO = 0;
    if (quadVAO == 0) {
        float quadVertices[] = {
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        bloom->set_quadVAO(quadVAO);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GraphicsController::draw_bloom(const resources::Shader *shader_blur, const resources::Shader *shader_final, resources::Bloom *bloom, float exposure, bool use_bloom) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    shader_blur->use();
    for (unsigned int i = 0; i < amount; i++) {
        bloom->activate_pingpong_FBO(horizontal);
        shader_blur->set_int("horizontal", horizontal);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, first_iteration ? bloom->get_colorbuffer(1) : bloom->get_pingpong_colorbuffers(!horizontal));
        render_quad(bloom);
        horizontal = !horizontal;
        if (first_iteration) first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader_final->use();
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, bloom->get_colorbuffer(0));
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, bloom->get_pingpong_colorbuffers(!horizontal));
    shader_final->set_bool("bloom", use_bloom);
    shader_final->set_float("exposure", exposure);
    render_quad(bloom);
}

}
