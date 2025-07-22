//
// Created by cvnpko on 7/20/25.
//

#ifndef BLOOM_HPP
#define BLOOM_HPP

#include <memory>
#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformController.hpp>

namespace engine::resources {

class Bloom {
    friend class ResourcesController;

public:
    void destroy();

    uint32_t get_colorbuffer(bool horizontal) const;

    uint32_t get_pingpong_colorbuffers(bool horizontal) const;

    uint32_t get_quad_vao() const;

    uint32_t get_rbo_depth() const;

    void set_quad_vao(uint32_t quad_vao);

    void activate_hdr_fbo() const;

    void activate_pingpong_fbo(bool horizontal) const;

private:
    uint32_t m_pingpong_fbo[2];
    uint32_t m_pingpong_colorbuffers[2];
    uint32_t m_colorbuffers[2];
    uint32_t m_hdr_fbo;
    uint32_t m_quad_vao;
    uint32_t m_rbo_depth;

    Bloom(uint32_t pingpong_fbo[2], uint32_t pingpong_colorbuffers[2], uint32_t colorbuffers[2], uint32_t hdr_fbo, uint32_t rbo_depth)
    : m_hdr_fbo(hdr_fbo)
  , m_quad_vao(0)
  , m_rbo_depth(rbo_depth) {
        for (int i = 0; i < 2; i++) {
            m_pingpong_fbo[i] = pingpong_fbo[i];
            m_pingpong_colorbuffers[i] = pingpong_colorbuffers[i];
            m_colorbuffers[i] = colorbuffers[i];
        }
    }
};
}// namespace engine

#endif //BLOOM_HPP
