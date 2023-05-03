

#include "app/app.hpp"
#include <etna/init.hpp>
#include <iostream>

int main() {
    if (!Etna::initialize()) {
        std::cerr << "Could not find Vulkan library\n";
        return EXIT_FAILURE;
    }

    App app;

    try {
        app.initialize();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}