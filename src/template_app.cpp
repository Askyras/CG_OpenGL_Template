#include "template_app.h"
#include "objects.h"

#include "CG/mesh.h"
#include "CG/geometries.h"

#include "GL/glew.h"

#include "imgui/imgui.h"


int main()
{
    auto app = TemplateApp("CG Template", 1920, 1080);
    CG::run(app);
}


TemplateApp::TemplateApp(const char* title, int width, int height)
    : Application(title, width, height)
{
    // Create scene objects
    auto obj = std::make_unique<MeshObject>();
    CG::MeshBuilder b;
    CG::geom_quad(b.positions, b.normals, b.texcoords, b.indices);
    obj->mesh = b.build();
    obj->tint_color = glm::vec4(1, 0, 0, 1);

    // Define scene hierarchy
    scene.add_child(std::move(obj));

    // Initialize scene objects
    scene.init();
}

void TemplateApp::update(double dt)
{
    // Update scene objects and camera
	camera.update(dt, this);
    scene.update(dt);
}

void TemplateApp::draw_gui()
{
    // Define the application GUI here
    ImGui::Begin("Gui window");

    static bool show_demo_window = false;
    ImGui::Checkbox("Show demo window", &show_demo_window);
    if (show_demo_window)
        ImGui::ShowDemoWindow();

    ImGui::Text("Camera pos: (%.1f, %.1f)", camera.tf.translation.x, camera.tf.translation.y);
    ImGui::Text("View height: %.1f, move speed: %.1f", camera.fovy, camera.move_speed);

    ImGui::End();
}

void TemplateApp::render()
{
    // Set up global OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewport.x, viewport.y);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClearDepth(0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Fill out camera data and render the scene hierarchy with it
    CG::CameraData cam;
    cam.view = camera.get_view();
    cam.projection = camera.get_projection();
    cam.position = camera.tf.translation;
    cam.viewport = glm::vec2(viewport);
    scene.render(cam);

	draw_gui();
}