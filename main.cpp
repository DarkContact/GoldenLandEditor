#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Can't init SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Simple SDL3",
                                          800, 600,
                                          NULL);

    if (!window) {
        SDL_Log("Can't create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Ждём 3 секунды, потом закрываемся
    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
