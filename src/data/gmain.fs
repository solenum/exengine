#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_position;
uniform sampler2D u_norm;
uniform sampler2D u_colorspec;

uniform vec3 u_view_position;
uniform bool u_ambient_pass;
uniform bool u_point_active;

/* point light */
const int MAX_PL = 64;
struct point_light {
  vec3 position;
  vec3 color;
  bool is_shadow;
  float far;
};
// for dynamic lights
uniform point_light u_point_light;
uniform samplerCube u_point_depth;
// for static ones done in a single render pass
uniform point_light u_point_lights[MAX_PL];
uniform int         u_point_count;
/* ------------ */

vec3 pcf_offset[20] = vec3[]
(
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

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
  vec3 specular = l.color * specs * spec;

  // attenuation
  float attenuation = 1.0f / (1.0f + 0.1f * (distance * distance));
  diffuse  *= attenuation;
  specular *= (attenuation*1.5);

  // shadows
  float costheta = clamp(dot(normals, light_dir), 0.0, 1.0);
  float bias     = 0.2*tan(acos(costheta));
  bias           = clamp(bias, 0.1, 0.2);
  float shadow = 0.0f;
  if (l.is_shadow) {
    vec3 frag_to_light  = fragpos - l.position;
    float current_depth = length(frag_to_light);
    float view_dist     = length(u_view_position - fragpos);

    // PCF smoothing
    float radius = (1.0 + (view_dist / l.far)) / l.far;
    float offset = 0.1;
    int   samples = 20;
    float closest_depth = 0.0f;
    for (int i=0; i<samples; ++i) {
      closest_depth  = texture(u_point_depth, frag_to_light + pcf_offset[i] * radius).r;
      closest_depth *= l.far;
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
  vec3 diffuse = vec3(0.0f);

  if (u_ambient_pass == true) {
    diffuse += texture(u_colorspec, uv).rgb*0.1f;
  } else {
    // shadow casters
    if (u_point_active && u_point_count <= 0)
      diffuse += calc_point_light(u_point_light);
  }
    
  // non shadow casters
  if (u_point_count > 0)
    for (int i=0; i<u_point_count; i++)
      diffuse += calc_point_light(u_point_lights[i]);

  vec3 tex_color = vec3(1.0) - exp(-diffuse / u_white_point);
  color = vec4(aces_tonemap(tex_color), 1.0);
}
