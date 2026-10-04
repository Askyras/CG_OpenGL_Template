#pragma once

#include "CG/application.h"
#include "CG/camera.h"
#include "CG/scene.h"

#include "lights.h"


class EditorApp : public CG::Application {
public:
	CG::SceneHierarchy scene;
	LightsUniformBuffer lights_buffer = LightsUniformBuffer(10, 0); // max 10 lights, binding index 0

	CG::FreeCam3D camera;
	bool mouse_locked = true;
	bool freeze_movement = false;

	EditorApp(const char* title = "3D Editor", int width = 1920, int height = 1080);

	void update(double dt) override;

	void render() override;

	void draw_gui();
};