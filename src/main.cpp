#include "core/application.hpp"
#include "ui/consolecapture.hpp"
#include <iostream>
#include <exception>

int main() {
    ConsoleCapture::getInstance().startCapture();

    try {
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        ConsoleCapture::getInstance().stopCapture();
        return EXIT_FAILURE;
    }

    ConsoleCapture::getInstance().stopCapture();
    return EXIT_SUCCESS;
}