#ifndef QUICK_CPPKG_DEPENDENCY_HPP
#define QUICK_CPPKG_DEPENDENCY_HPP

#include <string>

struct Dependency {
    std::string name;
    std::string version;
    std::string include_path;
    std::string library_path;
    std::string type; // header-only / static / dynamic
};

#endif
