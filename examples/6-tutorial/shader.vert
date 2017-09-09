//shader.vert
void main(void)
{
  // os - object space, es - eye space, cs - clip space
  vec4 osVert = gl_Vertex;
  vec4 esVert = gl_ModelViewMatrix * osVert;
  vec4 csVert = gl_ProjectionMatrix * esVert;
  gl_Position = csVert;
  // Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex

  // Varying float depth
  float depth = -esVert.z;
}
