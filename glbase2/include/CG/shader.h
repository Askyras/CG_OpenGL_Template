#pragma once

#include "GL/glew.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <source_location>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <variant>
#include <optional>


namespace CG
{
	using ShaderDefineValue = std::variant<std::string, int, unsigned int, double, float, bool, glm::vec2, glm::vec3, glm::vec4>;
	using ShaderDefines = std::unordered_map<std::string, ShaderDefineValue>; // {"#DEF", value} map. Resolves to "#define DEF value" (value is formatted based on type).

	// String processor for centralised versioning, #include resolution, and adding #defines at shader compilation time.
	class ShaderPreprocessor {
	public:
		std::string gl_version_str = "#version 410 core";    // First line prepended to all processed shader sources.
		std::string log_folder = "shader_preprocessor_logs"; // write all preprocessor output files to this folder (if not empty string) for debugging
		std::string errors;                                  // Readonly, error log output from the last processed shader.

		// Preprocess the given shader source string, prepending the version string and any given defines, and resolving #include paths
		bool preprocess_shader_source(const std::string& src, std::string& out, ShaderDefines defines = {});

		// Clear cache of loaded include files. Use when hot reloading shaders.
        void clear_cache() { cached_includes.clear(); }

	protected:
		bool append_defines(std::string& src, ShaderDefines defines);
		// Optionally prepend gl version and resolve `#include "path"` directives
		bool resolve_includes(const std::string& src, std::string& out);

		bool load_include_file(const std::string& filename);

		std::string substitution_string(const ShaderDefineValue& value);

		std::unordered_map<std::string, std::string> cached_includes;
		std::unordered_set<std::string> inserted_includes;
		std::vector<std::string> include_stack;
	};

	extern ShaderPreprocessor G_shader_preprocessor;


	// RAII wrapper for a single compiled shader stage (vert/frag/etc).
	// Handles move semantics and deletion, and the constructor with arguments creates and compiles the shader.
    // Use ShaderProgram to link multiple Shader objects into a complete shader program.
	class Shader {
	public:
		GLuint handle = 0;

		Shader() = default;
		// resource_path is relative to the resource directory
		Shader(GLenum type, const std::string& resource_path, ShaderDefines defines = {}, const std::source_location loc = std::source_location::current());
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&& other) noexcept;
		Shader& operator=(Shader&& other) noexcept;
		~Shader();

		operator GLuint() const { return handle; }
	};

	// RAII wrapper for a compiled and linked shader program.
    // Handles move semantics and deletion, and the construction by linking a set of Shader objects.
    // Can be used directly for GL calls in place of the underlying GLuint due to the implicit conversion operator.
	class ShaderProgram {
	public:
		GLuint handle = 0;

		ShaderProgram() = default;
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		ShaderProgram(ShaderProgram&& other) noexcept;
		ShaderProgram& operator=(ShaderProgram&& other) noexcept;
		~ShaderProgram();

		ShaderProgram(const Shader& vs, const Shader& fs, const char* dbg_name = "vs+fs", const std::source_location loc = std::source_location::current());
		ShaderProgram(const Shader& vs, const Shader& gs, const Shader& fs, const char* dbg_name = "vs+gs+fs", const std::source_location loc = std::source_location::current());
		ShaderProgram(const Shader& cs, const char* dbg_name = "cs", const std::source_location loc = std::source_location::current());

        operator GLuint() const { return handle; }
	};
}