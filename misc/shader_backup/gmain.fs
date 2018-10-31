#START VS
#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 uv;

void main() 
{
  gl_Position = vec4(in_position.x, in_position.y, 0.0f, 1.0f);
  uv = in_uv;
}
#END VS


#START FS
#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D u_position;
uniform sampler2D u_norm;
uniform sampler2D u_colorspec;
uniform sampler2D u_ssao;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_inverse_view;

uniform bool u_ambient_pass;

/* spot lights */
const int MAX_SL = 32;
struct spot_light {
  vec3 position;
  vec3 direction;
  vec3 color;
  float inner;
  float outer;
  bool is_shadow;
  float far;
};
// for dynamic spotlights
uniform spot_light u_spot_light;
uniform sampler2D  u_spot_depth;
// for static spotlights
uniform spot_light u_spot_lights[MAX_SL];
uniform int        u_spot_count;
uniform bool       u_spot_active;
/* ------------ */

/* dir light */
struct dir_light {
  vec3 position;
  vec3 target;
  vec3 color;
  float far;
};
uniform dir_light u_dir_light;
uniform sampler2D u_dir_depth;
uniform mat4      u_dir_transform;
uniform bool      u_dir_active;
/* ------------ */

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
uniform bool        u_point_active;
/* ------------ */

vec3 pcf_offset[20] = vec3[]
(
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec3 calc_point_light(point_light light)
{
  point_light l = light;
  l.position = vec3(u_view * vec4(l.position, 1.0));

  // point light
  vec3 fragpos = texture(u_position, uv).rgb;
  vec3 normals = texture(u_norm, uv).rgb * 2.0 - 1.0;
  vec3 diff    = texture(u_colorspec, uv).rgb;
  float spec   = texture(u_colorspec, uv).a*1.0f;

  vec3 view_dir  = normalize(-fragpos);
  vec3 light_dir = l.position - fragpos;
  float dist = length(light_dir);
  light_dir = normalize(light_dir);
  
  // diffuse
  vec3 diffuse   = max(dot(light_dir, normals), 0.0) * diff * l.color;

  // specular
  vec3 halfwayd  = normalize(light_dir + view_dir);
  float specs    = pow(max(dot(normals, halfwayd), 0.0), 64.0f);
  vec3 specular = l.color * specs * spec;

  // attenuation
  // float attenuation = 1.0f / (1.0f + 0.1f * (dist * dist));
  float attenuation = 1.0f / (1.0f + 0.14f * dist + 0.07f * (dist * dist));
  // float attenuation = 1.0f / (1.0f + dist);
  // float attenuation = max(0.0, 4.0 - pow(dist, 1.0/2.0));
  diffuse  *= attenuation;
  specular *= attenuation;

  // shadows
  float costheta = clamp(dot(normals, light_dir), 0.0, 1.0);
  float bias     = 0.2*tan(acos(costheta));
  bias           = clamp(bias, 0.1, 0.2);
  float shadow = 0.0f;
  if (l.is_shadow) {
    vec3 frag_to_light  = mat3(u_inverse_view) * (fragpos - l.position);
    float current_depth = length(frag_to_light);
    float view_dist     = length(-fragpos);

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

void main()
{
  vec3 diffuse = vec3(0.0f);
  vec3 reflection = vec3(0.0f);
  float ao = texture(u_ssao, uv).r;

  if (u_ambient_pass) {
    diffuse += texture(u_colorspec, uv).rgb * 0.06;

    // vec3 normals = normalize(mat3(u_inverse_view) * normalize(texture(u_norm, uv).rgb));
    // vec3 fragpos = mat3(u_inverse_view) * texture(u_position, uv).rgb;
    // float spec   = texture(u_colorspec, uv).a;
    // vec3 eye = normalize(u_eye_dir);
    // eye = normalize(fragpos);
    // eye.y = -eye.y;
    // vec3 reflected = normalize(reflect(eye, normals));
    // reflection = texture(u_reflection, reflected).rgb * 5.5;
    // diffuse *= reflection * spec;
  } else {
    // shadow casters
    if (u_point_active && u_point_count <= 0)
      diffuse += calc_point_light(u_point_light);

    // if (u_spot_active && u_spot_count <= 0)
      // diffuse += calc_spot_light(u_spot_light);
  }
    
  // non shadow casters
  if (u_point_count > 0)
    for (int i=0; i<u_point_count; i++)
      diffuse += calc_point_light(u_point_lights[i]);

  vec3 tex_color = vec3(1.0) - exp(-diffuse * 1.0);
  color = vec4(aces_tonemap(tex_color), 2.5);
  color *= ao;
  color *= min(100.0 / length(texture(u_position, uv).rgb), 1.0);
  // color = vec4(diffuse * ao, 1.0f);
}
#END FS