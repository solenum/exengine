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

uniform samplerCube u_point_depths0, u_point_depths1, u_point_depths2, u_point_depths3, u_point_depths4;
uniform int         u_point_count;
uniform float       u_point_far;
/* ------------ */

vec3 pcf_offset[20] = vec3[]
(
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec3 calc_point_light(point_light l, int ind)
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
  float specs    = pow(max(dot(normals, halfwayd), 0.0), 32.0);
  vec3 specular = l.color * specs * spec;

  // attenuation
  float attenuation = 1.0f / (1.0f + 0.064f * (distance * distance));
  diffuse  *= attenuation;
  specular *= attenuation;

  // shadows
  float costheta = clamp(dot(normals, light_dir), 0.0, 1.0);
  float bias     = 0.2*tan(acos(costheta));
  bias           = clamp(bias, 0.01, 0.2);
  float shadow = 0.0f;
  if (l.is_shadow && 0 == 1) {
    vec3 frag_to_light  = fragpos - l.position;
    float current_depth = length(frag_to_light);
    float view_dist     = length(u_view_position - fragpos);

    // PCF smoothing
    float radius = (1.0 + (view_dist / u_point_far)) / 50.0;
    float offset = 0.1;
    int   samples = 20;
    float closest_depth = 0.0f;
    for (int i=0; i<samples; ++i) {
      if (ind == 0)
        closest_depth = texture(u_point_depths0, frag_to_light + pcf_offset[i] * radius).r;
      else if (ind == 1)
        closest_depth = texture(u_point_depths1, frag_to_light + pcf_offset[i] * radius).r;
      else if (ind == 2)
        closest_depth = texture(u_point_depths2, frag_to_light + pcf_offset[i] * radius).r;
      else if (ind == 3)
        closest_depth = texture(u_point_depths3, frag_to_light + pcf_offset[i] * radius).r;
      else if (ind == 4)
        closest_depth = texture(u_point_depths4, frag_to_light + pcf_offset[i] * radius).r;
      
      closest_depth      *= u_point_far;
      if (current_depth - bias > closest_depth)
        shadow += 1.0;
    }
    shadow /= float(samples);
  }

  return vec3((1.0 - shadow) * (diffuse + specular));
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
  vec3 diffuse = texture(u_colorspec, uv).rgb*0.1f;

  for (int i=0; i<u_point_count; i++)
    diffuse += calc_point_light(u_point_lights[i], i);
  
  vec3 tex_color = vec3(1.0) - exp(-diffuse / u_white_point);
  color = vec4(aces_tonemap(tex_color), 1.0);
}
