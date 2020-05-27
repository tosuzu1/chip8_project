#include <SDL2/SDL.h>
#include <stdlib.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define SDL_SCREEN_X 640
#define DISPLAY_RESOLUTION_VERTICAL 32
#define SDL_SCREEN_Y 320

int main () 
{
    uint8_t displayGrid[DISPLAY_RESOLUTION_HORIZONTAL][DISPLAY_RESOLUTION_VERTICAL];

    for(int x = 0; x < DISPLAY_RESOLUTION_HORIZONTAL; x += 1)
    {
        for(int y = 0; y < DISPLAY_RESOLUTION_VERTICAL; y += 1)
        {
            displayGrid[x][y] = rand() % 10; // Display between 0 and 9
        }
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *screen = SDL_CreateWindow("Chip-8",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 320, SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    

    for(int y = 0; y < DISPLAY_RESOLUTION_VERTICAL; y++) 
    {
        for(int x = 0; x < DISPLAY_RESOLUTION_HORIZONTAL; x++) 
        {
            printf("disp[%d][%d] = %d\n", x, y, displayGrid[x][y]);
            if(displayGrid[x][y]  >= 4)
            {
                SDL_Rect dstrect;
                dstrect.x = x * 10;
                dstrect.y = y * 10;
                dstrect.w = 10;
                dstrect.h = 10;

                SDL_RenderFillRect(renderer, &dstrect);
            }
        }
    }
    SDL_RenderPresent(renderer);
    sleep(10);
    SDL_DestroyWindow( screen );
    screen = NULL;
    SDL_Quit();
}