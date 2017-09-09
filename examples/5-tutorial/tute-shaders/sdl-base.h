/* Adapted from code originally written by aholkner */

#ifndef SDL_BASE_H
#define SDL_BASE_H

#if __cplusplus
extern "C" {
#endif

#define GL_GLEXT_PROTOTYPES

#if _WIN32
#	include <Windows.h>
#	include <GL/glew.h>
#endif
#if __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#else
#	include <GL/gl.h>
#	include <GL/glu.h>
#endif

#include <SDL2/SDL.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif


/* Implement these yourself */
void init(int argc, char **argv);
void reshape(int w, int h);
void update(int milliseconds);
void display();
bool event(SDL_Event *event);
void cleanup();

/* This is updated every second by the main loop -- no need to calculate it yourself.*/
extern int fps;

/* Call this to quit. */
void quit();

#if __cplusplus
}
#endif

#endif
