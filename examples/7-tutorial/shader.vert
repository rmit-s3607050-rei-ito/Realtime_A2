//shader.vert

#define M_PI 3.1415926535897932384626433832795

// uniform = values obtained from sinewave3D-glm.cpp file
// [VALUES]
uniform mat3 nMat;          // normalMatrix
uniform mat4 mvMat;         // modelViewMatrix
uniform mat4 pMat;          // projectionMatrix
uniform int ts;             // tesselation
uniform int waveDim;
uniform float time;

// varying = values shared with shader.frag
varying vec3 vColor;

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
  vec4 v = gl_Vertex;

  // CALCULATE Y VALUES IN HERE, X AND Z TO BE CALCULATED IN CPU AND PASSED HERE
  /* In drawSineWave, stepSize was calculated via dividing float by int, that does
   * not work in this case, so tess is converted to a float.
   */
  float tesselation = float(ts);
  float stepSize = 2.0 / tesselation;

  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  int i, j;
  vec3 rEC, nEC;

  vec4 esVert, csVert;

  // Sine wave
  for (j = 0; j < ts; j++){
    for (i = 0; i <= ts; i++){
      if (waveDim == 2) {
        v.y = A1 * sin(k1 * v.x + w1 * time);
      } else if (waveDim == 3) {
        v.y = A1 * sin(k1 * v.x + w1 * time) + A2 * sin(k2 * v.z + w2 * time);
      }

      esVert = mvMat * v;
      csVert = pMat * esVert;
      gl_Position = csVert;

      vColor = computeLighting(esVert, nMat * normalize(gl_Normal));
    }
  }

  //calculateSineWave(waveDim, ts, time);

  //vec4 esVert = mvMat * gl_Vertex;
  //vec4 csVert = pMat * esVert;
  //gl_Position = csVert;

  //vColor = computeLighting(esVert, nMat * normalize(gl_Normal));
}
