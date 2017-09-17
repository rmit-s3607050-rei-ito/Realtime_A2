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
- flat shading (when shaders on), is not working

QUESTIONS & ANSWERS
1. Is there any visual difference between fixed pipeline and shader lighting? Should there be?

There's slight visual difference between the two lighting modes mainly being that fixed is lighter.
This must mean the lighting calculations that are made between the two must differ. Fixed pipeline
must have attenuation applied making the shading more accurate.

2. Is there any performance difference between fixed pipeline and using shader lighting?
There is a massive performance difference between the two lighting modes, shader bolstering performance
when fixed pipeline is also on by ~4x. By itself without fixed pipeline the shader doesn't have any other difference
compared to when both are off. This is probably because the lighting calculations for fixed are either heavier than the shader or are being done on the CPU, compared to the GPU. The fixed pipeline itself will boost performance by 2x.

We tested this using: Tesselation = 128, Filled Mode, 2D-Sine Wave
 a. Fixed pipeline off and Shader off = ~40 frame rate
 b. Fixed pipeline off and Shader on  = ~40 frame rate
 c. Fixed pipeline on and Shader off  = ~70 frame rate
 d. Fixed pipeline on and Shader on   = ~300 frame rate

3. What overhead/slowdown factor is there for performing animation compared to static geometry using the vertex shader? For static i.e. geometry is it worth pre-computing and storing the sine wave values in buffers?

Performing small tests we have:
  a. [Vertex shader: ON, 2D-Wave, filled, static]   = f/s = ~40 (128 tesselation), at ~= 540 to 550 (32 tesselation)
  b. [Vertex shader: ON, 2D-Wave, filled, animated] = f/s = ~40 (128 tesselation), at ~= 530 to 540 (32 tesselation)
Regardless of whether the wave is animated or not, there is little to no difference in performance (at 32 tesselation its a 10 fps jump but miniscule non the less).
This is because irregardless of whether it is animated or not, the y value of the sine wave is calculated in the gpu.
Having time vary for animation doesn't affect calculations to produce a large noticable difference.

When turning VBOs on (pre-computing + buffer use) we get:
  c. [Vertex shader: ON, 2D-Wave, filled, static]   = frame rate ~4300 to 4500 (32 Tesselation)
  d. [Vertex shader: ON, 2D-Wave, filled, animated] = frame rate ~4300 to 4400 (32 Tesselation)
These also illustrate the difference between having an animated vs static sine wave drawn when comparing performance (where there isn't that noticable of a change).
However pre-computing the x and z coordinates of the geometry itself significantly improves the performance by ~8 fold (from ~500 to ~4000).
Without the use of shaders and using vbos, there will be a difference between an animated vs static wave (as the vbo has to be recalculated every frame due to the y values changing).
But by combining the vbo with the shader this is not the case. At every frame the vbo doesn't need to be recalculated since all it does is
pass in the x and z coordinates (which stay the same), the y values are instead calculated on the gpu.  

So essentially when shaders are on:
 - Static vs Animated sine wave     = Little to no difference.
 - VBOs vs immediate mode rendering = ~8 fold jump in performance

4. Is there any difference in performance between per vertex and per pixel shader based lighting?

In theory there is a difference in performance difference since the calculations that have to be made for
per-pixel lighting occur much more frequently in comparison to per-vertex since it happens in the
fragment shader, while making making it appear much more smooth. But testing it with the wave, there
was harly any difference in the framerate leading me to believe that modern GPUs can handle the increased
calculation frequency although in larger applications it would be interesting to see if it does effect
performance.

5. What is the main visual difference between Phong and Blinn-Phong lighting?

The main difference between Phong and Blinn-Phong lighting is how the specular reflections look.
Phong is more concentrated  (thinner) since it's using the reflection vector instead of the half vector that is used for Blinn-Phong making Phong more suited to handling highlights.
