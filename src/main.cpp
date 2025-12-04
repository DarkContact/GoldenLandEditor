#include <exception>

#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_main.h>

#include "Application.h"

int main(int, char**) {
    try {
        Application app;
        app.mainLoop();
    } catch (const std::exception& ex) {
        fprintf(stderr, "Exception: %s\n", ex.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", ex.what(), NULL);
        return -1;
    }

    return 0;
}
