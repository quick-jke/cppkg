#ifndef QUICK_CPPKG_BUILD_HPP
#define QUICK_CPPKG_BUILD_HPP

#include "command.hpp"
#include <iostream>
#include <filesystem>
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

        std::string build_cmd = buildCompilationCommand(compiler);
        
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

    std::string buildCompilationCommand(const std::string& compiler) {
        if (compiler == "cl") {
            // Windows (MSVC)
            return compiler + 
                  " /std:" + config_["cpp_version"].get<std::string>() +
                  " /EHsc /Iinclude /Fobuild/ /Febuild/" + config_["name"].get<std::string>() + " " + config_["exec"].get<std::string>();
        } else {
            // Linux/macOS (g++/clang++)
            std::string cmd = compiler + 
                            " -std=" + config_["cpp_version"].get<std::string>() +
                            " -Iinclude -o build/" + config_["name"].get<std::string>() + " " + config_["exec"].get<std::string>();
            
            auto dependencies = getDependencies();
            if (!dependencies.empty()) {
                cmd += " " + dependencies;
            }
            
            return cmd;
        }
    }

    std::string getDependencies() {
        // Здесь должна быть логика чтения зависимостей из cppkg.json
        // и формирования соответствующих флагов компиляции
        // Это заглушка для примера
        // return "-Lvendor/lib -lboost -lssl";  // Пример использования библиотек
        return "";  // Пример использования библиотек
    }

    std::string getConfigValue(const std::string& key, const std::string& default_value) {
        // Здесь должна быть реализация чтения значений из cppkg.json
        // Это заглушка для примера
        static std::unordered_map<std::string, std::string> config = {
            {"cpp_version", "17"},
            {"name", "my_project"}
        };
        
        auto it = config.find(key);
        return (it != config.end()) ? it->second : default_value;
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
