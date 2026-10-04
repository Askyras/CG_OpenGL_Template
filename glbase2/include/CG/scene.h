#pragma once

#include "CG/transform.h"

#include <format>
#include <memory>
#include <optional>
#include <string>

#include "glm/mat4x4.hpp"
#include "imgui/imgui.h"


namespace CG
{
    // Camera parameters passed down the scene hierarchy during rendering.
    struct CameraData {
        glm::mat4 projection; // Camera projection matrix
        glm::mat4 view;       // World-to-camera view matrix
        glm::vec3 position;   // Worldspace camera position
        glm::vec2 viewport;   // Application viewport size in pixels
    };

    class SceneHierarchy; // forward

    // Empty base class for an object in the scene hierarchy.
    // Subclass this to override initialisation or rendering behavior.
    // Simple update logic can be implemented without subclassing by setting the on_update callback.
    class SceneObject {
        friend class SceneHierarchy;
    public:
        const std::string name;                                        // Name, for debugging and GUI
        Transform tf;                                                  // Local transform relative to parent
        glm::mat4 world_matrix = glm::mat4(1);                         // Complete world transform including inherited parent transforms. Recalculated after each update().
        std::function<void(SceneObject&, double)> on_update = nullptr; // Custom update callback for simple update logic without subclassing. Called after the actual update() method, and before children are updated.

    protected:
        SceneObject* parent = nullptr;
        std::vector<std::unique_ptr<SceneObject>> children;

    public:
        SceneObject(const std::string& name = "Unnamed")
            : name(name) {}

        SceneObject* add_child(std::unique_ptr<SceneObject>&& child) {
            child->parent = this;
            return children.emplace_back(std::move(child)).get();
        }

        std::unique_ptr<SceneObject> remove_child(int idx) {
            if (idx < 0 || idx >= children.size())
                return nullptr;
            std::unique_ptr<SceneObject> removed = std::move(children[idx]);
            removed->parent = nullptr;
            children.erase(children.begin() + idx);
            return removed;
        }

        virtual void init() {}

        virtual void update(double dt) {}

        virtual void render(const CameraData& cam) {}

        virtual void inspector_gui() {}

        std::optional<SceneObject*> get_parent() const {
            return parent ? std::optional<SceneObject*>(parent) : std::nullopt;
        }
        const std::vector<std::unique_ptr<SceneObject>>& get_children() const {
            return children;
        }

        // Returns the world position of this object including any inherited parent transforms.
        glm::vec3 get_world_position() const {
            return glm::vec3(world_matrix[3]);
        }
        // Returns the (unnormalized) world-space direction of this object's local +X axis, after any parent transforms.
        // Note: non-uniform scaling with rotation in parents introduces skew, causing these axes to no longer be orthogonal
        glm::vec3 get_world_x_vector() const {
            return glm::vec3(world_matrix[0]);
        }
        // Returns the (unnormalized) world-space direction of this object's local +Y axis, after any parent transforms.
        // Note: non-uniform scaling with rotation in parents introduces skew, causing these axes to no longer be orthogonal
        glm::vec3 get_world_y_vector() const {
            return glm::vec3(world_matrix[1]);
        }
        // Returns the (unnormalized) world-space direction of this object's local +Z axis, after any parent transforms.
        // Note: non-uniform scaling with rotation in parents introduces skew, causing these axes to no longer be orthogonal
        glm::vec3 get_world_z_vector() const {
            return glm::vec3(world_matrix[2]);
        }
        // Returns the scale along this object's local axes, after any parent transforms.
        // Note: non-uniform scaling with rotation in parents introduces skew, which is not represented by this scale.
        glm::vec3 get_axis_scales() const {
            return glm::vec3(
                glm::length(glm::vec3(world_matrix[0])),
                glm::length(glm::vec3(world_matrix[1])),
                glm::length(glm::vec3(world_matrix[2]))
            );
        }

    private:
        void _init() {
            init();
            for (auto& child : children)
                child->_init();
        }

        void _update(double dt) {
            update(dt);
            if (on_update)
                on_update(*this, dt);
            world_matrix = parent ? (parent->world_matrix * tf.matrix()) : tf.matrix();
            for (auto& child : children)
                child->_update(dt);
        }

        void _render(const CameraData& cam) {
            render(cam);
            for (auto& child : children)
                child->_render(cam);
        }

        void _inspector_gui() {
            inspector_gui();
            for (int i = 0; i < children.size(); i++) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                ImGui::PushID(i);
                if (ImGui::TreeNode(std::format("{}: {}", i, children[i]->name).c_str())) {
                    children[i]->_inspector_gui();
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
    };


    // Class holding the entire scene hierarchy.
    // This is the entry point for init, update, render, and gui calls, which are propagated depth-first down the hierarchy.
    class SceneHierarchy {
    public:
        std::vector<std::unique_ptr<SceneObject>> objects;

        template<class T> requires std::derived_from<T, SceneObject>
        T* add_child(std::unique_ptr<T>&& child) {
            T* result = child.get();
            std::unique_ptr<SceneObject> base_ptr = std::move(child);
            objects.emplace_back(std::move(base_ptr));
            return result;
        }

        void init() {
            for (auto& obj : objects)
                obj->_init();
        }

        void update(double dt) {
            for (auto& obj : objects)
                obj->_update(dt);
        }

        void render(const CameraData& cam) {
            for (auto& obj : objects)
                obj->_render(cam);
        }

        void inspector_gui() {
            for (int i = 0; i < objects.size(); i++) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                ImGui::PushID(i);
                if (ImGui::TreeNode(std::format("{}: {}", i, objects[i]->name).c_str())) {
                    objects[i]->_inspector_gui();
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
    };
}