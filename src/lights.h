#pragma once

#include "CG/buffer.h"

#include "glm/vec3.hpp"
#include "GL/glew.h"

#include <vector>


// Mirror the layout of the Light struct in GLSL.
struct alignas(16) Light {
    enum Type : int {
        PointLight = 0,
        DirectionalLight = 1,
        SpotLight = 2
    };
    glm::vec3 pos = glm::vec3(0.0f);     // Worldspace light position. Point/spot only
    Type type = PointLight;              // Light type
    glm::vec3 col = glm::vec3(1.0f);     // Light color (RGB) multiplied by intensity (so >1 is allowed).
    float falloff = 1;                   // Attenuation factor for distance-based falloff. 0 = infinite range, 1 = normal inverse square falloff. Point/spot only.
    glm::vec3 dir = glm::vec3(0, 0, -1); // Light direction (must be normalized). Directional/spot only.
    float phi = 0.9238795f;              // Cosine of spotlight cutoff angle (angle between direction and edge of light cone). Spot only.

    // Constructor helpers limited to the relevant arguments for each type.
    static Light point(glm::vec3 pos, glm::vec3 col = glm::vec3(1.0f), float falloff = 1.0f) {
        return Light{ pos, PointLight, col, falloff };
    }
    static Light directional(glm::vec3 dir, glm::vec3 col = glm::vec3(1.0f)) {
        return Light{ glm::vec3(0), DirectionalLight, col, 0, glm::normalize(dir) };
    }
    static Light spotlight(glm::vec3 pos, glm::vec3 dir, float open_angle_deg = 45, glm::vec3 col = glm::vec3(1.0f), float falloff = 1.0f) {
        return Light{ pos, SpotLight, col, falloff, glm::normalize(dir), glm::cos(0.5f * glm::radians(open_angle_deg)) };
    }
};
static_assert(sizeof(Light) == 48);
static_assert(offsetof(Light, pos) == 0);
static_assert(offsetof(Light, type) == 12);
static_assert(offsetof(Light, col) == 16);
static_assert(offsetof(Light, falloff) == 28);
static_assert(offsetof(Light, dir) == 32);
static_assert(offsetof(Light, phi) == 44);


// Wrapper for a uniform buffer object (UBO) containing an array of lights.
class LightsUniformBuffer {
public:
    std::vector<Light> lights;
    const unsigned int max_lights = 0;
    const GLuint binding_index = 0;
    CG::BufferResource ubo;
    static constexpr size_t header_size = 16;

    LightsUniformBuffer(unsigned int max_lights, GLuint binding_index)
        : max_lights(max_lights), binding_index(binding_index)
    {
        lights.reserve(max_lights);
        glGenBuffers(1, &ubo.handle);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, header_size + sizeof(Light) * max_lights, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void upload() {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        unsigned int n_lights = glm::min<unsigned int>(max_lights, lights.size());
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(n_lights), &n_lights);
        if (n_lights > 0)
            glBufferSubData(GL_UNIFORM_BUFFER, header_size, sizeof(Light) * n_lights, lights.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
};