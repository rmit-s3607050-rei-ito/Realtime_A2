//shader.frag

uniform float fragShininess;
uniform bool fragPhong, fragPixel, lighting;

varying vec3 color, normal;
varying vec4 position;

vec3 fragColor;

vec3 computePixelLighting(vec4 rEC, vec3 nEC) {

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

    if (fragPhong) {
      vec3 R = reflect(vEC, nEC);
      float NdotR = dot(nEC, R);
      if (NdotR < 0.0)
        NdotR = 0.0;
      vec3 specular = (Ls * Ms * pow(NdotR, fragShininess)); //calculate specular
      color += specular; //add specular to final color
    }
    else {
      vec3 H = (lEC + vEC);
      H = normalize(H);
      float NdotH = dot(nEC, H);
      if (NdotH < 0.0)
        NdotH = 0.0;
      vec3 specular = (Ls * Ms * pow(NdotH, fragShininess)); //calculate specular
      color += specular; //add specular to final color
    }
  }

  return color;
}

void main (void)
{
  fragColor = computePixelLighting(position, normal);

  if (lighting) {
    if (fragPixel)
      gl_FragColor = vec4(fragColor, 1);
    else
      gl_FragColor = vec4(color, 1);
  }
  else
    gl_FragColor = vec4(vec3(0.0), 1);
}
