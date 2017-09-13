# Realtime_A2

Students:
Rei Ito - s3607050
Pacific Thai - s3429648

Questions:
Is there any visual difference between fixed pipeline and shader lighting? Should there be?


Is there any performance difference between fixed pipeline and using shader lighting?


What overhead/slowdown factor is there for performing animation compared to static geometry using the vertex shader? For static i.e. geometry is it worth pre-computing and storing the sine wave values in buffers?

Is there any difference in performance between per vertex and per pixel shader based lighting?
There is since the calculations that have to be made for per-pixel lighting occur much more frequently
in comparison to per-vertex, making it appear much more smooth but taking a performance hit.

What is the main visual difference between Phong and Blinn-Phong lighting?
The main difference between Phong and Blinn-Phong lighting is how the specular reflections look.
