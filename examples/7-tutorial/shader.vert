//shader.vert

uniform vec3 uColor;        // uniform = pulled from sinewave3D-glm.cpp file
uniform mat3 nMat;          // normalMatrix
uniform mat4 mvMat;         // modelViewMatrix
varying vec3 vColor;        // varying = shared with shader.frag

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
  vec4 esVert = mvMat * gl_Vertex;
  vec4 csVert = gl_ProjectionMatrix * esVert;
  gl_Position = csVert;

  //vColor = uColor;
  //vColor = computeLighting(esVert, gl_Normal);
  vColor = computeLighting(esVert, nMat * normalize(gl_Normal));
}
