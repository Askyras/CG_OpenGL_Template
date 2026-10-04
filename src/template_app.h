#pragma once

#include "CG/application.h"
#include "CG/camera.h"
#include "CG/scene.h"


class TemplateApp : public CG::Application {
public:
	CG::SceneHierarchy scene;

	CG::FreeCam2D camera;

	TemplateApp(const char* title = "CG Template", int width = 1280, int height = 720);

	void update(double dt) override;

	void render() override;

    void draw_gui();
};