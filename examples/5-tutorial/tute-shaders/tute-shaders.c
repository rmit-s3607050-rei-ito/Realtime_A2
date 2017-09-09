/* Adapted from code originally written by aholkner */

#include "sdl-base.h"
#include "shaders.h"

#include <GL/glu.h>
#include <GL/glut.h>			/* for screen text only */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CAMERA_VELOCITY 0.005		/* Units per millisecond */
#define CAMERA_ANGULAR_VELOCITY 0.05	/* Degrees per millisecond */
#define CAMERA_MOUSE_X_VELOCITY 0.3	/* Degrees per mouse unit */
#define CAMERA_MOUSE_Y_VELOCITY 0.3	/* Degrees per mouse unit */

typedef struct
{
  float x, y, z;
} vector_t;

// [ADDITION]: Shader program used and vertex/fragment files
static int shaderProgram;
static const char* vertexFile = "./shader.vert";
static const char* fragmentFile = "./shader.frag";

static vector_t camera_pos;  /* Position, in world space, of the camera */
static float camera_heading; /* Direction in degrees the camera faces */
static float camera_pitch;	 /* Up/down degrees for camera */
static int mouse1_down;		   /* Left mouse button Up/Down. Only move camera when down. */

/* sphere arrays */
static int num_verts = 0;
static int stacks = 16, slices = 16;
static vector_t* vertices = NULL;
static vector_t* normals = NULL;

/* Store the state (true = pressed, false = not pressed) of the keys we're interested in. */
typedef struct Controls
{
  bool w;
  bool s;
  bool a;
  bool d;
  bool left;
  bool right;
} Controls;

Controls controls;

/* Store render state variables.  Can be toggled with function keys. */
static struct
{
  int lighting;
  int cull;
  int cullFace;
  int polygonMode[2];
  int useShaders;
} renderstate;

enum {front_face = 0, back_face };

/* Light and materials */
static float light0_position[] = {0.0, 0.0, 1.0, 0.0};

void draw_axes(float length) {
  // Disable all render effects for axes
  if (renderstate.useShaders)
    glUseProgram(0);
  if (renderstate.lighting)
    glDisable(GL_LIGHTING);
  if (renderstate.cull)
    glDisable(GL_CULL_FACE);

  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINE_STRIP);
    glVertex3f(0, 0, 0);
    glVertex3f(length, 0, 0);
  glEnd();

  glColor3f(0.0, 1.0, 0.0);
  glBegin(GL_LINE_STRIP);
    glVertex3f(0, 0, 0);
    glVertex3f(0, length, 0);
  glEnd();

  glColor3f(0.0, 0.0, 1.0);
  glBegin(GL_LINE_STRIP);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, length);
  glEnd();

  // Re-enable render effects
  if (renderstate.useShaders)
    glUseProgram(shaderProgram);
  if (renderstate.lighting)
    glEnable(GL_LIGHTING);
  if (renderstate.cull)
    glEnable(GL_CULL_FACE);
}

void print_matrix(float m[])
{
  for (int j=0; j<4; j++) {
    for (int i=0; i<4; i++)
      printf("%f ", m[i*4+j]);
    printf("\n");
  }
}

void print_GL_state(int p)
{
  switch (p) {
  case GL_MODELVIEW_MATRIX:
    {
      float m[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, m);
      printf("Modelview Matrix\n");
      print_matrix(m);
      break;
    }
  case GL_PROJECTION_MATRIX:
    {
      float m[16];
      glGetFloatv(GL_PROJECTION_MATRIX, m);
      printf("Projection Matrix\n");
      print_matrix(m);
      break;
    }
  default:
    break;
  }
}

void generate_sphere(float radius, int stacks, int slices)
{
  int stack, slice;
  int i;

  /* Free previous data */
  free(vertices);
  free(normals);

  /* Allocate new data */
  num_verts = (stacks + 1) * (slices + 1);
  vertices = malloc(sizeof(vector_t) * num_verts);
  normals = malloc(sizeof(vector_t) * num_verts);

  /* Generate vertices */
  for (stack = 0; stack <= stacks; stack++) {
    float phi = M_PI * stack / stacks;
    for (slice = 0; slice <= slices; slice++) {
      float theta = 2.0 * M_PI * slice / slices;
      i = stack * (slices + 1) + slice;
      vertices[i].x = radius * sinf(phi) * cosf(theta);
      vertices[i].y = radius * sinf(phi) * sinf(theta);
      vertices[i].z = radius * cosf(phi);
      normals[i].x = vertices[i].x / radius;
      normals[i].y = vertices[i].y / radius;
      normals[i].z = vertices[i].z / radius;
    }
  }
}

void draw_sphere(int stacks, int slices)
{
  int stack, slice;

  /* Draw two triangles per quad to fill stacks/slices */
  glBegin(GL_TRIANGLES);
  for (stack = 0; stack < stacks; stack++) {
    for (slice = 0; slice < slices; slice++) {
      glNormal3fv((float*)(normals + stack * (slices+1) + slice));
      glVertex3fv((float*)(vertices + stack * (slices+1) + slice));
      glNormal3fv((float*)(normals + (stack+1) * (slices+1) + slice));
      glVertex3fv((float*)(vertices + (stack+1) * (slices+1) + slice));
      glNormal3fv((float*)(normals + (stack+1) * (slices+1) + slice + 1));
      glVertex3fv((float*)(vertices + (stack+1) * (slices+1) + slice + 1));

      glNormal3fv((float*)(normals + stack * (slices+1) + slice));
      glVertex3fv((float*)(vertices + stack * (slices+1) + slice));
      glNormal3fv((float*)(normals + (stack+1) * (slices+1) + slice + 1));
      glVertex3fv((float*)(vertices + (stack+1) * (slices+1) + slice + 1));
      glNormal3fv((float*)(normals + stack * (slices+1) + slice + 1));
      glVertex3fv((float*)(vertices + stack * (slices+1) + slice + 1));
    }
  }
  glEnd();
}

void update_renderstate()
{
  // Enable/Disable shaderProgram use
  if (renderstate.useShaders)
    glUseProgram(shaderProgram);
  else
    glUseProgram(0);

  if (renderstate.lighting) {
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  }  else  {
    glDisable(GL_LIGHTING);
  }
  if (renderstate.cull)  {
    glEnable(GL_CULL_FACE);
    glCullFace(renderstate.cullFace);
  } else {
    glDisable(GL_CULL_FACE);
  }
  glPolygonMode(GL_FRONT, renderstate.polygonMode[front_face]);
  glPolygonMode(GL_BACK, renderstate.polygonMode[back_face]);
}

void init(int argc, char **argv)
{
  glutInit(&argc, argv);

  glClearColor(0, 0, 0, 0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);

  camera_pos.x = 0;
  camera_pos.y = 0;
  camera_pos.z = -1;
  camera_heading = 180.0;
  camera_pitch = 0.0;
  mouse1_down = 0;

  /* Turn off all keystate flags */
  memset(&controls, 0, sizeof(controls));

  /* Default render modes */
  renderstate.lighting = 1;
  renderstate.cull = 0;
  renderstate.cullFace = GL_BACK;
  renderstate.polygonMode[front_face] = GL_FILL;
  renderstate.polygonMode[back_face] = GL_FILL;
  renderstate.useShaders = 0;

  generate_sphere(1.0, stacks, slices);

  // Define the shader program using the input files (predefined)
  shaderProgram = getShader(vertexFile, fragmentFile);

  update_renderstate();
}

void reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  /* Set the projection matrix - even though it doesn't actually change! */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, 0, 2);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void display()
{
  int err;

  //#define DEBUG
  //#undef DEBUG
#ifdef DEBUG
  print_GL_state(GL_PROJECTION_MATRIX);
#endif

  /* Clear the colour and depth buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Camera transformation */
  glLoadIdentity();
  glPushMatrix();
  glTranslatef(-camera_pos.x, -camera_pos.y, camera_pos.z);
  glRotatef(-camera_pitch, 1, 0, 0);
  glRotatef(-camera_heading, 0, 1, 0);

#ifdef DEBUG
  print_GL_state(GL_MODELVIEW_MATRIX);
#endif

  /* Draw the scene */
  glColor3f(1.0, 0.0, 0.0);
  draw_sphere(stacks, slices);
  draw_axes(1.0);
  glPopMatrix();

  while ((err = glGetError()) != GL_NO_ERROR)
    printf("glGetError %s\n", gluErrorString(err));
}

void update(int milliseconds)
{
  float cam_rads = 0;

  /* Calculate the velocity of the camera, based on which keys are pressed.  Could have used SDL_GetKeyState instead of storing it ourselves. */
  float camera_dx = 0.0;
  float camera_dz = 0.0;
  float camera_dangle = 0.0;
  if (controls.w)
    camera_dz += CAMERA_VELOCITY;
  if (controls.s)
    camera_dz -= CAMERA_VELOCITY;
  if (controls.d)
    camera_dx += CAMERA_VELOCITY;
  if (controls.a)
    camera_dx -= CAMERA_VELOCITY;
  if (controls.left)
    camera_dangle += CAMERA_ANGULAR_VELOCITY;
  if (controls.right)
    camera_dangle -= CAMERA_ANGULAR_VELOCITY;

  /* Update position of camera based on heading, velocity and time */
  camera_pos.x += milliseconds * (sin(-cam_rads) * camera_dz + cos(cam_rads) * camera_dx);
  camera_pos.z += milliseconds * (cos(-cam_rads) * camera_dz + sin(cam_rads) * camera_dx);

  /* Update angle of camera based on velocity and time */
  camera_heading += camera_dangle * milliseconds;
  cam_rads = camera_heading * M_PI / 180.0;
}

void set_mousestate(unsigned char button, int state)
{
  switch (button) {
  case SDL_BUTTON_LEFT:
    mouse1_down = state;
    break;
  default:
    break;
  }
}

bool event(SDL_Event *event)
{
  static int first_mousemotion = 1;

  switch (event->type) {
  case SDL_KEYDOWN:
    /* Handle non-state keys */
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      return true;
      break;
    case SDLK_F1:
      renderstate.lighting = !renderstate.lighting;
      update_renderstate();
      break;
    case SDLK_F2:
      if (renderstate.polygonMode[front_face] == GL_FILL)
	     renderstate.polygonMode[front_face] = GL_LINE;
      else
	     renderstate.polygonMode[front_face] = GL_FILL;
       update_renderstate();
      break;
    case SDLK_F3:
      if (renderstate.polygonMode[back_face] == GL_FILL)
	     renderstate.polygonMode[back_face] = GL_LINE;
      else
	     renderstate.polygonMode[back_face] = GL_FILL;
       update_renderstate();
       break;
    case SDLK_F4:
      renderstate.cull = !renderstate.cull;
      update_renderstate();
      break;
    case SDLK_F5:
      if (renderstate.cullFace == GL_BACK)
	     renderstate.cullFace = GL_FRONT;
      else
	     renderstate.cullFace = GL_BACK;
       update_renderstate();
       break;
    case SDLK_F6:
      renderstate.useShaders = !renderstate.useShaders;
      update_renderstate();
      break;
      /* Handle state keys */
    case SDLK_w:
      controls.w = true;
      break;
    case SDLK_s:
      controls.s = true;
      break;
    case SDLK_d:
      controls.d = true;
      break;
    case SDLK_a:
      controls.a = true;
      break;
    case SDLK_LEFT:
      controls.left = true;
      break;
    case SDLK_RIGHT:
      controls.right = true;
      break;

    default:
      break;
    }
    break;

  case SDL_KEYUP:
    /* Handle state keys */
    switch (event->key.keysym.sym) {
    case SDLK_w:
      controls.w = false;
      break;
    case SDLK_s:
      controls.s = false;
      break;
    case SDLK_d:
      controls.d = false;
      break;
    case SDLK_a:
      controls.a = false;
      break;
    case SDLK_LEFT:
      controls.left = false;
      break;
    case SDLK_RIGHT:
      controls.right = false;
      break;

    default:
      break;
    }
    break;

  case SDL_MOUSEBUTTONDOWN:
    set_mousestate(event->button.button, 1);
    break;

  case SDL_MOUSEBUTTONUP:
    set_mousestate(event->button.button, 0);
    break;

  case SDL_MOUSEMOTION:
    if (first_mousemotion) {
      /* The first mousemotion event will have bogus xrel and yrel, so ignore it. */
      first_mousemotion = 0;
      break;
    }
    if (mouse1_down) {
      /* Only move the camera if the mouse is down */
      camera_heading -= event->motion.xrel * CAMERA_MOUSE_X_VELOCITY;
      camera_pitch -= event->motion.yrel * CAMERA_MOUSE_Y_VELOCITY;
    }
    break;

  default:
    break;
  }

  return false;
}

void cleanup()
{
  free(vertices);
  free(normals);
}
