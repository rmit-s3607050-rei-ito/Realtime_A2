//shader.vert

uniform float uShininess;
uniform bool uPhong, uPixel, uPositional, uFixed, uFlat;

varying vec3 vColor, vPosition, vNormal;

vec3 computeVertexLighting(vec3 rEC, vec3 nEC) {

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
    vec3 Ld = vec3(1.0); //intensity of the (point) light source
    vec3 Md = vec3(0.8); //diffuse reflection coefficient

    nEC = normalize(nEC); //normalize scene normals
    float NdotL = dot(nEC, lEC); //dot product between normalized scene normals & light
    vec3 diffuse = (Ld * Md * NdotL); //calculate diffuse
    color += diffuse; //add diffuse to final color

    vec3 Ls = vec3(1.0); //intensity of the (point) light source
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

void main(void)
{
  // os - object space, es - eye space, cs - clip space
  vec4 osVert = gl_Vertex;
  vec4 esVert = gl_ModelViewMatrix * osVert;
  vec4 csVert = gl_ProjectionMatrix * esVert;
  gl_Position = csVert;
  // Equivalent to: gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex

  vPosition = vec3(osVert);
  vNormal = gl_Normal;

  if (uFixed && !uPixel)
    vColor = computeVertexLighting(vPosition, vNormal);
  else
    vColor = vec3(gl_Color);
}
