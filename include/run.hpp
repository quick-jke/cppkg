#ifndef QUICK_CPPKG_RUN_HPP
#define QUICK_CPPKG_RUN_HPP
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
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
                std::cerr << RED << "❌ Failed to open cppkg.json" << RESET << std::endl;
                return;
            }

            try {
                json config;
                file >> config;
                app_name = config.value("name", "Unnamed Project");
            } catch (const json::parse_error& e) {
                std::cerr << RED << "❌ Failed to parse cppkg.json: " << e.what() << RESET << std::endl;
                return;
            }
        } else {
            std::cerr << RED << "❌ Project structure is invalid. Missing cppkg.json" << RESET << std::endl;
            return;
        }

        int exit_code = runExecutable(app_name);
    }

    int runExecutable(const std::string& name) {
        std::string full_path = buildExecutablePath(name);

        if (!fs::exists(full_path)) {
            std::cerr << RED << "❌ File not found: " << full_path << RESET << std::endl;
            return -1;
        }

        std::cout << BLUE << "▶️ Running: " << full_path << RESET << std::endl;

        int result = std::system(full_path.c_str());

        if (result == 0) {
            std::cout << GREEN << "✅ Succes" << RESET << std::endl;
        } else {
            std::cerr << RED << "❌ Error" << RESET << std::endl;
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
