#include "editor_example.h"

#include "CG/math.h"
#include "CG/geometries.h"
#include "objects.h"

#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

#include <format>


int main()
{
    auto app = EditorApp();
    CG::run(app);
}


EditorApp::EditorApp(const char* title, int width, int height)
    : Application(title, width, height)
{
    LitMeshObject::lights_buffer = &lights_buffer;
    LightSource::lights_buffer = &lights_buffer;

    CG::MeshBuilder b;

    camera.tf.translation = glm::vec3(0, 1, 2);

    auto uv_cube = std::make_unique<LitMeshObject>("uv_cube");
    CG::geom_cube(b.positions, b.normals, b.texcoords, b.indices);
    uv_cube->mesh = b.build();
    uv_cube->texture = CG::load_texture("uv_checker.png", CG::TextureFlags_Default, GL_TEXTURE_2D);
    uv_cube->tf.translation = glm::vec3(.5, .5, -.5);
    uv_cube->tf.scale = glm::vec3(0.5f);

    auto teapot = std::make_unique<LitMeshObject>("teapot");
    CG::geom_teapot(b.positions, b.normals, b.texcoords, b.indices);
    teapot->mesh = b.build();
    teapot->tint_color = glm::vec4(0.71f, 0.63f, 0.61f, 1);
    teapot->tf.translation = glm::vec3(.5, 1.46, -.5);

    auto orbit_attachment = std::make_unique<CG::SceneObject>("moon_orbit");
    orbit_attachment->tf.rotation.z = glm::radians(20.f);
    orbit_attachment->on_update = [&](CG::SceneObject& self, double dt) {
        if (!freeze_movement) self.tf.rotation.y += 0.25 * PI * dt;
        };

    auto moon = std::make_unique<LitMeshObject>("moon");
    CG::geom_sphere(b.positions, b.normals, b.texcoords, b.indices);
    moon->mesh = b.build();
    moon->texture = CG::load_texture("luna.png", CG::TextureFlags_Default, GL_TEXTURE_2D);
    moon->tf.translation = glm::vec3(1.5, 0, 0);
    moon->tf.scale = glm::vec3(0.2f);
    moon->on_update = [&](CG::SceneObject& self, double dt) {
        if (!freeze_movement) self.tf.rotation.y -= 0.5 * PI * dt;
        };
    moon->ka = glm::vec3(0);
    moon->kd = glm::vec3(1);
    moon->ks = glm::vec3(0.2);

    auto moons_light = std::make_unique<LightSource>(
        Light::point(glm::vec3(0, 0, -2), glm::vec3(0, .5, 1)), "moons_light");
    moons_light->tf.scale /= moon->tf.scale;

    auto ground_plane = std::make_unique<LitMeshObject>("ground_plane");
    CG::geom_disk(b.positions, b.normals, b.texcoords, b.indices, 0, 64);
    ground_plane->mesh = b.build();
    ground_plane->tint_color = glm::vec4(0.1f, 0.1f, 0.1f, 1);
    ground_plane->ks = glm::vec3(0.2);
    ground_plane->tf.translation.y = -0.01;
    ground_plane->tf.rotation.x = glm::radians(-90.f);
    ground_plane->tf.scale = glm::vec3(10.0f);


    // Define scene hierarchy
    scene.add_child(std::move(uv_cube));

    moon->add_child(std::move(moons_light));
    orbit_attachment->add_child(std::move(moon));
    teapot->add_child(std::move(orbit_attachment));
    scene.add_child(std::move(teapot));

    scene.add_child(std::move(ground_plane));

    scene.add_child(std::make_unique<LightSource>(
        Light::directional(glm::vec3(-1, -2, 0)), "Directional"));
    scene.add_child(std::make_unique<LightSource>(
        Light::spotlight(glm::vec3(0, 1.5, 2), glm::normalize(glm::vec3(.2, -.2, -1)), 20, glm::vec3(1, 1, .5), 0.5), "Spotlight"));


    scene.add_child(std::make_unique<InfiniteGrid>());

    scene.init();
}

void EditorApp::update(double dt)
{
    if (input.key_pressed[SDL_SCANCODE_F]) {
        if (is_mouse_locked())
            unlock_mouse();
        else
            lock_mouse();
    }
    if (input.key_pressed[SDL_SCANCODE_ESCAPE] && is_mouse_locked()) {
        unlock_mouse();
    }

    bool camera_dragging = SDL_BUTTON_RMASK & SDL_GetMouseState(nullptr, nullptr);
    camera.update(dt, this, is_mouse_locked() || camera_dragging);

    scene.update(dt);
}

void EditorApp::draw_gui()
{
    ImGui::PushFont(nullptr, 16);

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Once);
    ImGui::Begin("Gui window");

    static bool show_demo_window = false;
    ImGui::Checkbox("Show demo window", &show_demo_window);
    if (show_demo_window)
        ImGui::ShowDemoWindow();


    ImGui::SeparatorText("Camera");

    ImGui::TextWrapped(is_mouse_locked() ? "Camera mouse control active, press [F]/[Esc] to leave" : "Camera mouse control inactive, press [F] or hold [RMB] to look around");
    ImGui::TextWrapped("[WASD], [Space], [Ctrl] to move, hold [Shift] to move faster");

    ImGui::Text("Pos: (%.1f, %.1f, %.1f)", camera.tf.translation.x, camera.tf.translation.y, camera.tf.translation.z);
    auto dir = camera.tf.get_forward_vec();
    int ax = glm::abs(dir.x) >= glm::abs(dir.z) ? 0 : 2;
    auto polar = CG::to_polar(dir);
    ImGui::Text("Facing: %.0f° (%s%s), %.0f° %s",
        glm::degrees(polar[2]), dir[ax] < 0 ? "-" : "+", ax == 0 ? "X" : "Z", glm::degrees(glm::abs(polar[1] - PI_2)), dir.y >= 0 ? "up" : "down");

    ImGui::Checkbox("Move axis aligned", &camera.move_axis_aligned);

    ImGui::DragFloat("Speed", &camera.move_speed, 0.1f, 0.1f, 50.f, "%.1f");


    ImGui::SeparatorText("Scene");

    ImGui::Checkbox("Freeze animated movements", &freeze_movement);
    static bool show_inspector = true;
    ImGui::Checkbox("Inspector gui", &show_inspector);

    ImGui::End();

    if (show_inspector) {
        ImGui::SetNextWindowPos(ImVec2(viewport.x, 0), ImGuiCond_Always, ImVec2(1, 0));
        ImGui::SetNextWindowSize(ImVec2(400, viewport.y), ImGuiCond_Always);
        ImGui::Begin("Inspector", &show_inspector, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        if (ImGui::CollapsingHeader("Scene Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {
            scene.inspector_gui();
        }
        ImGui::End();
    }

    ImGui::PopFont();
}

void EditorApp::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport.x, viewport.y);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClearDepth(0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER); // reverse-z camera projection
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // use pre-multiplied alpha blending
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    lights_buffer.upload();

    CG::CameraData cam;
    cam.view = camera.get_view();
    cam.projection = camera.get_projection();
    cam.position = camera.tf.translation;
    cam.viewport = glm::vec2(viewport);
    scene.render(cam);

    draw_gui();
}