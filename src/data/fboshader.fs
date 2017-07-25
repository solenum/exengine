#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_position;
uniform sampler2D u_norm;
uniform sampler2D u_colorspec;

uniform vec3 u_view_position;

/* point light */
const int MAX_PL = 64;
struct point_light {
  vec3 position;
  vec3 color;
  bool is_shadow;
};
uniform point_light u_point_lights[MAX_PL];
uniform samplerCube u_point_depth; 
uniform int  u_pl_count;
uniform bool u_point_active;
/* ------------ */

vec3 calc_point_light(point_light l)
{
  // point light
  vec3 fragpos = texture(u_position, uv).rgb;
  vec3 normals = texture(u_norm, uv).rgb;
  vec3 diff    = texture(u_colorspec, uv).rgb;
  float spec   = texture(u_colorspec, uv).a;

  vec3 view_dir  = normalize(u_view_position - fragpos);
  float distance = length(l.position - fragpos);
  vec3 light_dir = normalize(l.position - fragpos);
  
  // diffuse
  vec3 diffuse   = max(dot(light_dir, normals), 0.0) * diff * l.color;

  // specular
  vec3 halfwayd  = normalize(light_dir + view_dir);
  float specs    = pow(max(dot(normals, halfwayd), 0.0), 64.0);
  vec3 specular = l.color * (2*specs) * spec;

  // attenuation
  float attenuation = 1.0f / (1.0f + 0.064f * (distance * distance));
  diffuse  *= attenuation;
  specular *= attenuation;
  return vec3(diffuse + specular);
}

vec3 aces_tonemap(vec3 x) {
  float a = 2.51;
  float b = 0.03;
  float c = 2.43;
  float d = 0.59;
  float e = 0.14;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

const vec3 u_white_point = vec3(0.75, 0.75, 0.75);

void main()
{
  /*vec3 tex_color = textureLod(u_texture, uv, 0.0).rgb;
  tex_color = vec3(1.0) - exp(-tex_color / u_white_point);
  color = vec4(aces_tonemap(tex_color), 1.0);*/

  vec3 diffuse = vec3(0.0f);

  for (int i=0; i<u_pl_count; i++)
    diffuse += calc_point_light(u_point_lights[i]);

  color = vec4(diffuse, 1.0);
}
