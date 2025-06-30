#ifndef QUICK_CPPKG_INIT_HPP
#define QUICK_CPPKG_INIT_HPP
#include "command.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <unordered_map>

namespace fs = std::filesystem;

struct ProjectConfig {
    std::string name;
    std::string cpp_version;
    std::string exec_path;
};

class InitHandler : public CommandHandler {
public:
    InitHandler(const std::optional<std::string>& project_name, const std::string& cpp_version) : cpp_version_(normalizeCppVersion(cpp_version)) {
        if(project_name.has_value()){
            project_name_ = project_name.value();
        }
    }

    void execute() override {
        if (cpp_version_.empty()) {
            std::cerr << "Error: Unsupported C++ version '" << cpp_version_
                    << "'. Supported versions: 11, 14, 17, 20, 23\n";
            return;
        }

        std::string project_dir;
        std::string effective_project_name;

        if (!project_name_.empty()) {
            project_dir = project_name_;
            effective_project_name = project_name_;
            try {
                fs::create_directory(project_dir);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error creating directory: " << e.what() << "\n";
                return;
            }
        } else {
            
            project_dir = ".";
            effective_project_name = fs::current_path().filename().string();
            std::cout << effective_project_name << std::endl;
        }
        if (!createProjectStructure(project_dir, {effective_project_name, cpp_version_})) {
            std::cerr << "Error: Failed to create project structure\n";
            return;
        }

        std::cout << "Initialized project '" << effective_project_name 
                << "' with " << cpp_version_ << " standard\n";
    }

private:
    std::string project_name_;
    std::string cpp_version_;

    bool createProjectStructure(const std::string& dir, const ProjectConfig& config) {
        try {
            fs::create_directory(dir + "/src");
            fs::create_directory(dir + "/include");
            
            if (!createGitignore(dir) || 
                !createCppkgJson(dir, config) ||
                !createMainCpp(dir, config) ||
                !createMainHpp(dir, config)){
                return false;
            } 
                    
            
            return true;
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << "\n";
            return false;
        }
        catch (...) {
            std::cerr << "Unknown error during initialization\n";
            return false;
        }
    }

    std::string normalizeCppVersion(const std::string& version) {
        static const std::unordered_map<std::string, std::string> cpp_versions = {
            {"11", "c++11"}, {"14", "c++14"}, {"17", "c++17"}, 
            {"20", "c++20"}, {"23", "c++23"}, {"cpp11", "c++11"},
            {"cpp14", "c++14"}, {"cpp17", "c++17"}, {"cpp20", "c++20"}, 
            {"cpp23", "c++23"}
        };

        auto it = cpp_versions.find(version);
        return (it != cpp_versions.end()) ? it->second : "";
    }


    bool createGitignore(const std::string& dir) {
        std::ofstream gitignore(dir + "/.gitignore");
        if (!gitignore) return false;
        
        gitignore << "*.o\n*.log\ncmake-build*/\nbuild/\n";
        return true;
    }

    bool createCppkgJson(const std::string& dir, const ProjectConfig& config) {
        std::ofstream json(dir + "/cppkg.json");
        if (!json) return false;
        
        json << "{\n"
             << "  \"name\": \"" << config.name << "\",\n"
             << "  \"version\": \"0.1.0\",\n"
             << "  \"cpp_version\": \"" << config.cpp_version << "\",\n"
             << "  \"exec\": \"src/main.cpp\",\n"
             << "  \"dependencies\": {}\n"
             << "}";
        return true;
    }

    bool createMainCpp(const std::string& dir, const ProjectConfig& config) {
        std::ofstream main(dir + "/src/main.cpp");
        if (!main) return false;
        
        main << "#include <iostream>\n\n"
             << "#include \"" << config.name << ".hpp\"\n\n"
             << "int main() {\n"
             << "    std::cout << \"Hello, World!\" << std::endl;\n"
             << "    return 0;\n"
             << "}\n";
        return true;
    }

    std::string toUpperCaseWithUnderscores(const std::string& input) {
        std::string result;
        result.reserve(input.size()); 
        
        for (char c : input) {
            if (c == '-') {
                result += '_';
            } else {
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
        }
        
        return result;
    }

    bool createMainHpp(const std::string& dir, const ProjectConfig& config) {
        std::ofstream header(dir + "/include/" + config.name + ".hpp");
        if (!header) return false;
        
        header << "#ifndef INCLUDE_" << toUpperCaseWithUnderscores(config.name) << "_HPP\n"
             << "#define INCLUDE_" << toUpperCaseWithUnderscores(config.name) << "_HPP\n\n\n"
             << "#endif";
        return true;
    }

};

#endif
