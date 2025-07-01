#ifndef QUICK_CPPKG_RUN_HPP
#define QUICK_CPPKG_RUN_HPP
#include "command.hpp"
#include <filesystem>
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
using json = nlohmann::json;
namespace fs = std::filesystem;
class RunHandler : public CommandHandler{
public:
    void execute() override {
        std::string app_name;
        if (fs::exists("cppkg.json")) {
            std::ifstream file("cppkg.json");
            if (!file.is_open()) {
                std::cerr << "❌ Failed to open cppkg.json\n";
                return;
            }

            try {
                json config;
                file >> config;
                app_name = config.value("name", "Unnamed Project");
            } catch (const json::parse_error& e) {
                std::cerr << "❌ Failed to parse cppkg.json: " << e.what() << "\n";
                return;
            }
        } else {
            std::cerr << "❌ Project structure is invalid. Missing cppkg.json\n";
            return;
        }

        std::cout << "Project name: " << app_name << std::endl;

        int exit_code = runExecutable(app_name);
    }

    int runExecutable(const std::string& name) {
        std::string full_path = buildExecutablePath(name);

        if (!fs::exists(full_path)) {
            std::cerr << "❌ File not found: " << full_path << "\n";
            return -1;
        }

        std::cout << "▶️ Running: " << full_path << "\n";

        int result = std::system(full_path.c_str());

        if (result == 0) {
            std::cout << "✅ Succes\n";
        } else {
            std::cerr << "❌ Error\n";
        }

        return result;
    }
    std::string buildExecutablePath(const std::string& name) {
        std::string full_path = "build/" + name;

        #ifdef _WIN32
            full_path += ".exe";
        #endif

        return full_path;
    }
};

#endif
