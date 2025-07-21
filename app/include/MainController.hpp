//
// Created by cvnpko on 7/14/25.
//

#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

#include <memory>
#include <engine/core/Controller.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/core/Engine.hpp>

namespace app {

class MainController final : public engine::core::Controller {
public:
    std::string_view name() const override { return "app::MainController"; }

private:
    void initialize() override;

    void set_light(engine::resources::Shader *shader);

    bool loop() override;

    void poll_events() override;

    void update() override;

    void begin_draw() override;

    void draw_skybox();

    void draw_instancing();

    void draw_dunes();

    void draw_pyramids();

    void draw_sphinx();

    void draw_camels();

    void draw_bloom();

    void draw_moon();

    void draw_sun();

    void draw_pyramid();

    void draw() override;

    void end_draw() override;

    void update_camera();

    bool m_cursor_enabled{true};
    bool m_bloom{true};
    bool m_spotlight_enabled{false};

    glm::vec3 m_sun_light{glm::vec3(255.0f, 255.0f, 224.0f) / 255.0f};
    float m_sun_light_coeff{0.7f};

    glm::vec3 m_moon_light{glm::vec3(247.0f, 234.0f, 198.0f) / 255.0f};
    float m_moon_light_coeff{0.1f};

    glm::vec3 m_pyramid_light{glm::vec3(247.0f, 234.0f, 198.0f) / 255.0f};
    float m_pyramid_light_coeff{1.0f};

    float m_ambient_coeff{0.01f};
    float m_angle{0.0f};

    struct PointLight {
        glm::vec3 position;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        float constant;
        float linear;
        float quadratic;

    } m_point_light_sun, m_point_light_moon, m_point_light_pyramid;

};

}// app

#endif //MAINCONTROLLER_HPP
