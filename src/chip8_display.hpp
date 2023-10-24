#pragma once

#include <cstdint>
#include <SDL2/SDL.h>
#include "chip8.hpp"


class Chip8_display
{

public:
	Chip8_display(char const* title, int windowWidth, int windowHeight);
	~Chip8_display();
	void Update(void const* buffer, int pitch);
	bool ProcessInput(uint8_t* keys);

private:
	SDL_Window* screen{};
	SDL_GLContext gl_context{};
	SDL_Renderer* renderer{};
    SDL_Event sdl_events;
};