#pragma once

#include "glm/vec2.hpp"
#include "SDL3/SDL.h"


namespace CG
{
	// Gathers some user input data into one place, so that it can be queried at any time during the frame, instead of just during event handling
	class Input {
	public:
		glm::vec2 mouse_pos{ 0, 0 };                      // Mouse position this frame
		glm::vec2 mouse_delta{ 0, 0 };                    // Mouse movement since last frame
		SDL_MouseButtonFlags mouse_clicked = 0;           // Mouse buttons newly pressed this frame
        int mouse_scroll = 0;                             // Scrollwheel "ticks" scrolled this frame
		bool key_pressed[SDL_SCANCODE_COUNT] = { false }; // Keyboard scancodes newly pressed this frame

		void clear() {
			mouse_delta = { 0, 0 };
			mouse_clicked = 0;
			mouse_scroll = 0;
			for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
				key_pressed[i] = false;
			}
		}
	};
}