//
// Created by cvnpko on 7/14/25.
//
#include <MainController.hpp>
#include <MainEventController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <spdlog/spdlog.h>

namespace app {
void MainController::setLight(engine::resources::Shader *shader) {
    auto graphics = get<engine::graphics::GraphicsController>();

    shader->use();
    shader->set_vec3("pointLightSun.position", glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(angle),
                                                                     glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(m_point_light_sun.position, 1.0)));
    shader->set_vec3("pointLightSun.ambient", m_point_light_sun.ambient);
    shader->set_vec3("pointLightSun.diffuse", m_point_light_sun.diffuse);
    shader->set_vec3("pointLightSun.specular", m_point_light_sun.specular);
    shader->set_float("pointLightSun.constant", m_point_light_sun.constant);
    shader->set_float("pointLightSun.linear", m_point_light_sun.linear);
    shader->set_float("pointLightSun.quadratic", m_point_light_sun.quadratic);

    shader->set_vec3("pointLightMoon.position", glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(angle),
                                                                      glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(m_point_light_moon.position, 1.0)));
    shader->set_vec3("pointLightMoon.ambient", m_point_light_moon.ambient);
    shader->set_vec3("pointLightMoon.diffuse", m_point_light_moon.diffuse);
    shader->set_vec3("pointLightMoon.specular", m_point_light_moon.specular);
    shader->set_float("pointLightMoon.constant", m_point_light_moon.constant);
    shader->set_float("pointLightMoon.linear", m_point_light_moon.linear);
    shader->set_float("pointLightMoon.quadratic", m_point_light_moon.quadratic);

    shader->set_vec3("pointLightPyramid.position", m_point_light_pyramid.position);
    shader->set_vec3("pointLightPyramid.ambient", m_point_light_pyramid.ambient);
    shader->set_vec3("pointLightPyramid.diffuse", m_point_light_pyramid.diffuse);
    shader->set_vec3("pointLightPyramid.specular", m_point_light_pyramid.specular);
    shader->set_float("pointLightPyramid.constant", m_point_light_pyramid.constant);
    shader->set_float("pointLightPyramid.linear", m_point_light_pyramid.linear);
    shader->set_float("pointLightPyramid.quadratic", m_point_light_pyramid.quadratic);

    shader->set_vec3("spotLight.position", graphics->camera()->Position);
    shader->set_vec3("spotLight.direction", graphics->camera()->Front);
    shader->set_vec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.diffuse", m_spotlight_enabled ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_vec3("spotLight.specular", m_spotlight_enabled ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f));
    shader->set_float("spotLight.constant", 1.0f);
    shader->set_float("spotLight.linear", 0.09f);
    shader->set_float("spotLight.quadratic", 0.032f);
    shader->set_float("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader->set_float("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();
    auto graphics = engine::graphics::GraphicsController::get<engine::graphics::GraphicsController>();
    engine::graphics::PerspectiveMatrixParams &perspective_matrix = graphics->perspective_params();
    perspective_matrix.Far = 500.0f;

    auto shader = get<engine::resources::ResourcesController>()->shader("basic");
    shader->use();
    shader->set_int("diffuseTexture", 0);

    auto shader_instancing = get<engine::resources::ResourcesController>()->shader("instancing");
    shader_instancing->use();
    shader_instancing->set_int("diffuseTexture", 0);

    auto shader_blur = get<engine::resources::ResourcesController>()->shader("bloom_blur");
    shader_blur->use();
    shader_blur->set_int("image", 0);

    auto shader_bloom_final = get<engine::resources::ResourcesController>()->shader("bloom_final");
    shader_bloom_final->use();
    shader_bloom_final->set_int("scene", 0);
    shader_bloom_final->set_int("bloomBlur", 1);

    m_point_light_sun.position = glm::vec3(0.0f, 300.0f, 0.0f);
    m_point_light_sun.ambient = m_sun_light * m_sun_light_coeff * m_ambient_coeff;
    m_point_light_sun.diffuse = m_sun_light * m_sun_light_coeff;
    m_point_light_sun.specular = m_sun_light * m_sun_light_coeff;
    m_point_light_sun.constant = 1.0f;
    m_point_light_sun.linear = 0.009f;
    m_point_light_sun.quadratic = 0.00004f;

    m_point_light_moon.position = glm::vec3(0.0f, -300.0f, 0.0f);
    m_point_light_moon.ambient = m_moon_light * m_moon_light_coeff * m_ambient_coeff;
    m_point_light_moon.diffuse = m_moon_light * m_moon_light_coeff;
    m_point_light_moon.specular = m_moon_light * m_moon_light_coeff;
    m_point_light_moon.constant = 1.0f;
    m_point_light_moon.linear = 0.009f;
    m_point_light_moon.quadratic = 0.00004f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(50.00f, 30.0f, 0.0f));
    model = glm::scale(model, glm::vec3(50.0f));
    m_point_light_pyramid.position = glm::vec3(model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_point_light_pyramid.ambient = m_pyramid_light * m_pyramid_light_coeff * m_ambient_coeff;
    m_point_light_pyramid.diffuse = m_pyramid_light * m_pyramid_light_coeff;
    m_point_light_pyramid.specular = m_pyramid_light * m_pyramid_light_coeff;
    m_point_light_pyramid.constant = 1.0f;
    m_point_light_pyramid.linear = 0.09f;
    m_point_light_pyramid.quadratic = 0.0032f;
}

bool MainController::loop() {
    const auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE)
                .state() == engine::platform::Key::State::JustPressed) { return false; }
    return true;
}

void MainController::poll_events() {
    const auto platform = get<engine::platform::PlatformController>();
    auto main_event_controller = get<app::MainEventController>();
    angle = main_event_controller->get_event_angle();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) { platform->set_enable_cursor(m_cursor_enabled = !m_cursor_enabled); }
    if (platform->key(engine::platform::KeyId::KEY_SPACE).state() == engine::platform::Key::State::JustPressed) { m_bloom = !m_bloom; }
    if (platform->key(engine::platform::KEY_F).state() == engine::platform::Key::State::JustPressed) { m_spotlight_enabled = !m_spotlight_enabled; }
}

void MainController::update() { update_camera(); }

void MainController::begin_draw() { engine::graphics::OpenGL::clear_buffers(); }

void MainController::draw() {
    auto bloom = get<engine::resources::ResourcesController>()->bloom();
    bloom->activate_hdrFBO();
    engine::graphics::OpenGL::clear_buffers();
    draw_dunes();
    draw_pyramids();
    draw_sphinx();
    draw_camels();
    draw_instancing();
    draw_moon();
    draw_sun();
    draw_pyramid();
    draw_skybox();
    draw_bloom();
}

void MainController::end_draw() { get<engine::platform::PlatformController>()->swap_buffers(); }

void MainController::draw_dunes() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("basic");
    auto dunes = get<engine::resources::ResourcesController>()->model("dunes");
    shader->use();

    setLight(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.1f, 0.0f));
    model = glm::scale(model, glm::vec3(15.0f, 2.0f, 15.0f));
    shader->set_mat4("model", model);
    dunes->draw(shader);
}

void MainController::draw_pyramids() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("basic");
    auto pyramid = get<engine::resources::ResourcesController>()->model("pyramid");
    shader->use();

    setLight(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model1 = glm::mat4(1.0f);
    model1 = glm::translate(model1, glm::vec3(50.00f, -1.098f, 0.0f));
    model1 = glm::scale(model1, glm::vec3(376.8f));
    shader->set_mat4("model", model1);
    pyramid->draw(shader);

    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(0.0f, -1.09f, -50.0f));
    model2 = glm::scale(model2, glm::vec3(367.2f));
    shader->set_mat4("model", model2);
    pyramid->draw(shader);

    glm::mat4 model3 = glm::mat4(1.0f);
    model3 = glm::translate(model3, glm::vec3(-50.0f, -1.081f, -90.0f));
    model3 = glm::scale(model3, glm::vec3(168.0f));
    shader->set_mat4("model", model3);
    pyramid->draw(shader);
}

void MainController::draw_sphinx() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("basic");
    auto sphinx = get<engine::resources::ResourcesController>()->model("sphinx");
    shader->use();

    setLight(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 7.7f, 50.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f));
    shader->set_mat4("model", model);
    sphinx->draw(shader);
}

void MainController::draw_camels() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("basic");
    auto camel = get<engine::resources::ResourcesController>()->model("camel");
    shader->use();

    setLight(shader);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model1 = glm::mat4(1.0f);
    model1 = glm::translate(model1, glm::vec3(-20.0f, -0.58f, 20.0f));
    model1 = glm::rotate(model1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::scale(model1, glm::vec3(2.0f));
    shader->set_mat4("model", model1);
    camel->draw(shader);
    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(-25.0f, -0.55f, 25.0f));
    model2 = glm::rotate(model2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model2 = glm::scale(model2, glm::vec3(2.0f));
    shader->set_mat4("model", model2);
    camel->draw(shader);
}

void MainController::draw_skybox() {
    auto main_event_controller = get<app::MainEventController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("skybox");
    auto skybox_cube = get<engine::resources::ResourcesController>()->skybox(main_event_controller->is_day() ? "day" : "night");
    get<engine::graphics::GraphicsController>()->draw_skybox(shader, skybox_cube);
}

void MainController::draw_instancing() {
    auto shader = get<engine::resources::ResourcesController>()->shader("instancing");
    auto instancing_model = get<engine::resources::ResourcesController>()->instancing("bird");
    shader->use();
    setLight(shader);
    get<engine::graphics::GraphicsController>()->draw_instancing(shader, instancing_model);
}

void MainController::draw_bloom() {
    auto shader_blur = get<engine::resources::ResourcesController>()->shader("bloom_blur");
    auto shader_final = get<engine::resources::ResourcesController>()->shader("bloom_final");
    auto main_event_controller = get<app::MainEventController>();
    auto bloom = get<engine::resources::ResourcesController>()->bloom();
    get<engine::graphics::GraphicsController>()->draw_bloom(shader_blur, shader_final, bloom, main_event_controller->is_day() ? 1.20f : 1.80f, m_bloom);
}

void MainController::draw_moon() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("light_shader");
    auto main_event_controller = get<app::MainEventController>();
    auto moon = get<engine::resources::ResourcesController>()->model("moon");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()
                                     ->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, -300.0f, 0.0f));
    model = glm::scale(model, glm::vec3(15.0f));
    shader->set_mat4("model", model);
    shader->set_vec3("light", m_moon_light);
    shader->set_float("light_coeff", m_moon_light_coeff);
    moon->draw(shader);
}

void MainController::draw_sun() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("light_shader");
    auto sun = get<engine::resources::ResourcesController>()->model("sun");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, 300.0f, 0.0f));
    model = glm::scale(model, glm::vec3(15.0f));
    shader->set_mat4("model", model);
    shader->set_vec3("light", m_sun_light);
    shader->set_float("light_coeff", m_sun_light_coeff);
    sun->draw(shader);
}

void MainController::draw_pyramid() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto shader = get<engine::resources::ResourcesController>()->shader("light_shader");
    auto pyramid = get<engine::resources::ResourcesController>()->model("pyramid");
    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(50.00f, 23.8f, 0.0f));
    model = glm::scale(model, glm::vec3(50.0f));
    shader->set_mat4("model", model);
    shader->set_vec3("light", m_pyramid_light);
    shader->set_float("light_coeff", m_pyramid_light_coeff);
    pyramid->draw(shader);
}

void MainController::update_camera() {
    auto platform = get<engine::platform::PlatformController>();
    auto camera = get<engine::graphics::GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_W).state() == engine::platform::Key::State::Pressed) { camera->move_camera(engine::graphics::Camera::FORWARD, 6 * dt); }
    if (platform->key(engine::platform::KEY_S).state() == engine::platform::Key::State::Pressed) { camera->move_camera(engine::graphics::Camera::BACKWARD, 6 * dt); }
    if (platform->key(engine::platform::KEY_A).state() == engine::platform::Key::State::Pressed) { camera->move_camera(engine::graphics::Camera::LEFT, 6 * dt); }
    if (platform->key(engine::platform::KEY_D).state() == engine::platform::Key::State::Pressed) { camera->move_camera(engine::graphics::Camera::RIGHT, 6 * dt); }
    auto mouse = platform->mouse();
    camera->rotate_camera(mouse.dx, mouse.dy);
    camera->zoom(mouse.scroll);
}

}// app