//
// Created by cvnpko on 7/20/25.
//

#include <engine/resources/Bloom.hpp>
#include <engine/graphics/OpenGL.hpp>

namespace engine::resources {

uint32_t Bloom::get_colorbuffer(bool horizontal) const { return m_colorbuffers[horizontal]; }

uint32_t Bloom::get_pingpong_colorbuffers(bool horizontal) const { return m_pingpong_colorbuffers[horizontal]; }

void Bloom::activate_hdr_fbo() const { graphics::OpenGL::bind_frame_buffer(m_hdr_fbo); }

void Bloom::activate_pingpong_fbo(bool horizontal) const { graphics::OpenGL::bind_frame_buffer(m_pingpong_fbo[horizontal]); }

uint32_t Bloom::get_quad_vao() const { return m_quad_vao; }

uint32_t Bloom::get_rbo_depth() const { return m_rbo_depth; }

void Bloom::set_quad_vao(uint32_t quad_vao) { m_quad_vao = quad_vao; }
}