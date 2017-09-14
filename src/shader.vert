//shader.vert

uniform float shininess;
uniform bool phong, perPixel;

varying vec3 color, normal;
varying vec4 position;

vec3 computeVertexLighting(vec4 rEC, vec3 nEC) {

  vec3 color = vec3(0.0); //final return color to be used

  vec3 La = vec3(0.2); //ambient intensity
  vec3 Ma = vec3(0.2); //ambient reflection coefficient
  vec3 ambient = (La * Ma); //calculate ambient
  color += ambient; //add ambient to final color

  vec3 lEC = vec3 ( 0.0, 0.0, 1.0 ); //light position

  float dp = dot(nEC, lEC); //dot product between light & scene normals (lambertion)
  if (dp > 0.0) {
    vec3 Ld = vec3(1.0); //intensity of the (point) light source
    vec3 Md = vec3(0.8); //diffuse reflection coefficient

    nEC = normalize(nEC); //normalize scene normals
    float NdotL = dot(nEC, lEC); //dot product between normalized scene normals & light
    vec3 diffuse = (Ld * Md * NdotL); //calculate diffuse
    color += diffuse; //add diffuse to final color

    vec3 Ls = vec3(1.0); //intensity of the (point) light source
    vec3 Ms = vec3(1.0); //specular reflection coefficient

    vec3 vEC = vec3(0.0, 0.0, 1.0); //viewer direction

    if (phong) {
      vec3 R = reflect(vEC, nEC);
      float NdotR = dot(nEC, R);
      if (NdotR < 0.0)
        NdotR = 0.0;
      vec3 specular = (Ls * Ms * pow(NdotR, shininess)); //calculate specular
      color += specular; //add specular to final color
    }
    else {
      vec3 H = (lEC + vEC);
      H = normalize(H);
      float NdotH = dot(nEC, H);
      if (NdotH < 0.0)
        NdotH = 0.0;
      vec3 specular = (Ls * Ms * pow(NdotH, shininess)); //calculate specular
      color += specular; //add specular to final color
    }
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
  position = gl_Position;
  normal = gl_Normal;
  // Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex
  if (!perPixel)
    color = computeVertexLighting(gl_Position, gl_Normal);
}
