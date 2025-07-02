#ifndef QUICK_CPPKG_BUILD_HPP
#define QUICK_CPPKG_BUILD_HPP

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define GREY    "\033[90m"

#include "command.hpp"
#include "dependency.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include <unordered_set>

namespace fs = std::filesystem;
using json = nlohmann::json;

class BuildHandler : public CommandHandler {
public:
    BuildHandler() : config_(json::parse(std::ifstream("cppkg.json"))){}
    void execute() override {
        if (!fs::exists("_packages")) {
            try {
                fs::create_directory("_packages");
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error: " << e.what() << '\n';
                return;
            }
        }

        std::cout << YELLOW << "🚀 Starting build process..." << RESET << std::endl;

        if (!checkProjectStructure()) {
            std::cerr << RED << "❌ Project structure is invalid. Missing required files/directories." << RESET << std::endl;
            return;
        }

        std::string build_cmd = buildCompilationCommand();

        std::cout << "🔧 Running build command:\n" << build_cmd << RESET << std::endl;

        int result = system(build_cmd.c_str());
        if (result == 0) {
            std::cout << GREEN << "✅ Build successful! Executable created in build/ directory" << RESET << std::endl;
        } else {
            std::cerr << RED << "❌ Build failed with error code: " << result << RESET << std::endl;
        }
    }

private:
    json config_;

    std::vector<std::string> findHeaderDirectories() {
        std::unordered_set<std::string> header_dirs;

        const std::unordered_set<std::string> header_extensions = { ".h", ".hpp", ".hh", ".hxx" };

        for (const auto& entry : fs::recursive_directory_iterator(".")) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();

                if (header_extensions.count(ext)) {
                    std::string dir = entry.path().parent_path().string();
                    header_dirs.insert(dir);
                }
            }
        }

        return {header_dirs.begin(), header_dirs.end()};
    }

    std::vector<std::string> getSourceFiles() {
        std::vector<std::string> sources;

        if (config_.contains("sources") && config_["sources"].is_array()) {
            for (const auto& s : config_["sources"]) {
                if (fs::exists(s.get<std::string>())) {
                    sources.push_back(s.get<std::string>());
                }
            }

        } else {
            for (const auto& entry : fs::recursive_directory_iterator(".")) {
                if (entry.path().extension() == ".cpp" &&
                    entry.path().parent_path() != "build" &&
                    entry.path().parent_path() != "_packages") {

                    sources.push_back(entry.path().string());
                }
            }
        }
        
        return sources;
    }
    
    bool checkProjectStructure() {
        if (!fs::exists(config_["exec"])) {
            std::cerr << "❌ Missing source file\n";
            return false;
        }
        
        if (!fs::exists("cppkg.json")) {
            std::cerr << "❌ Missing configuration file: cppkg.json\n";
            return false;
        }
        
        if (!fs::exists("build")) {
            fs::create_directory("build");
        }
        
        return true;
    }

    std::string findCompiler() {
        std::vector<std::string> possible_compilers;

        #ifdef _WIN32
            possible_compilers = {"clang++", "g++", "cl"};
        #else
            possible_compilers = {"g++", "clang++"};
        #endif

        for (const auto& compiler : possible_compilers) {
            #ifdef _WIN32
                std::string cmd = "where " + compiler + " >nul 2>&1";
            #else
                std::string cmd = "command -v " + compiler + " >/dev/null 2>&1";
            #endif
            
            if (system(cmd.c_str()) == 0) {
                return compiler;
            }
        }
        
        return "";
    }

    std::vector<Dependency> getDependencies() {
        return {
            {
                "fmt", "9.0.0",
                "_packages/fmt/9.0.0/include",
                "_packages/fmt/9.0.0/lib/libfmt.a",
                "static"
            }
        };
    }

    std::string buildCompilationCommand() {
        auto compiler = findCompiler();
        if (compiler.empty()) {
            throw std::runtime_error(getCompilerInstallInstructions());
        }
        auto source_files = getSourceFiles();
        if (source_files.empty()) {
            throw std::runtime_error(RED + std::string("❌ No source files found to compile") + RESET + "\n");
        }
        std::string cmd;
        auto cpp_version = config_["cpp_version"].get<std::string>();
        auto name = config_["name"].get<std::string>();
        std::vector<std::string> include_paths = findHeaderDirectories();
        auto dependencies = getDependencies();

        if (compiler == "cl") {
            // Windows (MSVC)
            cmd = compiler + 
                " /std:" + cpp_version +
                " /EHsc /nologo";

            for (const auto& path : include_paths) {
                cmd += " /I\"" + path + "\"";
            }

            for (const auto& dep : dependencies) {
                if (!dep.include_path.empty()) {
                    cmd += " /I\"" + dep.include_path + "\"";
                }
            }

            cmd += " /Fobuild/ /Febuild/" + name;

            for (const auto& src : source_files) {
                cmd += " \"" + src + "\"";
            }

            for (const auto& dep : dependencies) {
                if (dep.type == "static" && !dep.library_path.empty()) {
                    cmd += " \"" + dep.library_path + "\"";
                }
            }

        } else {
            // Linux/macOS (g++/clang++)
            cmd = compiler + 
                " -std=" + cpp_version +
                " -Wall -Wextra -pedantic";

            for (const auto& path : include_paths) {
                cmd += " -I\"" + path + "\"";
            }

            for (const auto& dep : dependencies) {
                if (!dep.include_path.empty()) {
                    cmd += " -I\"" + dep.include_path + "\"";
                }
            }

            cmd += " -o build/" + name;

            for (const auto& src : source_files) {
                cmd += " \"" + src + "\"";
            }

            for (const auto& dep : dependencies) {
                if ((dep.type == "static" || dep.type == "shared") && !dep.library_path.empty()) {
                    cmd += " \"" + dep.library_path + "\"";
                }
            }
        }

        return cmd;
    }

    std::string getCompilerInstallInstructions() {
        std::stringstream oss;
        oss << RED << "❌ Compiler not found. Please install one of the following:" << RESET << std::endl;
        
        #ifdef _WIN32
            oss << YELLOW << "1. Visual Studio Build Tools (https://visualstudio.microsoft.com/visual-cpp-build-tools/ )\n2. MinGW-w64 (https://mingw-w64.org/doku.php )" << RESET << std::endl;
        #elif __linux__
            oss << YELLOW << "1. GCC: sudo apt install g++\n2. Clang: sudo apt install clang" << RESET << std::endl;
        #elif __APPLE__
            oss << YELLOW << "1. Xcode Command Line Tools: xcode-select --install" << RESET << std::endl;
        #endif
        return oss.str();
    }

    bool needsRebuild(const std::string& source, const std::string& object) {
        if (!fs::exists(object)) return true;

        auto src_time = fs::last_write_time(source);
        auto obj_time = fs::last_write_time(object);

        return src_time > obj_time;
    }
};

#endif
