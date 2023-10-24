#include "chip8_display.hpp"

Chip8_display::Chip8_display (char const* title, int windowWidth, int windowHeight)
{
     // Setup SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *screen = SDL_CreateWindow(title,  
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        windowWidth, 
        windowHeight, 
        SDL_WINDOW_OPENGL);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event sdl_events;
}

Chip8_display::~Chip8_display()
{
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	SDL_Quit();
}