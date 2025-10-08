#include "application.hpp"
#include <iostream>

Application::Application() {
    window.initWindow();
}

Application::~Application() {
}

void Application::run() {
    mainLoop();
}

void Application::mainLoop() {
    while (!window.shouldClose()) {
        window.pollEvents();

    }
}