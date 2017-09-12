//shader.vert

uniform vec3 uColor;        // uniform = pulled from sinewave3D-glm.cpp file
uniform mat4 mvMat;
varying vec3 vColor;        // varying = shared with shader.frag

void main(void)
{
  vec4 esVert = gl_Vertex;
  vec4 csVert = gl_ProjectionMatrix * esVert;
  gl_Position = csVert;

  vColor = uColor;
}
