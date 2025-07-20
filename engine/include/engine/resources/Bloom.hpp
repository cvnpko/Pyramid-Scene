//
// Created by cvnpko on 7/20/25.
//

#ifndef BLOOM_HPP
#define BLOOM_HPP

#include <memory>
#include <assimp/Importer.hpp>
#include <engine/core/Controller.hpp>
#include <engine/platform/BloomObserver.hpp>
#include <engine/platform/PlatformController.hpp>

namespace engine::resources {

class Bloom {
    friend class ResourcesController;

public:
    void destroy();

    uint32_t get_colorbuffer(bool horizontal) const;

    uint32_t get_pingpong_colorbuffers(bool horizontal) const;

    uint32_t get_quadVAO() const;

    uint32_t get_rboDepth() const;

    void set_quadVAO(uint32_t quadVAO);

    void activate_hdrFBO() const;

    void activate_pingpong_FBO(bool horizontal) const;

private:
    uint32_t m_pingpong_FBO[2];
    uint32_t m_pingpong_colorbuffers[2];
    uint32_t m_colorbuffers[2];
    uint32_t m_hdrFBO;
    uint32_t m_quadVAO;
    uint32_t m_rboDepth;

    Bloom(uint32_t pingpong_FBO[2], uint32_t pingpong_colorbuffers[2], uint32_t colorbuffers[2], uint32_t hdr_FBO, uint32_t rboDepth)
    : m_hdrFBO(hdr_FBO)
  , m_quadVAO(0)
  , m_rboDepth(rboDepth) {
        auto observer = std::make_unique<platform::BloomObserver>();
        engine::core::Controller::get<engine::platform::PlatformController>()->register_platform_event_observer(
                std::move(observer));
        for (int i = 0; i < 2; i++) {
            m_pingpong_FBO[i] = pingpong_FBO[i];
            m_pingpong_colorbuffers[i] = pingpong_colorbuffers[i];
            m_colorbuffers[i] = colorbuffers[i];
        }
    }
};
}// namespace engine

#endif //BLOOM_HPP
