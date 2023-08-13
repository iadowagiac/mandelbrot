#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include <SDL2/SDL.h>

#include "graphics.h"

#define MAX_ITERATIONS 256

#define IMAGE_WIDTH  480
#define IMAGE_HEIGHT 272

#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 272

static struct timeval now, last_time;

static struct pixelmap pixelmap;

static unsigned long color[MAX_ITERATIONS];

void
fatal_error(char *error, ...)
{
    va_list args;
    va_start(args, error);
    fprintf(stderr, "error: ");
    vfprintf(stderr, error, args);
    fputc('\n', stderr);
    va_end(args);
    exit(EXIT_FAILURE);
}

float
getdt(void)
{
    struct timeval res;
    gettimeofday(&now, NULL);
    timersub(&now, &last_time, &res);
    return (float)res.tv_usec / 1000000;
}

void
initialize_colors(void)
{
    int stage = 0;
    float r = 0.0,
          g = 0.0,
          b = 0.0;
    for (int i = 0; i < sizeof(color) / sizeof(*color); i++)
    {
        color[i] = ABGR8888(1.0, b, g, r);
        switch (stage)
        {
            case 0:
                r += 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (r >= 1.0)
                {
                    r = 1.0;
                    stage = 1;
                }
                break;
            case 1:
                g += 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (g >= 1.0)
                {
                    g = 1.0;
                    stage = 2;
                }
                break;
            case 2:
                r -= 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (r <= 0.0)
                {
                    r = 0.0;
                    stage = 3;
                }
                break;
            case 3:
                b += 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (b >= 1.0)
                {
                    b = 1.0;
                    stage = 4;
                }
                break;
            case 4:
                g -= 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (g <= 0.0)
                {
                    g = 0.0;
                    stage = 5;
                }
                break;
            case 5:
                r += 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (r >= 1.0)
                {
                    r = 1.0;
                    stage = 6;
                }
                break;
            case 6:
                b -= 1.0 / (sizeof(color) / sizeof(*color) / 8);
                if (b <= 0.0)
                {
                    r = 0.0;
                    stage = 0;
                }
                break;
        }
    }
}

int 
plot_mandelbrot_set(void)
{
    static float zoom = 0.0;
    float dt = getdt();

    zoom += dt;

    for (int py = 0; py < pixelmap.height; py++)
    {
        for (int px = 0; px < pixelmap.width; px++)
        {
            unsigned long c;
            int i = 0;
            float xtemp,
                  x = 0.0,
                  y = 0.0,
                  x0 = ((float)px / (IMAGE_WIDTH  / 2.47) - 2.00 - 0.82 * zoom) / zoom,
                  y0 = ((float)py / (IMAGE_HEIGHT / 2.24) - 1.12 - 0.20 * zoom) / zoom;
            while (x * x + y * y <= 2 * 2 && i < MAX_ITERATIONS)
            {
                xtemp = x * x - y * y + x0;
                y = 2 * x * y + y0;
                x = xtemp;
                i++;
            }
            c = color[i % (sizeof(color) / sizeof(*color))]; 
            setpixel(&pixelmap, px, py, c);
        }
    }
}

void
draw_scene(void)
{
    // Clear the pixelmap.
    setallpixels(&pixelmap, ABGR8888(1.0, 0.9, 0.9, 0.9));

    plot_mandelbrot_set();
}

/**
 * This is where my stuff ends and SDL begins.
 **/
void
update_screen(SDL_Renderer *renderer)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect =
    {
        .x = 0,
        .y = 0,
        .w = WINDOW_WIDTH,
        .h = WINDOW_HEIGHT,
    };

    draw_scene();

    surface = SDL_CreateRGBSurfaceFrom(pixelmap.pixeldata,
                                       pixelmap.width, pixelmap.height,
                                       32, 4 * pixelmap.stride,
                                       0xFF000000, 0x00FF0000,
                                       0x0000FF00, 0x000000FF);

    texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

int
main(int argc, char *argv[])
{

    bool running = true;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN,
                                &window, &renderer);
    if (window == NULL)
        fatal_error("failed to create window");
    SDL_SetWindowTitle(window, "mandelbrot set renderer");

    // Create the pixelmap.
    createpixelmap(&pixelmap, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 32, NULL);

    // Initialize the mandelbrot colors.
    initialize_colors();

    gettimeofday(&last_time, NULL);

    while (running)
    {
        update_screen(renderer);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;
            }
        }
        last_time = now;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
