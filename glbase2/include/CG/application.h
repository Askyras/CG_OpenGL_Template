#pragma once

#include "CG/input.h"

#include <cstdint>
#include "glm/vec2.hpp"
#include "SDL3/SDL.h"


namespace CG
{
	// Generic windowed application class.
    // Subclass this and override the virtual methods to add your own application logic.
	class Application {
	public:
		bool quit = false;
		SDL_Window* window;
		SDL_GLContext gl_context;
		Input input;
		glm::ivec2 viewport;

	public:
		Application(const char* title = "CG Template", int width = 1280, int height = 720);
		virtual ~Application();

		friend void run(Application& app);

        virtual void handle_event(const SDL_Event* event) {}

		virtual void update(double dt) {}

		virtual void render() {}

		// returns the time in seconds since application initialisation
		double get_time_s() const {
            return SDL_GetTicksNS() * 1e-9;
		}

        bool is_mouse_locked() const {
            return mouse_locked;
        }
		void lock_mouse();
		void unlock_mouse();
		
	private:
		virtual void init(const char* win_title, int win_width, int win_height);

		virtual bool new_frame();

		bool initialized = false;
		uint64_t last_frame_time_ns = 0;
		bool mouse_locked = false;
		glm::vec2 original_mouse_pos;
	};

    // Application entry point. This will block until the application is quit.
	void run(Application& app);
}