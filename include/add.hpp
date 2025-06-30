#ifndef QUICK_CPPKG_ADD_HPP
#define QUICK_CPPKG_ADD_HPP

#include "command.hpp"
#include <vector> 
#include <iostream>
#include <string>

class AddHandler : public CommandHandler {
public:
    AddHandler(const std::vector<std::string>& packages) : packages_(packages) {}

    void execute() override {
        for (const auto& package : packages_) {
            size_t at_pos = package.find('@');
            std::string name = (at_pos != std::string::npos) 
                ? package.substr(0, at_pos) : package;
            std::string version = (at_pos != std::string::npos) 
                ? package.substr(at_pos + 1) : "latest";

            if (name.empty()) {
                std::cerr << "Error: Empty package name\n";
                continue;
            }

            std::cout << "Added package: " << name 
                      << " (version: " << version << ")\n";
        }
    }

private:
    std::vector<std::string> packages_;
};

#endif
