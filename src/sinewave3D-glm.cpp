/*
 * Simple 3D sine wave animation example using glm
 * $Id: sinewave3D-glm.cpp,v 1.8 2017/08/23 12:56:02 gl Exp gl $
 */

// NOTE: need to be placed before #include, enables glUseProgram() to work
#define GL_GLEXT_PROTOTYPES
#include "shaders.h"

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

// Shader program used and vertex/fragment files
static int shaderProgram;
static const char* vertexFile = "./shader.vert";
static const char* fragmentFile = "./shader.frag";
// Uniform locations for variables that are passed into the shader program;
static GLint tesselationLoc, dimensionLoc;
static GLint shineLoc, timeLoc;
static GLint phongLoc, pixelLoc, positionalLoc, fixedLoc, flatLoc;
static GLint normalMatLoc, modelViewMatLoc, projectionMatLoc;
static GLint lightingLoc;

typedef enum {
  d_drawSineWave,
  d_mouse,
  d_key,
  d_animation,
  d_lighting,
  d_OSD,
  d_matrices,
  d_computeLighting,
  d_nflags
} DebugFlags;

bool debug[d_nflags] =
{
  false, // d_drawSineWave
  false, // d_mouse
  false, // d_key
  false, // d_animation
  false, // d_lighting
  false, // d_OSD
  false, // d_matrices
  false, // d_computeLighting
};

typedef struct { float r, g, b; } color3f;

typedef enum { FRAME, FLAGS, VALUES } OSD;

// Buffer offset used in VBOs, essentially the same as assignment 1
#define BUFFER_OFFSET(i) ((void*)(i))

// Defined vertex for VBOs
typedef struct {
  glm::vec3 pos, normal, color;
} Vertex;

Vertex *vertices;             // Store vertices for vbos
unsigned int* indices;        // Store indices for vbos
size_t numVerts, numIndices;  // Count number of vertices/indices
unsigned vbo, ibo, cbo;       // Buffers

typedef struct {
  bool animate;
  float t, lastT;
  bool lighting;
  bool twoside;
  bool drawNormals;
  int width, height;
  int tess;
  int waveDim;
  int frameCount;
  float frameRate;
  float displayStatsInterval;
  int lastStatsDisplayT;
  bool displayOSD;
  bool consolePM;
  bool multiView;

  // additions
  bool flat;
  bool positional;
  bool fixed;
  bool useShaders;
  float shininess;
  bool phong;
  OSD option;
  bool perPixel;
  bool wave;
  bool vbo;
  bool wireframe;
} Global;

Global g =
{
  // preset in original code
  false, // animate
  0.0,   // t
  0.0,   // lastT
  true,  // lighting
  false, // twoSide
  false, // drawNormals
  0,     // width
  0,     // height
  8,     // tess
  2,     // waveDim
  0,     // frameCount
  0.0,   // frameRate
  1.0,   // displayStatsInterval
  0,     // lastStatsDisplayT
  true,  // displayOSD
  false, // consolePM
  false, // multiView

  // new added flags/values
  false, // flat
  false, // positional
  false, // fixed
  false, // useShaders
  20.0,  // shininess
  false, // phong
  FRAME, // option
  false, // perPixel
  false, // wave
  false, // vbo
  false, // wireframe
};

typedef enum { inactive, rotate, pan, zoom } CameraControl;

struct camera_t {
  int lastX, lastY;
  float rotateX, rotateY;
  float scale;
  CameraControl control;
} camera = { 0, 0, 30.0, -30.0, 1.0, inactive };

// Colors defined
glm::vec3 cyan(0.0, 1.0, 1.0);
glm::vec3 cyanDiffuse(0.0, 0.5, 0.5);
glm::vec3 yellow(1.0, 1.0, 0.0);
glm::vec3 white(1.0, 1.0, 1.0);
glm::vec3 grey(0.8, 0.8, 0.8);
glm::vec3 black(0.0, 0.0, 0.0);
GLfloat lightPosition[] = { 0.5, 0.5, 0.5, 0.0 };

const float milli = 1000.0;

glm::mat4 modelViewMatrix;
glm::mat3 normalMatrix;

/* ########## DEBUGGING RELATED FUNCTIONS ########## */
int err;

void printVec(float *v, int n)
{
  int i;

  for (i = 0; i < n; i++)
    printf("%5.3f ", v[i]);
  printf("\n");
}

void printMatrixLinear(float *m, int n)
{
  int i;

  for (i = 0; i < n; i++)
    printf("%5.3f ", m[i]);
  printf("\n");
}

void printMatrixColumnMajor(float *m, int n)
{
  int i, j;

  for (j = 0; j < n; j++) {
    for (i = 0; i < n; i++) {
      printf("%5.3f ", m[i*4+j]);
    }
    printf("\n");
  }
  printf("\n");
}

/* ########## ENABLING SHADER PROGRAM ########## */
void applyShading()
{
  // Define projection matrix
  glm::mat4 projectionMatrix = glm::ortho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);

  // Place program in use for shaders
  glUseProgram(shaderProgram);

  // Uniforms that can be passed into both shader.vert and shader.frag
  // ints
  glUniform1i(tesselationLoc, g.tess);
  glUniform1i(dimensionLoc, g.waveDim);
  // floats
  glUniform1f(shineLoc, g.shininess);
  glUniform1f(timeLoc, g.t);
  // bools
  glUniform1i(phongLoc, g.phong);
  glUniform1i(pixelLoc, g.perPixel);
  glUniform1i(positionalLoc, g.positional);
  glUniform1i(fixedLoc, g.fixed);
  glUniform1i(flatLoc, g.flat);
  glUniform1i(lightingLoc, g.lighting);
  // matricies
  glUniformMatrix3fv(normalMatLoc, 1, false, &normalMatrix[0][0]);
  glUniformMatrix4fv(modelViewMatLoc, 1, false, &modelViewMatrix[0][0]);
  glUniformMatrix4fv(projectionMatLoc, 1, false, &projectionMatrix[0][0]);
}

/* ########## DEFAULT FUNCTIONS ########## */
void init(void)
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  if (g.twoside)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  // Define the shader program using the input files (predefined)
  shaderProgram = getShader(vertexFile, fragmentFile);

  // Obtain uniform variables from the shader program
  // ints
  tesselationLoc = glGetUniformLocation(shaderProgram, "uTesselation");
  dimensionLoc = glGetUniformLocation(shaderProgram, "uDimension");
  // floats
  shineLoc = glGetUniformLocation(shaderProgram, "uShininess");
  timeLoc = glGetUniformLocation(shaderProgram, "uTime");
  // bools
  phongLoc = glGetUniformLocation(shaderProgram, "uPhong");
  pixelLoc = glGetUniformLocation(shaderProgram, "uPixel");
  positionalLoc = glGetUniformLocation(shaderProgram, "uPositional");
  fixedLoc = glGetUniformLocation(shaderProgram, "uFixed");
  flatLoc = glGetUniformLocation(shaderProgram, "uFlat");
  lightingLoc = glGetUniformLocation(shaderProgram, "uLighting");
  // mats
  normalMatLoc = glGetUniformLocation(shaderProgram, "uNormalMat");
  modelViewMatLoc = glGetUniformLocation(shaderProgram, "uModelViewMat");
  projectionMatLoc = glGetUniformLocation(shaderProgram, "uProjectionMat");
}

void reshape(int w, int h)
{
  g.width = w;
  g.height = h;
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void drawAxes(float length)
{
  glm::vec4 v;

  glPushAttrib(GL_CURRENT_BIT);
  glBegin(GL_LINES);

  /* x axis */
  glColor3f(1.0, 0.0, 0.0);
  v = modelViewMatrix * glm::vec4(-length, 0.0, 0.0, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(length, 0.0, 0.0, 1.0);
  glVertex3fv(&v[0]);

  /* y axis */
  glColor3f(0.0, 1.0, 0.0);
  v = modelViewMatrix * glm::vec4(0.0, -length, 0.0, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(0.0, length, 0.0, 1.0);
  glVertex3fv(&v[0]);

  /* z axis */
  glColor3f(0.0, 0.0, 1.0);
  v = modelViewMatrix * glm::vec4(0.0, 0.0, -length, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(0.0, 0.0, length, 1.0);
  glVertex3fv(&v[0]);

  glEnd();
  glPopAttrib();
}

void drawVector(glm::vec3 & o, glm::vec3 & v, float s,
  bool normalize, glm::vec3 & c)
{
  glPushAttrib(GL_CURRENT_BIT);
  glColor3fv(&c[0]);
  glBegin(GL_LINES);
  if (normalize)
    v = glm::normalize(v);

  glVertex3fv(&o[0]);
  glm::vec3 e(o + s * v);
  glVertex3fv(&e[0]);
  glEnd();
  glPopAttrib();
}

// Console performance meter
void consolePM()
{
  // Console output also toggles depending on the OSD option
  if (g.option == FRAME) {
    printf("FRAME\n"); //OSD option
    printf("frame rate (f/s):  %5.0f\n", g.frameRate);
    printf("frame time (ms/f): %5.0f\n", 1.0 / g.frameRate * 1000.0);
  }
  else if (g.option == FLAGS) {
    printf("FLAGS\n"); //OSD option
    printf("animation: %s\n", g.animate?"true":"false");
    printf("flat: %s\n", g.flat?"true":"false");
    printf("console: %s\n", g.consolePM?"true":"false");
    printf("positional: %s\n", g.positional?"true":"false");
    printf("fixed: %s\n", g.fixed?"true":"false");
    printf("shaders: %s\n", g.useShaders?"true":"false");
    printf("lighting: %s\n", g.lighting?"true":"false");
    printf("phong: %s\n", g.phong?"true":"false");
    printf("normals: %s\n", g.drawNormals?"true":"false");
    printf("per pixel: %s\n", g.perPixel?"true":"false");
    printf("wave: %s\n", g.wave?"true":"false");
    printf("vbo: %s\n", g.vbo?"true":"false");
    printf("multiview: %s\n", g.multiView?"true":"false");
    printf("wireframe: %s\n", g.wireframe?"true":"false");
  }
  else if (g.option == VALUES) {
    printf("VALUES\n"); //OSD option
    printf("shininess: %.2f\n", g.shininess);
    printf("tesselation: %d\n", g.tess);
    printf("dimension: %d\n", g.waveDim);
  }
}

// On screen display
void displayOSD()
{
  char buffer[30];
  char *bufp;
  int w, h;

  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  /* Set up orthographic coordinate system to match the window,
     i.e. (0,0)-(w,h) */
  w = glutGet(GLUT_WINDOW_WIDTH);
  h = glutGet(GLUT_WINDOW_HEIGHT);
  glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(1.0, 1.0, 0.0);
  if (g.option == FRAME) {
    // OSD option
    glRasterPos2i(10, 40);
    snprintf(buffer, sizeof buffer, "FRAME (o)");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // Frame rate
    glRasterPos2i(10, 25);
    snprintf(buffer, sizeof buffer, "frame rate (f/s):  %5.0f", g.frameRate);
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // Frame time
    glRasterPos2i(10, 10);
    snprintf(buffer, sizeof buffer, "frame time (ms/f): %5.0f",
      1.0 / g.frameRate * milli);
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
  }
  else if (g.option == FLAGS) {
    // OSD option
    glRasterPos2i(10, 220);
    snprintf(buffer, sizeof buffer, "FLAGS (o)");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // animation
    glRasterPos2i(10, 205);
    snprintf(buffer, sizeof buffer, "animation (a): %s", g.animate?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // shader type
    glRasterPos2i(10, 190);
    snprintf(buffer, sizeof buffer, "flat (b): %s", g.flat?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // console output
    glRasterPos2i(10, 175);
    snprintf(buffer, sizeof buffer, "console (c): %s", g.consolePM?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // light type
    glRasterPos2i(10, 160);
    snprintf(buffer, sizeof buffer, "positional (d): %s", g.positional?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // fixed
    glRasterPos2i(10, 145);
    snprintf(buffer, sizeof buffer, "fixed (f): %s", g.fixed?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // shaders
    glRasterPos2i(10, 130);
    snprintf(buffer, sizeof buffer, "shaders (g): %s", g.useShaders?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // lighting
    glRasterPos2i(10, 115);
    snprintf(buffer, sizeof buffer, "lighting (l): %s", g.lighting?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // lighting calculation method
    glRasterPos2i(10, 100);
    snprintf(buffer, sizeof buffer, "phong (m): %s", g.phong?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // normals
    glRasterPos2i(10, 85);
    snprintf(buffer, sizeof buffer, "normals (n): %s", g.drawNormals?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // lighting calculation type
    glRasterPos2i(10, 70);
    snprintf(buffer, sizeof buffer, "per pixel (p): %s", g.perPixel?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // shape
    glRasterPos2i(10, 55);
    snprintf(buffer, sizeof buffer, "wave (s): %s", g.wave?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // vbos
    glRasterPos2i(10, 40);
    snprintf(buffer, sizeof buffer, "vbo (v): %s", g.vbo?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // multiview
    glRasterPos2i(10, 25);
    snprintf(buffer, sizeof buffer, "multiview (4): %s", g.multiView?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // wireframe
    glRasterPos2i(10, 10);
    snprintf(buffer, sizeof buffer, "wireframe (w): %s", g.wireframe?"true":"false");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
  }
  else if (g.option == VALUES) {
    // OSD option
    glRasterPos2i(10, 55);
    snprintf(buffer, sizeof buffer, "VALUES (o)");
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // shininess
    glRasterPos2i(10, 40);
    snprintf(buffer, sizeof buffer, "shininess (H/h): %.2f", g.shininess);
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // tesselation
    glRasterPos2i(10, 25);
    snprintf(buffer, sizeof buffer, "tesselation (+/-): %d", g.tess);
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
    // dimention
    glRasterPos2i(10, 10);
    snprintf(buffer, sizeof buffer, "dimension (z): %d", g.waveDim);
    for (bufp = buffer; *bufp; bufp++)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);
  }

  glPopMatrix();  /* Pop modelview */
  glMatrixMode(GL_PROJECTION);

  glPopMatrix();  /* Pop projection */
  glMatrixMode(GL_MODELVIEW);

  glPopAttrib();
}

/* Perform ADS - ambient, diffuse and specular - lighting calculation
 * in eye coordinates (EC).
 */
glm::vec3 computeLighting(glm::vec3 & rEC, glm::vec3 & nEC)
{
  if (debug[d_computeLighting]) {
    printf("rEC ");
    printVec(&rEC[0], 3);
    printf("nEC ");
    printVec(&nEC[0], 3);
  }

  // Used to accumulate ambient, diffuse and specular contributions
  // Note: it is a vec3 being constructed with a single value which
  // is used for all 3 components
  glm::vec3 color(0.0);

  // Ambient contribution: A=La×Ma
  // Default light ambient color and default ambient material color
  // are both (0.2, 0.2, 0.2)
  glm::vec3 La(0.2);
  glm::vec3 Ma(0.2);
  glm::vec3 ambient(La * Ma);
  color += ambient;

  // Light direction vector. Default for LIGHT0 is a directional light
  // along z axis for all vertices, i.e. <0, 0, 1>
  glm::vec3 lEC( 0.5, 0.5, 0.5 );

  // Test if normal points towards light source, i.e. if polygon
  // faces toward the light - if not then no diffuse or specular
  // contribution
  float dp = glm::dot(nEC, lEC);
  if (dp > 0.0) {
    // Calculate diffuse and specular contribution

    // Lambert diffuse: D=Ld×Md×cosθ
    // Ld: default diffuse light color for GL_LIGHT0 is white (1.0, 1.0, 1.0).
    // Md: default diffuse material color is grey (0.8, 0.8, 0.8).
    glm::vec3 Ld(0.0, 0.5, 0.5);
    glm::vec3 Md(0.8);
    // Need normalized normal to calculate cosθ,
    // light vector <0, 0, 1> is already normalized
    nEC = glm::normalize(nEC);
    float NdotL = glm::dot(nEC, lEC);
    glm::vec3 diffuse(Ld * Md * NdotL);
    color += diffuse;

    // Blinn-Phong specular: S=Ls×Ms×cosⁿα
    // Ls: default specular light color for LIGHT0 is white (1.0, 1.0, 1.0)
    // Ms: specular material color, also set to white (1.0, 1.0, 1.0),
    // but default for fixed pipeline is black, which means can't see
    // specular reflection. Need to set it to same value for fixed
    // pipeline lighting otherwise will look different.
    glm::vec3 Ls(0.8, 0.8, 0.8);
    glm::vec3 Ms(1.0);
    // Default viewer is at infinity along z axis <0, 0, 1> i.e. a
    // non local viewer (see glLightModel and GL_LIGHT_MODEL_LOCAL_VIEWER)
    glm::vec3 vEC(0.0, 0.0, 1.0);
    // Blinn-Phong half vector (using a single capital letter for
    // variable name!). Need normalized H (and nEC, above) to calculate cosα.
    glm::vec3 H(lEC + vEC);
    H = glm::normalize(H);
    float NdotH = glm::dot(nEC, H);
    if (NdotH < 0.0)
      NdotH = 0.0;
    glm::vec3 specular(Ls * Ms * powf(NdotH, g.shininess));
    color += specular;
  }

  return color;
}

/* ########## VBO SETUP, BINDING, UNDBINDING ########## */
void bindVBOs()
{
  // Generate buffers for verticies, indices and colors
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ibo);

  // Verticies
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), vertices, GL_STATIC_DRAW);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Enable pointers to vertex and normal coordinate arrays
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
}

void unbindVBOs()
{
  // Disable client states that were previously enabled
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  // Free memory allocated to indicies and vertices
  free(indices);
  free(vertices);

  // Unbind buffers of VBOs when switching rendering mode (empty them)
  int buffer;

  // [1]. Array Buffers (Verticies)
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
  if (buffer != 0)
     glBindBuffer(GL_ARRAY_BUFFER, 0);

  // [2]. Element Array Buffers (Indices)
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &buffer);
  if (buffer != 0)
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initGridVBO(int tess)
{
  /* NOTE: With VBOs, both the grid and sine wave have been drawn using GL_TRIANGLES
   * instead of GL_QUADS. The code for calculating index and storing indices is
   * mainly based on assignment 1. */

  // Set up variables (same as drawGrid)
  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  glm::vec3 r, n, rEC, nEC;
  float x, z;
  float stepSize = 2.0 / tess;

  // Calculate number of verts and indices to use in calculations
  numVerts = (tess + 1) * (tess + 1);
  numIndices = tess * tess * 6;
  // Allocate memory to indices and verties to place later in buffers
  vertices = (Vertex*) calloc(numVerts, sizeof(Vertex));
  indices = (unsigned int*) calloc(numIndices, sizeof(int));

  /* [1.] Store vertices
   * - Logic is essentially the same as drawGrid(), but we found the r.z += stepSize,
   * section wasn't required, so it was left out */
  for (size_t j = 0; j <= tess; ++j) {
    for (size_t i = 0; i <= tess; ++i) {
      r.x = -1.0 + i * stepSize;
      r.z = -1.0 + j * stepSize;
      r.y = 0.0;

      size_t index = j * (tess + 1) + i;
      rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
      vertices[index].pos = rEC;

      if (g.lighting) {
        n = glm::vec3(0.0, 1.0, 0.0);
        nEC = normalMatrix * glm::normalize(n);
        if (g.fixed) {
          vertices[index].normal = nEC;
        } else {
          glm::vec3 colors = computeLighting(rEC, nEC);
          vertices[index].color = colors;
        }
      }
      else
        vertices[index].color = cyan;
    }
  }

  // [2]. Store indices
  size_t index = 0;
  for (size_t i = 0; i < tess ; ++i) {
    for (size_t j = 0; j < tess; ++j) {
      indices[index++] = i * (tess + 1) + j;
      indices[index++] = (i + 1) * (tess + 1) + j;
      indices[index++] = i * (tess + 1) + j + 1;
      indices[index++] = i * (tess + 1) + j + 1;
      indices[index++] = (i + 1) * (tess + 1) + j;
      indices[index++] = (i + 1) * (tess + 1) + j + 1;
    }
  }
}

void initWaveVBO(int tess)
{
  // Same variables as initGridVBO() except time added for wave animation
  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  glm::vec3 r, n, rEC, nEC, lrEC, lnEC, c;
  float x, z;
  float stepSize = 2.0 / tess;
  float t = g.t;

  numVerts = (tess + 1) * (tess + 1);
  numIndices = tess * tess * 6;
  vertices = (Vertex*) calloc(numVerts, sizeof(Vertex));
  indices = (unsigned int*) calloc(numIndices, sizeof(int));

  /* [1]. Store vertices
   * Uses same logic as drawSineWave(), instead replacing calls such as:
   * - glColor3fv with vertices[index].color, as we are storing these values instead
   * - glDrawElements will pass then draw the shapes using these vertices, also passing
   * them into the shader, if enabled */
  for (size_t j = 0; j <= tess; ++j) {
    for (size_t i = 0; i <= tess; ++i) {
      r.x = -1.0 + i * stepSize;
      r.z = -1.0 + j * stepSize;

      if(g.waveDim == 2) {
        if (g.useShaders && g.fixed)
          r.y = 0.0;
        else {
          r.y = A1 * sinf(k1 * r.x + w1 * t);
          if (g.lighting) {
            n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
            n.y = 1.0;
            n.z = 0.0;
          }
        }
      } else if (g.waveDim == 3) {
        if (g.useShaders && g.fixed)
          r.y = 0.0;
        else {
          r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
          if (g.lighting) {
            n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
            n.y = 1.0;
            n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
          }
        }
      }

      // Calculate index to store vertex at
      size_t index = j * (tess + 1) + i;

      /* When shaders on, ignore modelViewMatrix and normal, since it is passed into shader
       * it is required however for lighting calculations (when fixed is off) */
      if (g.useShaders) {
        rEC = glm::vec3(glm::vec4(r, 1.0));
        nEC = glm::normalize(n);
        if(g.lighting) {
          if(g.fixed)
            vertices[index].normal = nEC;
          else {
            lrEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
            lnEC = normalMatrix * glm::normalize(n);
            c = computeLighting(lrEC, lnEC);
            glColor3fv(&c[0]);
          }
        }
      } else {
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        nEC = normalMatrix * glm::normalize(n);
        if(g.lighting) {
          if(g.fixed)
            vertices[index].normal = nEC;
          else {
            c = computeLighting(rEC, nEC);
            vertices[index].color = c;
          }
        }
        else
          vertices[index].color = cyan;
      }
      vertices[index].pos = rEC;

      /* Applicable only to 3D wave, essentially same calcuations as above, also
       * mirroring drawSineWave() */
      if (g.waveDim == 3) {
        if (g.useShaders && g.fixed)
          r.y = 0.0;
        else {
          r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
          if (g.lighting) {
            n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
          }
        }
      }

      if (g.useShaders) {
        rEC = glm::vec3(glm::vec4(r, 1.0));
        nEC = glm::normalize(n);
        if(g.lighting) {
          if(g.fixed)
            vertices[index].normal = nEC;
          else {
            lrEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
            lnEC = normalMatrix * glm::normalize(n);
            c = computeLighting(lrEC, lnEC);
            vertices[index].color = c;
          }
        }
      } else {
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        nEC = normalMatrix * glm::normalize(n);
        if(g.lighting) {
          if(g.fixed)
            vertices[index].normal = nEC;
          else {
            c = computeLighting(rEC, nEC);
            vertices[index].color = c;
          }
        }
        else
          vertices[index].color = cyan;
      }
      vertices[index].pos = rEC;
    }
  }

  // [2]. Store indices
  size_t index = 0;
  for (size_t i = 0; i < tess ; ++i) {
    for (size_t j = 0; j < tess; ++j) {
      indices[index++] = i * (tess + 1) + j;
      indices[index++] = (i + 1) * (tess + 1) + j;
      indices[index++] = i * (tess + 1) + j + 1;
      indices[index++] = i * (tess + 1) + j + 1;
      indices[index++] = (i + 1) * (tess + 1) + j;
      indices[index++] = (i + 1) * (tess + 1) + j + 1;
    }
  }
}

void initVBOs()
{
  // Determine appropriate shape to initialize
  if(g.wave)
    initWaveVBO(g.tess);
  else
    initGridVBO(g.tess);
}

void resetVBOS()
{
  /* Recalculate new values of VBO, used when tesselating, moving camera, animating
   * sine wave */
  unbindVBOs();
  initVBOs();
  bindVBOs();
}

void drawVBOShape()
{
  // Set up pointers to in order to draw verties and indices
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
  glNormalPointer(GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(sizeof(glm::vec3)));
  glColorPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(sizeof(glm::vec3) + sizeof(glm::vec3)));

  // Draw all elements specified via VBOs
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}

/* ########## DRAWING SHAPES (GRID/SINEWAVE) ########## */
void drawGrid(int tess)
{
  /* Since there are 4 types of views being displayed at once, vbo has to update for each window
   * shown in the multiView */
  if(g.vbo && g.multiView)
    resetVBOS();

  float stepSize = 2.0 / tess;
  glm::vec3 r, n, rEC, nEC;
  int i, j;

  if (g.lighting && g.fixed) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    if (g.twoside)
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, &cyanDiffuse[0]);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &grey[0]);
    glMaterialf(GL_FRONT, GL_SHININESS, g.shininess);
  } else {
    glDisable(GL_LIGHTING);
    glColor3fv(&cyan[0]);
  }

  if (g.wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Render using VBOs
  if (g.vbo) {
    drawVBOShape();
  }
  // Render via immediate mode
  else {
    for (j = 0; j < tess; j++) {
      glBegin(GL_QUAD_STRIP);
      for (i = 0; i <= tess; i++) {
        r.x = -1.0 + i * stepSize;
        r.y = 0.0;
        r.z = -1.0 + j * stepSize;

        if (g.lighting) {
          n.x = 0.0;
          n.y = 1.0;
          n.z = 0.0;
        }

        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        if (g.lighting) {
          nEC = normalMatrix * glm::normalize(n);
          if (g.fixed) {
            glNormal3fv(&nEC[0]);
          } else {
            glm::vec3 c = computeLighting(rEC, nEC);
            glColor3fv(&c[0]);
          }
        }
        glVertex3fv(&rEC[0]);

        r.z += stepSize;

        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        if (g.lighting) {
          nEC = normalMatrix * glm::normalize(n);
          if (g.fixed) {
            glNormal3fv(&nEC[0]);
          } else {
            glm::vec3 c = computeLighting(rEC, nEC);
            glColor3fv(&c[0]);
          }
        }
        glVertex3fv(&rEC[0]);
      }

      glEnd();
    }
  }

  if (g.lighting)
    glDisable(GL_LIGHTING);

  // Normals
  if (g.drawNormals) {
    for (j = 0; j <= tess; j++) {
      for (i = 0; i <= tess; i++) {
        r.x = -1.0 + i * stepSize;
        r.y = 0.0;
        r.z = -1.0 + j * stepSize;

        n.y = 1.0;
        n.x = 0.0;
        n.z = 0.0;
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        nEC = normalMatrix * glm::normalize(n);
        drawVector(rEC, nEC, 0.05, true, yellow);
      }
    }
  }

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("displaySineWave: %s\n", gluErrorString(err));
  }
}

void drawSineWave(int tess)
{
  /* [1]. Refresh sine wave only when not using shaders because if shaders are in use,
   * vbo doesn't need to be recalculated. The y values are instead updated in GPU.
   * [2]. When shaders off however, reset the vbo for when it is animating or if
   * the display is set to multiView, as there are 4 differing types that need to be
   * rendered */

  if(g.vbo && !g.useShaders) {
    if(g.animate || g.multiView)
      resetVBOS();
  }

  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  float stepSize = 2.0 / tess;
  glm::vec3 r, n, rEC, nEC, lrEC, lnEC, c;
  int i, j;
  float t = g.t;

  if (g.useShaders) {
    applyShading();
  }
  else if (g.lighting && g.fixed) {
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    if (g.flat)
      glShadeModel(GL_FLAT);
    else
      glShadeModel(GL_SMOOTH);
    if (g.twoside)
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, &cyanDiffuse[0]);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &grey[0]);
    glMaterialf(GL_FRONT, GL_SHININESS, g.shininess);
  } else {
    glDisable(GL_LIGHTING);
    glColor3fv(&cyan[0]);
  }

  if (g.wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Sine wave
  if (g.vbo)
    drawVBOShape();
  else {
    for (j = 0; j < tess; j++) {
      glBegin(GL_QUAD_STRIP);
      for (i = 0; i <= tess; i++) {
        r.x = -1.0 + i * stepSize;
        r.z = -1.0 + j * stepSize;

        if (g.waveDim == 2) {
          if (g.useShaders && g.fixed)
            r.y = 0.0;
          else {
            r.y = A1 * sinf(k1 * r.x + w1 * t);
            if (g.lighting) {
              n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
              n.y = 1.0;
              n.z = 0.0;
            }
          }
        } else if (g.waveDim == 3) {
          if (g.useShaders && g.fixed)
            r.y = 0.0;
          else {
            r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
            if (g.lighting) {
              n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
              n.y = 1.0;
              n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
            }
          }
        }

        /* When shaders on: (reiterate)
         * - Pass in pos/normal eye coordinate without modelViewMatrix and normalMatrix
         * - When lighting is on however, it requires normal and modelViewMatrix*/
        if (g.useShaders) {
          rEC = glm::vec3(glm::vec4(r, 1.0));
          nEC = glm::normalize(n);
          if(g.lighting) {
            if(g.fixed)
              glNormal3fv(&nEC[0]);
            else {
              lrEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
              lnEC = normalMatrix * glm::normalize(n);
              c = computeLighting(lrEC, lnEC);
              glColor3fv(&c[0]);
            }
          }
        } // Shaders off, compute normally
        else {
          rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
          nEC = normalMatrix * glm::normalize(n);
          if(g.lighting) {
            if(g.fixed)
              glNormal3fv(&nEC[0]);
            else {
              c = computeLighting(rEC, nEC);
              glColor3fv(&c[0]);
            }
          }
        }
        glVertex3fv(&rEC[0]);

        r.z += stepSize;

        if (g.waveDim == 3) {
          if (g.useShaders && g.fixed)
            r.y = 0.0;
          else {
            r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
            if (g.lighting) {
              n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
            }
          }
        }

        if (g.useShaders) {
          rEC = glm::vec3(glm::vec4(r, 1.0));
          nEC = glm::normalize(n);
          if(g.lighting) {
            if(g.fixed)
              glNormal3fv(&nEC[0]);
            else {
              lrEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
              lnEC = normalMatrix * glm::normalize(n);
              c = computeLighting(lrEC, lnEC);
              glColor3fv(&c[0]);
            }
          }
        } else {
          rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
          nEC = normalMatrix * glm::normalize(n);
          if(g.lighting) {
            if(g.fixed)
              glNormal3fv(&nEC[0]);
            else {
              c = computeLighting(rEC, nEC);
              glColor3fv(&c[0]);
            }
          }
        }
        glVertex3fv(&rEC[0]);
      }
      glEnd();
    }
  }

  // Disable use of shaders if originally enabled
  if(g.useShaders)
    glUseProgram(0);
  if (g.lighting)
    glDisable(GL_LIGHTING);

  // Normals
  if (g.drawNormals) {
    for (j = 0; j <= tess; j++) {
      for (i = 0; i <= tess; i++) {
        r.x = -1.0 + i * stepSize;
        r.z = -1.0 + j * stepSize;

        n.y = 1.0;
        n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
        if (g.waveDim == 2) {
          r.y = A1 * sinf(k1 * r.x + w1 * t);
          n.z = 0.0;
        } else {
          r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
          n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
        }

        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
        nEC = normalMatrix * glm::normalize(n);
        drawVector(rEC, nEC, 0.05, true, yellow);
      }
    }
  }

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("displaySineWave: %s\n", gluErrorString(err));
  }
}

void idle()
{
  float t, dt;

  t = glutGet(GLUT_ELAPSED_TIME) / milli;

  // Accumulate time if animation enabled
  if (g.animate) {
    dt = t - g.lastT;
    g.t += dt;
    g.lastT = t;
    if (debug[d_animation])
      printf("idle: animate %f\n", g.t);
  }

  // Update stats, although could make conditional on a flag set interactively
  dt = (t - g.lastStatsDisplayT);
  if (dt > g.displayStatsInterval) {
    g.frameRate = g.frameCount / dt;
    g.lastStatsDisplayT = t;
    g.frameCount = 0;
    if (g.consolePM)
      consolePM();
  }

  glutPostRedisplay();
}

/* ########## DISPLAY BETWEEN MULTIVIEW/SINGLE ########## */
void displayMultiView()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glm::mat4 modelViewMatrixSave(modelViewMatrix);
  glm::mat3 normalMatrixSave(normalMatrix);

  // Front view
  modelViewMatrix = glm::mat4(1.0);
  glViewport(g.width / 16.0, g.height * 9.0 / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (!g.wave)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);


  // Top view
  modelViewMatrix = glm::mat4(1.0);
  modelViewMatrix = glm::rotate(modelViewMatrix, glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
  glViewport(g.width / 16.0, g.height / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16);
  drawAxes(5.0);
  if (!g.wave)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  // Left view
  modelViewMatrix = glm::mat4(1.0);
  modelViewMatrix = glm::rotate(modelViewMatrix, glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
  glViewport(g.width * 9.0 / 16.0, g.height * 9.0 / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (!g.wave)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  // General view
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateX * glm::pi<float>() / 180.0f, glm::vec3(1.0, 0.0, 0.0));
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateY * glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
  modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(camera.scale));
  normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));
  glViewport(g.width * 9.0 / 16.0, g.width / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (!g.wave)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  if (g.displayOSD)
    displayOSD();

  g.frameCount++;

  glutSwapBuffers();

}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);

  glViewport(0, 0, g.width, g.height);

  // General view
  modelViewMatrix = glm::mat4(1.0);
  normalMatrix = glm::mat3(1.0);

  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateX * glm::pi<float>() / 180.0f, glm::vec3(1.0, 0.0, 0.0));
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateY * glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
  modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(camera.scale));

  normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));

  if (debug[d_matrices]) {
    printf("modelViewMatrix\n");
    printMatrixColumnMajor(&modelViewMatrix[0][0], 4);
    printf("normalMatrix\n");
    printMatrixColumnMajor(&normalMatrix[0][0], 3);
  }

  drawAxes(5.0);
  if (!g.wave)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  if (g.displayOSD)
    displayOSD();

  glutSwapBuffers();

  g.frameCount++;

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("display: %s\n", gluErrorString(err));
  }
}

/* ########## USER INPUT ########## */
void keyboard(unsigned char key, int x, int y)
{
  /* Three states for osd (able to toggle between them)
   * 1. Frame related information
   * 2. Flags (that have been set/unset)
   * 3. Values (shininess, tesselation, dimension for sine wave)
   */
  const char* osd[] = { "FRAME", "FLAGS", "VALUES" };

  switch (key) {
  case 27: //quit
    printf("exit\n");
    exit(0);
    break;
  case 'a': //animation
    if (g.wave) {
      g.animate = !g.animate;
      if (g.animate) {
        g.lastT = glutGet(GLUT_ELAPSED_TIME) / milli;
      }
    }
    printf("animation: %s\n", g.animate?"true":"false");
    break;
  case 'b': //smooth/flat shading
    g.flat = !g.flat;
    printf("flat: %s\n", g.flat?"true":"false");
    break;
  case 'c': //console display
    g.consolePM = !g.consolePM;
    g.displayOSD = !g.displayOSD;
    printf("console: %s\n", g.consolePM?"true":"false");
    break;
  case 'd': //directional/positional lighting
    g.positional = !g.positional;
    printf("positional: %s\n", g.positional?"true":"false");
    break;
  case 'f': //gpu/cpu lighting
    g.fixed = !g.fixed;
    printf("fixed: %s\n", g.fixed?"true":"false");
    break;
  case 'g': //shaders
    g.useShaders = !g.useShaders;
    printf("shaders: %s\n", g.useShaders?"true":"false");
    break;
  case 'H': //increase shininess
    g.shininess += 5.0;
    if (g.shininess > 125.0)
      g.shininess = 125.0;
    printf("shininess: %.1f\n", g.shininess);
    break;
  case 'h': //decrease shininess
    g.shininess -= 5.0;
    if (g.shininess < 5.0)
      g.shininess = 5.0;
    printf("shininess: %.1f\n", g.shininess);
    break;
  case 'l': //lighting
    g.lighting = !g.lighting;
    printf("lighting: %s\n", g.lighting?"true":"false");
    break;
  case 'm': //specular lighting mode (Blinn-Phong/Phong)
    g.phong = !g.phong;
    printf("phong: %s\n", g.phong?"true":"false");
    break;
  case 'n': //normals
    g.drawNormals = !g.drawNormals;
    printf("normals: %s\n", g.drawNormals?"true":"false");
    break;
  case 'o': // cycle OSD options (enum)
    g.option = static_cast<OSD>(g.option+1);
    if (g.option > VALUES)
      g.option = FRAME;
    printf("osd: %s\n", osd[g.option]);
    break;
  case 'p': //per (vertex/pixel) lighting
    g.perPixel = !g.perPixel;
    printf("per pixel: %s\n", g.perPixel?"true":"flase");
    break;
  case 's': //shape change
    g.wave = !g.wave;
    if (!g.wave)
      g.animate = false;
    printf("wave: %s\n", g.wave?"true":"false");
    break;
  case 'v': //VBO mode
    g.vbo = !g.vbo;
    //assumes vbo is initally off: initalize and bind (if default on, use resetVBOS())
    if (g.vbo) {
      initVBOs();
      bindVBOs();
    }
    else
      unbindVBOs();
    printf("vbo: %s\n", g.vbo?"true":"false");
    break;
  case 'w': //wireframe
    g.wireframe = !g.wireframe;
    printf("wireframe: %s\n", g.wireframe?"true":"false");
    break;
  case 'z': //2D/3D wave
    g.waveDim++;
    if (g.waveDim > 3)
      g.waveDim = 2;
    printf("dimension: %d\n", g.waveDim);
    break;
  case '4': //multiview
    g.multiView = !g.multiView;
    if (g.multiView)
      glutDisplayFunc(displayMultiView);
    else
      glutDisplayFunc(display);
    printf("multiview: %s\n", g.multiView?"true":"false");
    break;
  case '+': //increase tesselation
    g.tess *= 2;
    printf("tesselation: %d\n", g.tess);
    break;
  case '-': //decrease tesselation
    g.tess /= 2;
    if (g.tess < 8)
      g.tess = 8;
    printf("tesselation: %d\n", g.tess);
    break;
  default:
    break;
  }

  if (g.vbo)  // Recalculate VBOs due to mode change
    resetVBOS();
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  if (debug[d_mouse])
    printf("mouse: %d %d %d\n", button, x, y);

  camera.lastX = x;
  camera.lastY = y;

  if (state == GLUT_DOWN)
    switch(button) {
    case GLUT_LEFT_BUTTON:
      camera.control = rotate;
      break;
    case GLUT_MIDDLE_BUTTON:
      camera.control = pan;
      break;
    case GLUT_RIGHT_BUTTON:
      camera.control = zoom;
      break;
    }
  else if (state == GLUT_UP)
    camera.control = inactive;
}

void motion(int x, int y)
{
  float dx, dy;

  if (debug[d_mouse]) {
    printf("motion: %d %d\n", x, y);
    printf("camera.rotate: %f %f\n", camera.rotateX, camera.rotateY);
    printf("camera.scale:%f\n", camera.scale);
  }

  dx = x - camera.lastX;
  dy = y - camera.lastY;
  camera.lastX = x;
  camera.lastY = y;

  switch (camera.control) {
  case inactive:
    break;
  case rotate:
    camera.rotateX += dy;
    camera.rotateY += dx;
    break;
  case pan:
    break;
  case zoom:
    camera.scale += dy / 100.0;
    break;
  }

  if(g.vbo) // When vbos on, recalculate when moving camera around
    resetVBOS();

  glutPostRedisplay();
}

/* ########## MAIN ########## */
int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (1024, 1024);
  glutInitWindowPosition (100, 100);
  glutCreateWindow (argv[0]);
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutMainLoop();
  return 0;
}
