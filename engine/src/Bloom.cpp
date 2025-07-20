//
// Created by cvnpko on 7/20/25.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine/resources/Bloom.hpp>
#include <engine/graphics/OpenGL.hpp>

namespace engine::resources {

uint32_t Bloom::get_colorbuffer(bool horizontal) const { return m_colorbuffers[horizontal]; }

uint32_t Bloom::get_pingpong_colorbuffers(bool horizontal) const { return m_pingpong_colorbuffers[horizontal]; }

void Bloom::activate_hdrFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
}

void Bloom::activate_pingpong_FBO(bool horizontal) const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_pingpong_FBO[horizontal]);
}

uint32_t Bloom::get_quadVAO() const { return m_quadVAO; }

uint32_t Bloom::get_rboDepth() const { return m_rboDepth; }

void Bloom::set_quadVAO(uint32_t quadVAO) { m_quadVAO = quadVAO; }
}