//
// Created by cvnpko on 7/20/25.
//

#include <engine/core/Controller.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/BloomObserver.hpp>
#include <engine/resources/Bloom.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <spdlog/spdlog.h>

namespace engine::platform {
void BloomObserver::on_mouse_move(MousePosition position) {}

void BloomObserver::on_scroll(MousePosition position) {}

void BloomObserver::on_key(Key key) {}

void BloomObserver::on_window_resize(int width, int height) {
    auto bloom = engine::core::Controller::get<engine::resources::ResourcesController>()->bloom();

    bloom->activate_hdr_fbo();

    for (unsigned int i = 0; i < 2; i++) { engine::graphics::OpenGL::bloom_observer_color_buffer(bloom->get_colorbuffer(i), width, height, i); }

    engine::graphics::OpenGL::bloom_observer_render_buffer(bloom->get_rbo_depth(), width, height);

    for (unsigned int i = 0; i < 2; i++) {
        bloom->activate_pingpong_fbo(i);
        engine::graphics::OpenGL::bloom_observer_color_buffer(bloom->get_colorbuffer(i), width, height, 0);
    }
    spdlog::info("resize successful");

}
}