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


QUESTIONS & ANSWERS
1. Is there any visual difference between fixed pipeline and shader lighting? Should there be?
There's no visual difference between the two lighting modes, since the lighting calculations
that are made are completely identical but the location of where the calculations are being made
are different; fixed being done in the CPU and the shader done on the GPU.

2. Is there any performance difference between fixed pipeline and using shader lighting?
There is a performance difference since putting the lighting calculations on the GPU puts less processing
on the CPU, increasing the overall performance. Since the GPU has faster processing, mainly dedicated to
graphics calculations,

3. What overhead/slowdown factor is there for performing animation compared to static geometry using the vertex shader? For static i.e. geometry is it worth pre-computing and storing the sine wave values in buffers?


4. Is there any difference in performance between per vertex and per pixel shader based lighting?
There is since the calculations that have to be made for per-pixel lighting occur much more frequently
in comparison to per-vertex, making it appear much more smooth but taking a performance hit.

5. What is the main visual difference between Phong and Blinn-Phong lighting?
The main difference between Phong and Blinn-Phong lighting is how the specular reflections look.
Phong is more concentrated since it's using the reflection vector instead of the half vector that is used for BLinn-Phong making Phong more suited to handling highlights.
