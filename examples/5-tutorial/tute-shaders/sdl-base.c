/* Adapted from code originally written by aholkner */

#include "sdl-base.h"

#include <stdio.h>
#include <stdlib.h>

/* Frame counting */
static int frameCount;
static Uint32 frameTime;
int fps;

static SDL_Window *mainWindow = 0;


int main(int argc, char **argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    return EXIT_SUCCESS;
  }

  int width = 512;
  int height = 512;

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  mainWindow = SDL_CreateWindow("Shader Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!mainWindow) {
    fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
    return EXIT_SUCCESS;
  }

  SDL_GLContext mainGLContext = SDL_GL_CreateContext(mainWindow);
  SDL_GL_MakeCurrent(mainWindow, mainGLContext);

#if _WIN32
  glewInit();
#endif

  init(argc, argv);
  reshape(width, height);

  bool done = false;

  /* Insert main program loop here */
  fps = 0;
  frameCount = 0;
  Uint32 lastFrameTime = frameTime = SDL_GetTicks();
  while (!done) {
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_QUIT:
	done = true;
	break;
      case SDL_WINDOWEVENT:
	switch (ev.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
	  if (ev.window.windowID == SDL_GetWindowID(mainWindow)) {
	    SDL_SetWindowSize(mainWindow, ev.window.data1, ev.window.data2);
	    reshape(ev.window.data1, ev.window.data2);
	  }
	  break;
	case SDL_WINDOWEVENT_CLOSE:
	  done = true;
	  break;
	default:
	  break;
	}
	break;

      default:
	done = event(&ev);
	break;
      }
    }

    Uint32 now = SDL_GetTicks();
    update(now - lastFrameTime);
    lastFrameTime = now;

    display();
    SDL_GL_SwapWindow(mainWindow);

    frameCount++;
    if (now - frameTime > 1000) {
      fps = (frameCount * 1000) / (now - frameTime);
      frameCount = 0;
      frameTime = now;
    }
  }

  cleanup();
  SDL_Quit();

  return EXIT_SUCCESS;
}
