//
// Created by cvnpko on 7/20/25.
//

#include <engine/core/Controller.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/BloomObserver.hpp>
#include <engine/resources/Bloom.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace engine::platform {
void BloomObserver::on_mouse_move(MousePosition position) {}

void BloomObserver::on_scroll(MousePosition position) {}

void BloomObserver::on_key(Key key) {}

void BloomObserver::on_window_resize(int width, int height) {
    auto bloom = engine::core::Controller::get<engine::resources::ResourcesController>()->bloom();

    bloom->activate_hdrFBO();

    for (unsigned int i = 0; i < 2; i++) {
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, bloom->get_colorbuffer(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, bloom->get_colorbuffer(i), 0);
    }
    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, bloom->get_rboDepth());
    CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloom->get_rboDepth());

    for (unsigned int i = 0; i < 2; i++) {
        bloom->activate_pingpong_FBO(i);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, bloom->get_pingpong_colorbuffers(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom->get_pingpong_colorbuffers(i), 0);
    }
    spdlog::info("resize successful");

}
}