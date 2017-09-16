//shader.vert

#define M_PI 3.1415926535897932384626433832795

uniform int uTesselation, uDimention;
uniform float uShininess, uTime;
uniform bool uPhong, uPixel, uPositional, uFixed, uFlat, uLighting;
uniform mat3 uNormalMat;
uniform mat4 uModelViewMat, uProjectionMat;

varying vec3 vColor, vPosition, vNormal;

vec3 computeVertexLighting(vec3 rEC, vec3 nEC)
{
  vec3 color = vec3(0.0); //final return color to be used

  vec3 La = vec3(0.2); //ambient intensity
  vec3 Ma = vec3(0.2); //ambient reflection coefficient
  vec3 ambient = (La * Ma); //calculate ambient
  color += ambient; //add ambient to final color

  vec3 lEC = vec3 ( 0.0, 0.0, 1.0 ); //light position
  if (uPositional)
    lEC = normalize(lEC - rEC);
  else
    normalize(lEC);

  float dp = dot(nEC, lEC); //dot product between light & scene normals (lambertion)
  if (dp > 0.0) {
    vec3 Ld = vec3(0.0, 0.5, 0.5); //intensity of the (point) light source
    vec3 Md = vec3(0.8); //diffuse reflection coefficient

    nEC = normalize(nEC); //normalize scene normals
    float NdotL = dot(nEC, lEC); //dot product between normalized scene normals & light
    vec3 diffuse = (Ld * Md * NdotL); //calculate diffuse
    color += diffuse; //add diffuse to final color

    vec3 Ls = vec3(0.8); //intensity of the (point) light source
    vec3 Ms = vec3(1.0); //specular reflection coefficient

    vec3 vEC = vec3(0.0, 0.0, 1.0); //viewer direction
    if (uPositional)
      vEC = normalize(vEC - rEC);
    else
      normalize(vEC);

    if (uPhong) { //Phong lighting
      vec3 R = reflect(lEC, nEC);
      R = normalize(-R);
      float VdotR = dot(vEC, R);
      if (VdotR < 0.0)
        VdotR = 0.0;
      vec3 specular = (Ls * Ms * pow(VdotR, uShininess)); //calculate specular
      color += specular; //add specular to final color
    }
    else { //Blinn-Phong lighting
      vec3 H = (lEC + vEC);
      H = normalize(H);
      float NdotH = dot(nEC, H);
      if (NdotH < 0.0)
        NdotH = 0.0;
      vec3 specular = (Ls * Ms * pow(NdotH, uShininess)); //calculate specular
      color += specular; //add specular to final color
    }
  }

  return color;
}

vec4 calcSineYValue()
{
  vec4 v = gl_Vertex;

  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;

  if (uDimention == 2) {
    v.y = A1 * sin(k1 * v.x + w1 * uTime);
  } else if (uDimention == 3) {
    v.y = A1 * sin(k1 * v.x + w1 * uTime) + A2 * sin(k2 * v.z + w2 * uTime);
  }

  return v;
}

vec3 calcNormals(vec4 vector)
{
  vec3 n;

  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;

  if (uDimention == 2) {
    if (uLighting) {
      n.x = - A1 * k1 * cos(k1 * vector.x + w1 * uTime);
      n.y = 1.0;
      n.z = 0.0;
    }
  } else if (uDimention == 3) {
    if (uLighting) {
      n.x = - A1 * k1 * cos(k1 * vector.x + w1 * uTime);
      n.y = 1.0;
      n.z = - A2 * k2 * cos(k2 * vector.z + w2 * uTime);
    }
  }

  return n;
}

void main(void)
{
  vec4 osVert = calcSineYValue();
  vec4 esVert = uModelViewMat * osVert;
  vec4 csVert = uProjectionMat * esVert;
  gl_Position = csVert;

  vPosition = vec3(esVert);
  vNormal = calcNormals(osVert);

  if (uFixed && !uPixel)
    vColor = computeVertexLighting(vPosition, uNormalMat * normalize(vNormal));
  else
    vColor = vec3(gl_Color);
}
