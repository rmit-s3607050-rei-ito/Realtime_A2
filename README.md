README
COSC1226 Real-Time Rendering and 3D Games Programming Assignment 2

AUTHORS
Rei Ito - s3607050
Pacific Thai - s3429648

CONTRIBUTORS
Shaders built with the help of 'sinewave3D-glm.cpp'.
'shaders.c' + 'shaders.h', adopted from code written by pknowels, was used to load shaders.

FILES
Makefile
shader.frag
shader.vert
shaders.c
shaders.h
sinewave3D-glm.cpp

INSTALL
To be run on linux systems:
make
./sinewave

BUGS
- Unsure on whether the directional/positional lighting in the shader is correct.
- Couldn't get the fixed pipeline lighting to have the exact same color values,
  than the CPU calculation colors.

QUESTIONS & ANSWERS
1. Is there any visual difference between fixed pipeline and shader lighting? Should there be?
There's slight visual difference between the two lighting modes mainly being that fixed is lighter.
This must mean the lighting calculations that are made between the two must differ. Fixed pipeline
must have attenuation applied making the shading more accurate.

2. Is there any performance difference between fixed pipeline and using shader lighting?


3. What overhead/slowdown factor is there for performing animation compared to static geometry using the vertex shader? For static i.e. geometry is it worth pre-computing and storing the sine wave values in buffers?


4. Is there any difference in performance between per vertex and per pixel shader based lighting?
There is a slight performance difference since the calculations that have to be made for per-pixel
lighting occur much more frequently in comparison to per-vertex, making it appear much more smooth
but taking a performance hit.

5. What is the main visual difference between Phong and Blinn-Phong lighting?
The main difference between Phong and Blinn-Phong lighting is how the specular reflections look.
Phong is more concentrated  (thinner) since it's using the reflection vector instead of the half vector that is used for Blinn-Phong making Phong more suited to handling highlights.
