//
// Created by cvnpko on 7/19/25.
//

#ifndef INSTANCING_HPP
#define INSTANCING_HPP
#include <engine/resources/Model.hpp>

namespace engine::resources {

class Instancing {
    friend class ResourcesController;

public:
    void destroy();

private:
    uint32_t m_amount;
    Model m_model;
    std::vector<glm::mat4> m_model_matrices;

    Instancing() = default;

    Instancing(std::vector<glm::mat4> model_matrices, Model &model, uint32_t amount) : m_model_matrices(std::move(model_matrices))
                                                                                   , m_model(std::move(model))
                                                                                   , m_amount(std::move(amount)) {}
};
}// namespace engine

#endif //INSTANCING_HPP
