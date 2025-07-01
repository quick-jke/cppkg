#ifndef QUICK_CPPKG_BUILD_HPP
#define QUICK_CPPKG_BUILD_HPP

#include "command.hpp"
#include "dependency.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include "nlohmann/json.hpp"

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
            }
        }
        
        std::cout << "🚀 Starting build process...\n";

        if (!checkProjectStructure()) {
            std::cerr << "❌ Project structure is invalid. Missing required files/directories.\n";
            return;
        }

        std::string compiler = findCompiler();
        if (compiler.empty()) {
            showCompilerInstallInstructions();
            return;
        }

        std::string build_cmd = buildCompilationCommand(compiler, config_["name"].get<std::string>(), config_["exec"].get<std::string>(), getDependencies());
        
        std::cout << "🔧 Running build command: " << build_cmd << "\n";
        
        int result = system(build_cmd.c_str());
        if (result == 0) {
            std::cout << "✅ Build successful! Executable created in build/ directory\n";
        } else {
            std::cerr << "❌ Build failed with error code: " << result << "\n";
        }
    }

private:
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

    std::string buildCompilationCommand(
        const std::string& compiler,
        const std::string& name,
        const std::string& exec,
        const std::vector<Dependency>& dependencies
    ) {
        std::string cmd;

        if (compiler == "cl") {
            // Windows (MSVC)
            cmd = compiler + 
                " /std:c++17" +
                " /EHsc";

            // Добавляем пути к заголовкам
            for (const auto& dep : dependencies) {
                if (!dep.include_path.empty()) {
                    cmd += " /I" + dep.include_path;
                }
            }

            cmd += " /Fobuild/ /Febuild/" + name + " " + exec;

            // Добавляем статические библиотеки
            for (const auto& dep : dependencies) {
                if (dep.type == "static" && !dep.library_path.empty()) {
                    cmd += " " + dep.library_path;
                }
            }

        } else {
            // Linux/macOS (g++, clang++)
            cmd = compiler + 
                " -std=c++17" +
                " -Iinclude";

            // Добавляем пути к заголовкам
            for (const auto& dep : dependencies) {
                if (!dep.include_path.empty()) {
                    cmd += " -I" + dep.include_path;
                }
            }

            cmd += " -o build/" + name + " " + exec;

            // Добавляем линковку статических библиотек
            for (const auto& dep : dependencies) {
                if (dep.type == "static" && !dep.library_path.empty()) {
                    cmd += " " + dep.library_path;
                }
            }
        }

        return cmd;
    }


    

    void showCompilerInstallInstructions() {
        std::cerr << "❌ Compiler not found. Please install one of the following:\n";
        
        #ifdef _WIN32
            std::cerr << "1. Visual Studio Build Tools (https://visualstudio.microsoft.com/visual-cpp-build-tools/ )\n";
            std::cerr << "2. MinGW-w64 (https://mingw-w64.org/doku.php )\n";
        #elif __linux__
            std::cerr << "1. GCC: sudo apt install g++\n";
            std::cerr << "2. Clang: sudo apt install clang\n";
        #elif __APPLE__
            std::cerr << "1. Xcode Command Line Tools: xcode-select --install\n";
        #endif
    }

    json config_;



};

#endif
