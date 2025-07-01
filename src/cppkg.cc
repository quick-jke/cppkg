#define CLI11_IMPLEMENTATION
#include "CLI/CLI.hpp"
#include <iostream>
#include <string>
#include <unordered_set>
#include <filesystem>

#include "init.hpp"
#include "add.hpp"
#include "build.hpp"
#include "run.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    CLI::App app{"cppkg"};
    
    std::string cpp_version;

    std::string project_name;
    auto init_cmd = app.add_subcommand("init", "Initialize a new project");
    init_cmd->add_option("project_name", project_name, "Project name")
        ->expected(0, 1)
        ->each([](const std::string& name) {
            if (name.empty()) return;

            for (char c : name) {
                if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
                    throw CLI::ValidationError("Project name", 
                        "Invalid character: '" + std::string(1, c) + "'. Only letters, digits, '_', and '-' are allowed.");
                }
            }
        });
    
    init_cmd->add_option("--cpp", cpp_version, "C++ standard version")
        ->required()
        ->transform([](std::string val) {
            if (val.rfind("cpp", 0) == 0)
                val = val.substr(3);
            return val;
        })
        ->check([](const std::string& val) {
            static const std::unordered_set<std::string> valid_versions = {"11", "14", "17", "20", "23"};
            return valid_versions.count(val) ? "" : "Invalid C++ version";
        });

    std::vector<std::string> packages;
    auto add_cmd = app.add_subcommand("add", "Add dependencies")
        ->alias("install");
    add_cmd->add_option("packages", packages, "Package names with optional @version")
        ->required();

    auto build_cmd = app.add_subcommand("build", "Build the project")
        ->alias("compile");

    auto run_cmd = app.add_subcommand("run", "Run the project")
        ->alias("start");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        if (app.got_subcommand(init_cmd) && project_name.empty()) {
            project_name = "";
        }

        if (app.got_subcommand(init_cmd)) {
            InitHandler handler(project_name, cpp_version);
            handler.execute();
        } else if (app.got_subcommand(add_cmd)) {
            AddHandler handler(packages);
            handler.execute();
        } else if (app.got_subcommand(build_cmd)) {
            BuildHandler handler;
            handler.execute();
        } else if (app.got_subcommand(run_cmd)){
            RunHandler handler;
            handler.execute();
        } else {
            std::cout << app.help();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
