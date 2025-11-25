#include <exception>

#include <SDL3/SDL_main.h>

#include "Application.h"

int main(int, char**) {
    try {
        Application app;
        app.run();
    } catch (const std::exception& ex) {
        fprintf(stderr, "Exception: %s", ex.what());
        return -1;
    }

    return 0;
}
