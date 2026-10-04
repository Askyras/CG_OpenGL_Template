#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>


namespace CG
{
    // Prepend CG_RESOURCE_PATH to the given relative resource path.
	inline std::optional<std::filesystem::path> resolve_resource_path(const std::string& resource_path)
	{
#ifndef CG_RESOURCE_PATH
        printf("CG_RESOURCE_PATH is not defined, cannot resolve resource paths\nSet the 'CG_RESOURCE_PATH' option in your project CMakeLists.txt\n");
		return std::nullopt;
#else
        try {
            std::string_view stripped{ resource_path };
            while (!stripped.empty() &&
                (stripped.front() == '/' || stripped.front() == '\\')) {
                stripped.remove_prefix(1);
            }
            std::filesystem::path relative_path{ resource_path };
            return std::filesystem::path(CG_RESOURCE_PATH) / relative_path;
        }
        catch (const std::exception& e) {
            printf("Failed to resolve resource path '%s': %s\n", resource_path.c_str(), e.what());
            return std::nullopt;
        }
#endif
	}

    inline bool read_resource_file(const std::string& resource_path, std::string& out)
    {
        auto path = resolve_resource_path(resource_path);
        if (!path)
            return false; // error already printed in resolve_resource_path
		auto stream = std::ifstream(*path);
		if (!stream.is_open()) {
			printf("Failed to open file: %s\n", path->string().c_str());
			return false;
		}
		out = std::string(
			std::istreambuf_iterator<char>(stream),
			std::istreambuf_iterator<char>());
		return true;
    }

	inline std::string read_resource_file(const std::string& path)
	{
		std::string out;
        return read_resource_file(path, out) ? out : "";
	}
}