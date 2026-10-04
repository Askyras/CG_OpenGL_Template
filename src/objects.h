#pragma once

#include "lights.h"

#include "CG/scene.h"
#include "CG/shader.h"
#include "CG/mesh.h"
#include "CG/texture.h"
#include "CG/math.h"
#include "CG/geometries.h"

#include <glm/gtc/type_ptr.hpp>

#include <memory>


class MeshObject : public CG::SceneObject {
public:
    std::shared_ptr<CG::Mesh> mesh;
    std::shared_ptr<CG::Texture> texture;
    glm::vec4 tint_color = glm::vec4(0); // pre-multiplied, so 0 tint does nothing
protected:
    std::shared_ptr<CG::ShaderProgram> shader;
    int debug_mode = 0;

public:
    MeshObject(const std::string& name = "Unnamed")
        : CG::SceneObject(name) {}

    void init() override
    {
        if (mesh && !texture && mesh->vbos[CG::BufferIndex_Colors] == 0 && tint_color.a == 0) {
            // assume it was forgotten
            tint_color.a = 1;
        }
        CG::ShaderDefines defines;
        if (mesh && mesh->vbos[CG::BufferIndex_Colors] != 0)
            defines["#VERTEX_COLOR"] = "";
        defines["#DEBUG"] = "";
        shader = std::make_shared<CG::ShaderProgram>(
            CG::Shader(GL_VERTEX_SHADER, "shaders/mesh.vert.glsl", defines),
            CG::Shader(GL_FRAGMENT_SHADER, "shaders/mesh.frag.glsl", defines),
            "mesh");
    }

    void render(const CG::CameraData& cam) override 
    {
        if (!mesh) {
            printf("MeshObject '%s' has no mesh set!\n", name.c_str());
            return;
        }
        glBindVertexArray(mesh->vao);
        glUseProgram(*shader);

        glUniform1i(glGetUniformLocation(*shader, "has_texture"), texture ? 1 : 0);
        if (texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(texture ? texture->target : GL_TEXTURE_2D, texture ? texture->tex_id : 0);
            glUniform1i(glGetUniformLocation(*shader, "tex"), 0);
        }

        glUniform4f(glGetUniformLocation(*shader, "color"), tint_color.r, tint_color.g, tint_color.b, tint_color.a);
        glUniform1i(glGetUniformLocation(*shader, "debug_mode"), (int)debug_mode);

        glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(world_matrix));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, glm::value_ptr(cam.view));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "proj"), 1, GL_FALSE, glm::value_ptr(cam.projection));

        if (mesh->vbos[CG::BufferIndex_Indices] != 0)
            glDrawElements(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mesh->draw_mode, 0, mesh->index_count);
    }

    void inspector_gui() override
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
        ImGui::Text("MeshObject");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit4("Tint Color", glm::value_ptr(tint_color));
        ImGui::SetNextItemWidth(200);
        ImGui::Combo("Debug view", (int*)&debug_mode, "None\0Normals\0Texcoords\0");
    }
};


class LitMeshObject : public MeshObject {
public:
    glm::vec3 ka = glm::vec3(0.1);
    glm::vec3 kd = glm::vec3(0.9);
    glm::vec3 ks = glm::vec3(1.0);
    inline static LightsUniformBuffer* lights_buffer;

    using MeshObject::MeshObject;

    void init() override
    {
        if (mesh && !texture && mesh->vbos[CG::BufferIndex_Colors] == 0 && tint_color.a == 0) {
            // assume it was forgotten
            tint_color.a = 1;
        }
        if (!lights_buffer) {
            printf("LitMeshObject requires a LightsUniformBuffer to be set before initialising!\n");
            return;
        }
        CG::ShaderDefines defines;
        defines["#MAX_LIGHTS"] = lights_buffer->max_lights;
        if (mesh && mesh->vbos[CG::BufferIndex_Colors] != 0)
            defines["#VERTEX_COLOR"] = "";
            if (texture)
                defines["#TEXTURED"] = "";
        defines["#DEBUG"] = "";
        shader = std::make_shared<CG::ShaderProgram>(
            CG::Shader(GL_VERTEX_SHADER, "shaders/mesh.vert.glsl", defines),
            CG::Shader(GL_FRAGMENT_SHADER, "shaders/mesh_lit.frag.glsl", defines),
            "lit mesh");
    }

    void render(const CG::CameraData& cam) override
    {
        if (!shader) return;
        if (!mesh) {
            printf("LitMeshObject '%s' has no mesh set!\n", name.c_str());
            return;
        }
        glBindVertexArray(mesh->vao);
        glUseProgram(*shader);

        glUniformBlockBinding(*shader, glGetUniformBlockIndex(*shader, "Lights"), lights_buffer->binding_index);

        glUniform1i(glGetUniformLocation(*shader, "has_texture"), texture ? 1 : 0);
        if (texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(texture ? texture->target : GL_TEXTURE_2D, texture ? texture->tex_id : 0);
            glUniform1i(glGetUniformLocation(*shader, "tex"), 0);
        }

        glUniform4f(glGetUniformLocation(*shader, "color"), tint_color.r, tint_color.g, tint_color.b, tint_color.a);
        glUniform1i(glGetUniformLocation(*shader, "debug_mode"), (int)debug_mode);
        glUniform3f(glGetUniformLocation(*shader, "ka"), ka.r, ka.g, ka.b);
        glUniform3f(glGetUniformLocation(*shader, "kd"), kd.r, kd.g, kd.b);
        glUniform3f(glGetUniformLocation(*shader, "ks"), ks.r, ks.g, ks.b);

        glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(world_matrix));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, glm::value_ptr(cam.view));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "proj"), 1, GL_FALSE, glm::value_ptr(cam.projection));
        glUniform3f(glGetUniformLocation(*shader, "cam_pos"), cam.position.x, cam.position.y, cam.position.z);

        if (mesh->vbos[CG::BufferIndex_Indices] != 0)
            glDrawElements(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mesh->draw_mode, 0, mesh->index_count);
    }

    void inspector_gui() override
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
        ImGui::Text("LitMeshObject");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit4("Tint Color", glm::value_ptr(tint_color));
        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit3("ka", glm::value_ptr(ka));
        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit3("kd", glm::value_ptr(kd));
        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit3("ks", glm::value_ptr(ks));
        ImGui::SetNextItemWidth(200);
        ImGui::Combo("Debug view", (int*)&debug_mode, "None\0Normals\0Texcoords\0Unlit\0");
    }
};


class BillboardObject : public MeshObject {
public:
    using MeshObject::MeshObject;

    void init() override
    {
        if (mesh && !texture && mesh->vbos[CG::BufferIndex_Colors] == 0 && tint_color.a == 0) {
            // assume it was forgotten
            tint_color.a = 1;
        }
        CG::ShaderDefines defines;
        if (mesh && mesh->vbos[CG::BufferIndex_Colors] != 0)
            defines["#VERTEX_COLOR"] = "";
        if (texture)
            defines["#TEXTURED"] = "";
        defines["#DEBUG"] = "";
        shader = std::make_shared<CG::ShaderProgram>(
            CG::Shader(GL_VERTEX_SHADER, "shaders/billboard.vert.glsl", defines),
            CG::Shader(GL_FRAGMENT_SHADER, "shaders/mesh.frag.glsl", defines),
            "billboard");
    }

    void render(const CG::CameraData& cam) override
    {
        if (!mesh) {
            printf("BillboardObject '%s' has no mesh set!\n", name.c_str());
            return;
        }
        glBindVertexArray(mesh->vao);
        glUseProgram(*shader);

        glUniform1i(glGetUniformLocation(*shader, "has_texture"), texture ? 1 : 0);
        if (texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(texture ? texture->target : GL_TEXTURE_2D, texture ? texture->tex_id : 0);
            glUniform1i(glGetUniformLocation(*shader, "tex"), 0);
        }

        glUniform4f(glGetUniformLocation(*shader, "color"), tint_color.r, tint_color.g, tint_color.b, tint_color.a);
        glUniform1i(glGetUniformLocation(*shader, "debug_mode"), (int)debug_mode);

        glUniformMatrix4fv(glGetUniformLocation(*shader, "model"), 1, GL_FALSE, glm::value_ptr(world_matrix));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "view"), 1, GL_FALSE, glm::value_ptr(cam.view));
        glUniformMatrix4fv(glGetUniformLocation(*shader, "proj"), 1, GL_FALSE, glm::value_ptr(cam.projection));

        if (mesh->vbos[CG::BufferIndex_Indices] != 0)
            glDrawElements(mesh->draw_mode, mesh->index_count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mesh->draw_mode, 0, mesh->index_count);
    }
};


class LightSource : public BillboardObject {
public:
    inline static LightsUniformBuffer* lights_buffer;
protected:
    using BillboardObject::mesh;
    using BillboardObject::texture;
    using BillboardObject::tint_color;

    inline static std::shared_ptr<CG::Texture> emitter_texture;
    inline static std::shared_ptr<CG::Mesh> quad_mesh;
    int light_index = -1; // index of this light in the LightsUniformBuffer, or -1 if not added yet

public:
    LightSource(Light new_light, const std::string& name = "Unnamed")
        : BillboardObject(name)
    {
        if (!lights_buffer) {
            printf("LightSource requires a LightsUniformBuffer to be set before creating!\n");
            return;
        }
        if (lights_buffer->lights.size() >= lights_buffer->max_lights) {
            printf("LightSource cannot be created because the LightsUniformBuffer is full\n");
            return;
        }
        tf.translation = new_light.pos;
        tf.rotate_to(new_light.dir);
        tf.scale = glm::vec3(0.05f); // 0.1 unit square
        tint_color = glm::vec4(new_light.col, 1.0f);
        light_index = lights_buffer->lights.size();
        lights_buffer->lights.push_back(new_light);
    }

    void init() override
    {
        if (!lights_buffer || light_index < 0)
            return;
        if (!emitter_texture) {
            emitter_texture = CG::load_texture("emitter.png");
        }
        if (!quad_mesh) {
            CG::MeshBuilder b;
            CG::geom_quad(b.positions, b.normals, b.texcoords, b.indices);
            quad_mesh = b.build();
        }
        texture = emitter_texture;
        mesh = quad_mesh;
        BillboardObject::init();
    }

    void render(const CG::CameraData& cam) override
    {
        if (!lights_buffer || light_index < 0 || light_index >= lights_buffer->lights.size())
            return;
        auto& light = lights_buffer->lights[light_index];
        light.pos = get_world_position();
        light.dir = -glm::normalize(get_world_z_vector());
        light.col = glm::vec3(tint_color);
        if (light.type == Light::DirectionalLight)
            return;
        BillboardObject::render(cam);
    }

    void inspector_gui() override
    {
        if (!lights_buffer || light_index < 0 || light_index >= lights_buffer->lights.size())
            return;

        auto& l = lights_buffer->lights[light_index];

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
        ImGui::Text("LightSource");
        ImGui::PopStyleColor();

        ImGui::SetNextItemWidth(200);
        ImGui::Combo("Type", (int*)&l.type, "Point light\0Directional light\0Spot light\0");

        ImGui::SetNextItemWidth(200);
        ImGui::ColorEdit3("Color", glm::value_ptr(tint_color));
        if (l.type != Light::DirectionalLight) {
            ImGui::SetNextItemWidth(200);
            ImGui::DragFloat("Falloff", &l.falloff, 0.01f, 0, 100);
            ImGui::SetNextItemWidth(200);
            ImGui::DragFloat3("pos", glm::value_ptr(tf.translation), 0.1f, -100, 100, "%.1f");
        }
        if (l.type != Light::PointLight) {
            ImGui::SetNextItemWidth(200);
            glm::vec3 degrees = glm::degrees(tf.rotation);
            if (ImGui::DragFloat3("rotation", glm::value_ptr(degrees), 0.1f, -180, 180, "%.1f"))
                tf.rotation = glm::radians(degrees);
        }
        if (l.type == Light::SpotLight) {
            float open_angle_deg = glm::degrees(2.0f * glm::acos(l.phi));
            ImGui::SetNextItemWidth(200);
            if (ImGui::DragFloat("Open angle", &open_angle_deg, 0.1f, 0.0f, 180.0f, "%.1f"))
                l.phi = glm::cos(0.5f * glm::radians(open_angle_deg));
        }
    }
};


class InfiniteGrid : public CG::SceneObject {
protected:
    CG::Mesh mesh;
    CG::ShaderProgram shader;

public:
    InfiniteGrid()
        : CG::SceneObject("InfiniteGrid") {
        glGenVertexArrays(1, &mesh.vao);
        mesh.index_count = 12;
    }

    void init() override
    {
        shader = CG::ShaderProgram(
            CG::Shader(GL_VERTEX_SHADER, "shaders/infinite_plane.vert.glsl"),
            CG::Shader(GL_FRAGMENT_SHADER, "shaders/infinite_grid.frag.glsl"),
            "grid shader");
    }

    void render(const CG::CameraData& cam) override
    {
        glBindVertexArray(mesh.vao);
        glUseProgram(shader);

        glm::vec3 pos_wrapped = glm::fmod(cam.position, glm::vec3(10.f));
        pos_wrapped = cam.position;
        glm::mat4 cam_view_wrapped = cam.view;
        //cam_view_wrapped[3] = glm::vec4(-pos_wrapped, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(cam_view_wrapped));
        glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, glm::value_ptr(cam.projection));
        glUniform2f(glGetUniformLocation(shader, "viewport"), cam.viewport.x, cam.viewport.y);
        glUniform3f(glGetUniformLocation(shader, "cam_pos"), pos_wrapped.x, pos_wrapped.y, pos_wrapped.z);

        glDrawArrays(GL_TRIANGLES, 0, mesh.index_count);
    }
};