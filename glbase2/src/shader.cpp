#include "CG/shader.h"
#include "CG/utils.h"

#include <cstdio>
#include <format>
#include <string_view>


CG::ShaderPreprocessor CG::G_shader_preprocessor;


static std::string loc_string(const std::source_location loc)
{
    return std::format("{}:{}", loc.file_name(), loc.line());
}

static void file_dump(bool success, const std::filesystem::path& out_path, const std::string& output)
{
	std::filesystem::create_directories(out_path.parent_path());
	std::ofstream file(out_path);
	if (!file.is_open()) {
		printf("Failed to open shader preprocessor log file: '%s'\n", out_path.string().c_str());
		return;
	}
	file << "/* Shader preprocessor log\n";
	file << "* Preprocessing succeeded: " << (success ? "true" : "false") << "\n";
	file << "* Errors:\n" << (CG::G_shader_preprocessor.errors.empty() ? "None" : CG::G_shader_preprocessor.errors) << "\n";
	file << "* Preprocessed source: */\n\n" << output << "\n";
	file.close();
}

static bool validate_shader(GLuint handle, const char* name, const std::source_location loc)
{
	char* log;
	GLint compile_success, log_len;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_success);
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		log = new char[log_len];
		glGetShaderInfoLog(handle, log_len, NULL, log);

		if (compile_success == GL_TRUE) {
			printf("Shader compilation for '%s' succeded with warning:\n\t%s\n%s\n", name, loc_string(loc).c_str(), log);
		}
		else {
			printf("Shader compilation for '%s' failed with error:\n\t%s\n%s\n", name, loc_string(loc).c_str(), log);
		}
		delete[] log;
	}
	if (!compile_success) {
		glDeleteShader(handle);
		handle = 0;
	}
	return compile_success;
}

static bool validate_program(GLuint handle, const char* name, const std::source_location loc)
{
	char* log;
	GLint link_success, log_len;
	glGetProgramiv(handle, GL_LINK_STATUS, &link_success);
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		log = new char[log_len];
		glGetProgramInfoLog(handle, log_len, NULL, log);
		if (link_success == GL_TRUE)
			printf("Shader program linking for '%s' succeded with warning:\n\t%s\n%s\n", name, loc_string(loc).c_str(), log);
		else
			printf("Shader program linking for '%s' failed with error:\n\t%s\n%s\n", name, loc_string(loc).c_str(), log);
		delete[] log;
	}
	if (!link_success) {
		glDeleteProgram(handle);
		handle = 0;
	}
	return link_success;
}


static bool match(const char* str, const char* pattern, const char* end)
{
    while (str < end && *pattern != '\0') {
        if (*str != *pattern)
            return false;
        str++;
        pattern++;
    }
    return true;
}

static const char* seek(const char* str, const char* pattern, const char* end)
{
    const char* c = str;
    while (c < end) {
        if (match(c, pattern, end))
            return c;
        c++;
    }
    return end;
}


bool CG::ShaderPreprocessor::preprocess_shader_source(const std::string& src, std::string& out, ShaderDefines defines)
{
	errors.clear();
	include_stack.clear();
	inserted_includes.clear();

    bool success = true;
	std::string intermediate;
	intermediate.reserve(gl_version_str.size() + 1 + src.size());
	intermediate += gl_version_str;
	intermediate += '\n';
	if (!defines.empty()) {
		success &= append_defines(intermediate, defines);
	}
    intermediate += src;
	success &= resolve_includes(intermediate, out);
	return success;
}

bool CG::ShaderPreprocessor::resolve_includes(const std::string& src, std::string& out)
{
	out.clear();
	out.reserve(src.size());

	bool success = true;
	const char* next = src.data();
	const char* end = src.data() + src.size();
	const char* block_start = src.data();
	bool is_line_start = true;
	while (next < end) {
		if (*next == '\n') {
			is_line_start = true;
			next++;
			continue;
		}
		if (match(next, "//", end)) {
			// Skip line comment
			next = seek(next, "\n", end);
			continue;
		}
		if (match(next, "/*", end)) {
			// Skip block comment
			next = seek(next, "*/", end) + 2;
			is_line_start = false;
			continue;
		}
		if (is_line_start && match(next, "#include", end)) {
			// Include directive
			out += std::string_view(block_start, next - block_start);
			auto line_end = seek(next, "\n", end);
			auto open_quote = seek(next, "\"", line_end);
			auto close_quote = seek(open_quote + 1, "\"", line_end);
			if (close_quote >= line_end || open_quote + 1 >= close_quote) {
				errors += std::format("Invalid #include directive: {}\n", std::string_view(next, line_end - next));
				success = false;
				block_start = next = line_end;
				continue;
			}
			block_start = next = line_end;
			std::string include_path(open_quote + 1, close_quote - open_quote - 1);

			if (std::find(include_stack.begin(), include_stack.end(), include_path) != include_stack.end()) {
				errors += std::format("Circular include detected: '{}'\n", include_path);
				success = false;
				continue;
			}
			if (inserted_includes.contains(include_path)) {
				// already included
				continue;
			}
			if (!cached_includes.contains(include_path)) {
				if (!load_include_file(include_path)) {
					errors += std::format("Failed to load include file: '{}'\n", include_path);
					success = false;
				}
			}
			inserted_includes.insert(include_path);

			// DFS recurse includes
			std::string resolved_include_src;
			auto size_before = include_stack.size();
			include_stack.push_back(include_path);
			success &= resolve_includes(cached_includes[include_path], resolved_include_src);
			include_stack.resize(size_before);

			out += resolved_include_src;
			continue;
		}
		is_line_start = false;
		next++;
	}
	if (block_start < end) {
		out += std::string_view(block_start, end - block_start);
	}
	return success;
}

static bool check_define_string(const std::string& str)
{
    if (str.size() < 2 || str[0] != '#' || (str[1] != '_' && !std::isalpha(str[1]))) {
        return false;
    }
    const char* c = str.data() + 1;
    const char* end = str.data() + str.size();
    while (c < end && (*c == '_' || std::isalnum(*c))) {
        c++;
    }
	return c == end;
}

bool CG::ShaderPreprocessor::append_defines(std::string& src, ShaderDefines defines)
{
	bool success = true;
	for (const auto& [k, v] : defines) {
		if (!check_define_string(k)) {
			errors += std::format("Invalid define key: '{}'. It must consist of a '#' followed by a valid identifier.\n", k);
			success = false;
			continue;
		}
		src += "#define ";
		src += std::string_view(k.data() + 1, k.size() - 1);
		src += " ";
		src += substitution_string(v);
		src += '\n';
	}
	return success;
}

bool CG::ShaderPreprocessor::load_include_file(const std::string& filename)
{
	return read_resource_file(filename, cached_includes[filename]);
}

std::string CG::ShaderPreprocessor::substitution_string(const ShaderDefineValue& value)
{
	return std::visit([](const auto& v) -> std::string {
		using T = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<T, std::string>) {
			return v;
		}
		else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double> || std::is_same_v<T, float>) {
			return std::to_string(v);
		}
		else if constexpr (std::is_same_v<T, unsigned int>) {
			return std::format("{}u", v);
		}
		else if constexpr (std::is_same_v<T, bool>) {
			return v ? "true" : "false";
		}
		else if constexpr (std::is_same_v<T, glm::vec2>) {
			return std::format("vec2({},{})", v.x, v.y);
		}
		else if constexpr (std::is_same_v<T, glm::vec3>) {
			return std::format("vec3({},{},{})", v.x, v.y, v.z);
		}
		else if constexpr (std::is_same_v<T, glm::vec4>) {
			return std::format("vec4({},{},{},{})", v.x, v.y, v.z, v.w);
		}
		else {
			return std::to_string(v);
		}
		}, value);
}


CG::Shader::Shader(GLenum type, const std::string& resource_path, ShaderDefines defines, const std::source_location loc)
{
	std::string src;
	if (!read_resource_file(resource_path, src))
		return;

	std::string preprocessed;
    bool success = G_shader_preprocessor.preprocess_shader_source(src, preprocessed, defines);
	if (!CG::G_shader_preprocessor.errors.empty()) {
		if (success)
			printf("Shader preprocessing for '%s' succeded with warning:\n\t%s\n%s\n", resource_path.c_str(), loc_string(loc).c_str(), CG::G_shader_preprocessor.errors.c_str());
		else
			printf("Shader preprocessing for '%s' failed with error:\n\t%s\n%s\n", resource_path.c_str(), loc_string(loc).c_str(), CG::G_shader_preprocessor.errors.c_str());
	}
	if (!CG::G_shader_preprocessor.log_folder.empty()) {
		file_dump(success, CG::G_shader_preprocessor.log_folder + "/" + resource_path, preprocessed);
	}
	if (success) {
		// OpenGL shader compilation:
		handle = glCreateShader(type);
		const GLchar* glsrc = preprocessed.c_str();
		glShaderSource(handle, 1, &glsrc, NULL);
		glCompileShader(handle);
		validate_shader(handle, resource_path.c_str(), loc);
	}
}

CG::Shader::Shader(Shader&& other) noexcept
	: handle(other.handle)
{
	other.handle = 0;
}

CG::Shader& CG::Shader::operator=(Shader&& other) noexcept
{
	if (this != &other) {
		if (handle != 0)
			glDeleteShader(handle);
		handle = other.handle;
		other.handle = 0;
	}
	return *this;
}

CG::Shader::~Shader()
{
	if (handle != 0) {
		glDeleteShader(handle);
		handle = 0;
	}
}


CG::ShaderProgram::ShaderProgram(const Shader& vs, const Shader& fs, const char* dbg_name, const std::source_location loc)
{
	handle = glCreateProgram();
	glAttachShader(handle, vs.handle);
	glAttachShader(handle, fs.handle);
	glLinkProgram(handle);
	validate_program(handle, dbg_name, loc);
}

CG::ShaderProgram::ShaderProgram(const Shader& vs, const Shader& gs, const Shader& fs, const char* dbg_name, const std::source_location loc)
{
	handle = glCreateProgram();
	glAttachShader(handle, vs.handle);
	glAttachShader(handle, gs.handle);
	glAttachShader(handle, fs.handle);
	glLinkProgram(handle);
	validate_program(handle, dbg_name, loc);
}

CG::ShaderProgram::ShaderProgram(const Shader& cs, const char* dbg_name, const std::source_location loc)
{
	handle = glCreateProgram();
	glAttachShader(handle, cs.handle);
	glLinkProgram(handle);
	validate_program(handle, dbg_name, loc);
}

CG::ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
	: handle(other.handle)
{
	other.handle = 0;
}

CG::ShaderProgram& CG::ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
	if (this != &other) {
		if (handle != 0)
			glDeleteProgram(handle);
		handle = other.handle;
		other.handle = 0;
	}
	return *this;
}

CG::ShaderProgram::~ShaderProgram()
{
	if (handle != 0) {
		glDeleteProgram(handle);
		handle = 0;
	}
}
