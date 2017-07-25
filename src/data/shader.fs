#version 330 core

in vec3 frag;
in vec3 normal;
in vec2 uv;
in vec4 color;
in vec4 frag_light_pos;
in float fog;
in mat3 TBN;

out vec4 out_color;

uniform sampler2D u_position;;
uniform sampler2D u_norm;
uniform sampler2D u_colorspec;

uniform bool u_is_textured; 
uniform bool u_is_spec;
uniform bool u_is_norm;
uniform bool u_dont_norm;

uniform bool u_is_billboard;
uniform bool u_is_lit;
uniform vec3 u_view_position;
uniform float u_far_plane;
uniform bool u_ambient_pass;

/* point light */
const int NR_PL = 32;
struct point_light {
  vec3 position;
  vec3 color;
  bool is_shadow;
};
uniform point_light u_point_lights[NR_PL];
uniform samplerCube u_point_depth; 
uniform bool u_point_active;
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
uniform bool u_dir_active;
/* ------------ */

vec3 pcf_offset[20] = vec3[]
(
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec3 calc_point_light(point_light l, samplerCube depth)
{
  // point light
  vec3 fragpos = texture(u_position, uv).rgb;
  vec3 normals = texture(u_norm, uv).rgb;
  vec3 diffuse = texture(u_colorspec, uv).rgb;
  float spec   = texture(u_colorspec, uv).a;

  vec3 view_dir  = normalize(u_view_position - fragpos);
  float distance = length(l.position - fragpos);
  vec3 light_dir = normalize(l.position - fragpos);
  
  // diffuse
  vec3 diff      = max(dot(normals, light_dir), 0.0) * diffuse * l.color;

  // specular
  vec3 halfwayd  = normalize(light_dir + view_dir);
  float specs    = pow(max(dot(normals, halfwayd), 0.0), 16.0);
  vec3 specular = l.color * specs * spec;

  // attenuation
  float attenuation = 1.0f / (1.0f + 0.064f * (distance * distance));
  diffuse *= attenuation;
  spec    *= attenuation;
  return vec3(diffuse + spec);

  /*vec3 norm = normalize(normal);

  if (u_is_norm && !u_dont_norm && u_is_textured) {
    norm = texture(u_norm, uv).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);
  }

  vec3 light_dir  = normalize(l.position - frag);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  float distance    = length(l.position - frag);
  float attenuation = 1.0f / (1.0f + 0.064f * (distance * distance));
  diffuse *= attenuation;

  // shadows
  float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
  float bias     = 0.2*tan(acos(costheta));
  bias           = clamp(bias, 0.01, 0.2);

  /*float shadow = 0.0f;
  if (l.is_shadow) {
    vec3 frag_to_light  = frag - l.position;
    float current_depth = length(frag_to_light);
    float view_dist     = length(u_view_position - frag);

    // PCF smoothing
    float radius = (1.0 + (view_dist / u_far_plane)) / 50.0;
    float offset = 0.1;
    int   samples = 20;
    for (int i=0; i<samples; ++i) {
      float closest_depth = texture(depth, frag_to_light + pcf_offset[i] * radius).r;
      closest_depth      *= u_far_plane;
      if (current_depth - bias > closest_depth)
        shadow += 1.0;
    }
    shadow /= float(samples);
  }

  if (u_is_spec && u_is_textured) {
    float spec = 0.0;
    vec3 view_dir = normalize(u_view_position - frag);
    vec3 halfwayDir = normalize(light_dir + view_dir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = l.color * (8*spec) * vec3(texture(u_spec, uv));
    diffuse += (specular * attenuation);
  }

  return vec3((1.0 - shadow) * diffuse);*/

  return vec3(diffuse);
}

/*vec3 calc_dir_light(dir_light l, sampler2D depth)
{
  vec3 proj = frag_light_pos.xyz / frag_light_pos.w;
  proj = proj * 0.5 + 0.5;

  // point light
  vec3 norm       = normalize(normal);

  if (u_is_norm && !u_dont_norm) {
    norm = texture(u_norm, uv).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);
  }

  vec3 light_dir  = normalize(l.position - l.target);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  // shadows
  float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
  float bias     = 0.4*tan(acos(costheta));
  bias           = clamp(bias, 0.3, 0.4);
  
  vec3 frag_to_light  = l.target - l.position;
  float current_depth = proj.z * l.far;
  float shadow        = 0.0f;
  vec2 tsize = 1.0 / textureSize(depth, 0);
  for (int x=-1; x<=1; ++x) {
    for (int y=-1; y<=1; ++y) {
      float pcf_depth = texture(depth, proj.xy + vec2(x, y) * tsize).r;
      pcf_depth *= l.far;
      shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  if (u_is_spec) {
    float spec = 0.0; 
    vec3 view_dir = normalize(u_view_position - frag);
    vec3 halfwayDir = normalize(light_dir + view_dir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = l.color * (2*spec) * vec3(texture(u_spec, uv));
    diffuse += specular;
  }

  if (proj.z > 1.0)
    shadow = 1.0f;

  return vec3((1.0 - shadow) * diffuse);
}*/

void main()
{
  vec3 diffuse = vec3(0.0f);
  
  for (int i=0; i<NR_PL; i++)
    diffuse += calc_point_light(u_point_lights[i], u_point_depth);

  out_color = vec4(diffuse, 1.0);

  /*if (u_is_lit) {
    vec3 diffuse = vec3(0.0f);

    // ambient lighting
    if (u_ambient_pass) {
      diffuse         = vec3(0.1f);
      vec3 norm       = normalize(normal);
      vec3 light_dir  = normalize(vec3(0, 100, 0) - frag);
      float diff      = max(dot(light_dir, norm), 0.0);
      diffuse        += vec3(diff * 0.05f);
    }

    vec3 p = vec3(0.0f);
    vec3 d = vec3(0.0f);
    
    // if (u_dir_active)
      // d = calc_dir_light(u_dir_light, u_dir_depth);
    for (int i=0; i<NR_PL; i++)
      p += calc_point_light(u_point_lights[i], u_point_depth);

    diffuse += p + d;

    if (u_is_textured) {
      out_color = vec4(diffuse * vec3(texture(u_texture, uv, -1.0)), 1.0);
    } else {
      out_color = vec4(diffuse * color.rgb, 1.0);
    }
  } else {
    if (u_is_textured) {
      out_color = texture(u_texture, uv, -1.0);
    } else {
      out_color = vec4(color.rgb, 1.0);
    }
  }*/
}