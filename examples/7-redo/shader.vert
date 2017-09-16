//shader.vert
varying vec3 color;

#define M_PI 3.1415926535897932384626433832795

uniform mat4 mvMat;
uniform mat3 nMat;
uniform mat4 pMat;

uniform int ts;
uniform int waveDim;
uniform float time;

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

void calcSineYValue()
{
  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  int i,j;

  float stepSize = 2.0 / float(ts);

  for (j = 0; j < ts; j++){
    for (i = 0; i <= ts; i++){
      if (waveDim == 2) {
        gl_Vertex.y = A1 * sin(k1 * gl_Vertex.x + w1 * time);
      } else if (waveDim == 3) {
        gl_Vertex.y = A1 * sin(k1 * gl_Vertex.x + w1 * time) + A2 * sin(k2 * gl_Vertex.z + w2 * time);
      }
    }
  }
}

void main(void)
{
  calcSineYValue();

  // os - object space, es - eye space, cs - clip space
  vec4 osVert = gl_Vertex;
  vec4 esVert = mvMat * osVert;
  vec4 csVert = pMat * esVert;
  gl_Position = csVert;
  // Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex
  gl_Normal = nMat * normalize(gl_Normal);

  color = computeLighting(esVert, gl_Normal);
}
