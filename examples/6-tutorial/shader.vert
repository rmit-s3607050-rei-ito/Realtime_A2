//shader.vert
varying vec3 color;

vec3 computeLighting(vec4 rEC, vec3 nEC) {
  float shininess = 50.0;
  // Using computeLighting() from sinewave3D-glm.cpp (modified to work)

  vec3 color = vec3(0.0);

  vec3 La = vec3(0.2);
  vec3 Ma = vec3(0.2);
  vec3 ambient = (La * Ma);
  color += ambient;

  vec3 lEC = vec3 ( 0.0, 0.0, 1.0 );

  float dp = dot(nEC, lEC);
  if (dp > 0.0) {
    vec3 Ld = vec3(1.0);
    vec3 Md = vec3(0.8);

    nEC = normalize(nEC);
    float NdotL = dot(nEC, lEC);
    vec3 diffuse = (Ld * Md * NdotL);
    color += diffuse;

    vec3 Ls = vec3(1.0);
    vec3 Ms = vec3(1.0);

    vec3 vEC = vec3(0.0, 0.0, 1.0);

    vec3 H = (lEC + vEC);
    H = normalize(H);
    float NdotH = dot(nEC, H);
    if (NdotH < 0.0)
      NdotH = 0.0;
    vec3 specular = (Ls * Ms * pow(NdotH, shininess));
    color += specular;
  }

  return color;
}

void main(void)
{
  // os - object space, es - eye space, cs - clip space
  vec4 osVert = gl_Vertex;
  vec4 esVert = gl_ModelViewMatrix * osVert;
  vec4 csVert = gl_ProjectionMatrix * esVert;
  gl_Position = csVert;
  // Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex

  color = computeLighting(esVert, gl_Normal);
}
