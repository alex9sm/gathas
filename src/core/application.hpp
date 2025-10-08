#pragma once

#include "window.hpp"

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    GatWindow window;

    void mainLoop();
};