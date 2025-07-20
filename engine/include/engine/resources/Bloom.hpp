//
// Created by cvnpko on 7/20/25.
//

#ifndef BLOOM_HPP
#define BLOOM_HPP

#include <assimp/Importer.hpp>

namespace engine::resources {

class Bloom {
    friend class ResourcesController;

public:
    void destroy();

    uint32_t get_colorbuffer(bool horizontal) const;

    uint32_t get_pingpong_colorbuffers(bool horizontal) const;

    uint32_t get_quadVAO() const;

    void set_quadVAO(uint32_t quadVAO);

    void activate_hdrFBO() const;

    void activate_pingpong_FBO(bool horizontal) const;

private:
    uint32_t m_pingpong_FBO[2];
    uint32_t m_pingpong_colorbuffers[2];
    uint32_t m_colorbuffers[2];
    uint32_t m_hdrFBO;
    uint32_t m_quadVAO;

    Bloom(uint32_t pingpong_FBO[2], uint32_t pingpong_colorbuffers[2], uint32_t colorbuffers[2], uint32_t hdr_FBO)
    : m_hdrFBO(hdr_FBO)
  , m_quadVAO(0) {
        for (int i = 0; i < 2; i++) {
            m_pingpong_FBO[i] = pingpong_FBO[i];
            m_pingpong_colorbuffers[i] = pingpong_colorbuffers[i];
            m_colorbuffers[i] = colorbuffers[i];
        }
    }
};
}// namespace engine

#endif //BLOOM_HPP
