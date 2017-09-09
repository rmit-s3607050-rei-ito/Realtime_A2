//shader.frag
void main (void)
{
  vec4 red = vec4(1, 0, 0, 1);
  vec4 green = vec4(0, 1, 0, 1);

  float depth = gl_FragCoord.z;

  if(depth < 0.5)
    gl_FragColor = red;     // 0.0 to ~0.49
  else
    gl_FragColor = green;   // 0.5 to 1.0
}
