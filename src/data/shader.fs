#version 330 core

in vec3 frag;
in vec3 normal;
in vec2 uv;
in vec4 color;
in vec4 frag_light_pos;
in float fog;
in mat3 TBN;

out vec4 out_color;

uniform sampler2D u_texture;
uniform sampler2D u_spec;
uniform sampler2D u_norm;

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
struct point_light {
  vec3 position;
  vec3 color;
};
uniform point_light u_point_light;
uniform samplerCube u_point_depth; 
uniform bool u_point_active;
/* ------------ */

/* dir light */
struct dir_light {
  vec3 position;
  vec3 color;
  float far;
};
uniform dir_light u_dir_light;
uniform sampler2D u_dir_depth;
uniform bool u_dir_active;
/* ------------ */

vec3 calc_point_light(point_light l, samplerCube depth)
{
  // point light
  vec3 norm = normalize(normal);

  if (u_is_norm && !u_dont_norm) {
    norm = texture(u_norm, uv).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm); 
  }

  vec3 light_dir  = normalize(l.position - frag);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  float distance    = length(l.position - frag);
  float attenuation = 1.0f / (1.0f + 0.044f * (distance * distance));
  diffuse *= attenuation;

  // shadows
  float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
  float bias     = 0.6*tan(acos(costheta));
  bias           = clamp(bias, 0.01, 0.6);

  vec3 frag_to_light  = frag - l.position;
  float closest_depth = texture(depth, frag_to_light).r;
  closest_depth      *= u_far_plane;
  float current_depth = length(frag_to_light);
  float shadow        = current_depth - bias > closest_depth ? 1.0 : 0.0;

  if (u_is_spec) {
    float spec = 0.0;
    vec3 view_dir = normalize(u_view_position - frag);
    vec3 halfwayDir = normalize(light_dir + view_dir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = l.color * spec * vec3(texture(u_spec, uv)*2);
    diffuse += (specular * attenuation);
  }

  return vec3((1.0 - shadow) * diffuse);
}

vec3 calc_dir_light(dir_light l, sampler2D depth)
{
  vec3 proj = frag_light_pos.xyz / frag_light_pos.w;
  proj = proj * 0.5 + 0.5;

  // point light
  vec3 norm       = normalize(normal);
  vec3 light_dir  = normalize(l.position - frag);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  // shadows
  float costheta = clamp(dot(norm, light_dir), 0.0, 1.0);
  float bias     = 0.4*tan(acos(costheta));
  bias        = clamp(bias, 0.05, 0.4);
  
  vec3 frag_to_light  = frag - l.position;
  float closest_depth = texture(depth, proj.xy).r;
  float current_depth = proj.z;
  closest_depth      *= l.far;
  current_depth      *= l.far;
  float shadow        = current_depth - bias > closest_depth ? 1.0 : 0.0;

  return vec3((1.0 - shadow) * diffuse);
}

void main()
{
  if (u_is_lit) {
    vec3 diffuse = vec3(0.0f);

    // ambient lighting
    if (u_ambient_pass) {
      diffuse         = vec3(0.05f);
      vec3 norm       = normalize(normal);
      vec3 light_dir  = normalize(vec3(0, 100, 0) - frag);
      float diff      = max(dot(light_dir, norm), 0.0);
      diffuse        += vec3(diff * 0.05);
    }

    vec3 p = vec3(0.0f);
    vec3 d = vec3(0.0f);
    
    if (u_dir_active)
      d = calc_dir_light(u_dir_light, u_dir_depth);
    if (u_point_active)
      p = calc_point_light(u_point_light, u_point_depth);

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
  }
}