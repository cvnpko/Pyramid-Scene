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

    Model *get_model() const { return m_model; }

    glm::mat4 get_model_matrix(int i) const { return m_model_matrices[i]; }

    uint32_t get_amount() const { return uint32_t(m_model_matrices.size()); }

private:
    uint32_t m_amount;
    Model *m_model;
    std::vector<glm::mat4> m_model_matrices;

    Instancing() = default;

    Instancing(std::vector<glm::mat4> &model_matrices, Model *model, uint32_t amount) : m_model_matrices(std::move(model_matrices))
                                                                                    , m_model(std::move(model))
                                                                                    , m_amount(std::move(amount)) {}
};
}// namespace engine

#endif //INSTANCING_HPP
