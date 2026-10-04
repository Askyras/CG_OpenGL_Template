#pragma once

#include "CG/math.h"

#define GLM_FORCE_RADIANS
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/transform.hpp"


namespace CG
{
	// Describes a 3D translate/rotate/scale transformation in an editable form.
	// Can be converted to a 4x4 matrix for concatenation with other transforms.
	struct Transform {
		Transform() {}
		Transform(glm::vec3 translation)
			: translation(translation) {
		}
		Transform(glm::vec3 scale, glm::vec3 rotation, glm::vec3 translation)
			: scale(scale), rotation(rotation), translation(translation) {
		}
		// for 2D transforms
		Transform(glm::vec2 scale, float rotation, glm::vec2 translation)
			: scale({ scale, 1 }), rotation({ 0, 0, rotation }), translation({ translation, 0 }) {
		}

		glm::mat4 matrix() const {
			return glm::translate(translation) * glm::mat4_cast(glm::quat(rotation)) * glm::scale(scale);
		}

		glm::vec3 scale = glm::vec3(1);
		glm::vec3 rotation = glm::vec3(0);
		glm::vec3 translation = glm::vec3(0);

		// Calculates the -Z direction resulting from this transform.
		inline glm::vec3 get_forward_vec() const {
			return glm::mat4_cast(glm::quat(rotation)) * glm::vec4(0, 0, -1, 0);
		}

		// Sets the rotation of this transform to orient the -Z axis in the given direction, with the up vector as a reference for roll.
		void rotate_to(glm::vec3 dir, glm::vec3 up = glm::vec3(0, 1, 0)) {
			dir = glm::normalize(dir);
			up = glm::normalize(up);
			glm::quat q = glm::quatLookAtRH(dir, up);
			rotation = glm::eulerAngles(q);
		}
	};
}