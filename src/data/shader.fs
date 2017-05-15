#version 330 core

in vec3 frag;
flat in vec3 normal;
in vec2 uv;
in vec4 color;
in vec4 frag_light_pos;
in float fog;

out vec4 out_color;

uniform sampler2D u_texture;
uniform bool u_is_billboard;
uniform bool u_is_textured;
uniform bool u_is_lit;
uniform vec3 u_view_position;
uniform float u_far_plane;

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
  vec3 norm       = normalize(normal);
  vec3 light_dir  = normalize(l.position - frag);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  float distance    = length(l.position - frag);
  distance          = clamp(distance, 0.0f, u_far_plane);
  float b           = 1.0f / (u_far_plane * u_far_plane * 0.05f);
  float attenuation = 1.0f / (1.0f * (distance * distance));
  diffuse *= attenuation;

  // shadows
  vec3 frag_to_light  = frag - l.position;
  float closest_depth = texture(depth, frag_to_light).r;
  closest_depth      *= u_far_plane;
  float current_depth = length(frag_to_light);
  float costheta = clamp(dot(norm, normalize(l.position)), 0.0, 1.0);
  float bias          = 0.8*tan(acos(costheta));
  bias                = clamp(bias, 0.0, 0.8);
  float shadow        = current_depth - bias > closest_depth ? 1.0 : 0.0;
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
  vec3 frag_to_light  = frag - l.position;
  float closest_depth = texture(depth, proj.xy).r;
  float current_depth = proj.z;
  closest_depth *= l.far;
  current_depth *= l.far;
  float costheta = clamp(dot(norm, normalize(l.position)), 0.0, 1.0);
  float bias          = 1.0*tan(acos(costheta));
  bias                = clamp(bias, 0.0, 1.0);
  float shadow        = current_depth - bias > closest_depth ? 1.0 : 0.0;

  return vec3((1.0 - shadow) * diffuse);
}

void main()
{
  if (u_is_lit) {
    vec3 diffuse = vec3(0.0f);

    vec3 p = vec3(0.0f);
    vec3 d = vec3(0.0f);
    
    if (u_dir_active)
      d = calc_dir_light(u_dir_light, u_dir_depth);
    if (u_point_active)
      p = calc_point_light(u_point_light, u_point_depth);

    diffuse = p + d;

    if (u_is_textured) {
      out_color = vec4(diffuse * vec3(texture(u_texture, uv)), 1.0f) * fog;
    } else {
      out_color = vec4(diffuse * color.rgb, 1.0f);
    }
  } else {
    if (u_is_textured) {
      out_color = texture(u_texture, uv) * fog;
    } else {
      out_color = vec4(color.rgb, 1.0f);
    }
  }
  
  // out_color = vec4(1.0f);
  //out_color = vec4(vec3(closest_depth / u_far_plane), 1.0);
}