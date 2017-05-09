#version 330 core

in vec3 frag;
// in vec3 normal;
flat in vec3 normal;
in vec2 uv;
in vec4 color;
in float fog;

out vec4 out_color;

uniform sampler2D u_texture;
uniform bool u_is_billboard;
uniform bool u_is_textured;
uniform bool u_is_lit;
uniform bool u_ambient_pass;
uniform vec3 u_view_position;
uniform float u_far_plane;

/* point lights */
struct point_light {
  vec3 position;
  vec3 color;
  float intensity;
};
uniform point_light u_point_light;
uniform samplerCube u_point_depth;
/* ------------ */

vec3 calc_point_light(point_light l, samplerCube depth)
{
  // point light
  vec3 norm       = normalize(normal);
  vec3 light_dir  = normalize(l.position - frag);
  float diff      = max(dot(light_dir, norm), 0.0);
  vec3 diffuse    = diff * l.color;

  float distance    = length(l.position - frag);
  float b           = 1.0f / (l.intensity * l.intensity * 0.02f);
  float attenuation = 1.0f / (1.0f + 0.2f * distance + b * (distance * distance));
  diffuse *= attenuation;

  // shadows
  vec3 frag_to_light  = frag - l.position; 
  float closest_depth = texture(depth, frag_to_light).r;
  closest_depth      *= u_far_plane;
  float current_depth = length(frag_to_light);
  // float bias          = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
  // float shadow        = current_depth - bias> closest_depth ? 1.0 : 0.0;
  float bias = 0.005 * tan(acos(dot(normal, light_dir)));
  bias = clamp(bias, 0.9,1);
  float shadow        = current_depth > closest_depth ? 1.0 : 0.0;
  return vec3((1.0 - shadow) * diffuse);
}

void main()
{
  if (u_is_lit) {
    vec3 ambient = vec3(0.1f);
    vec3 diffuse = vec3(0.0f);

    // calc point lights
    if (!u_ambient_pass) {
      diffuse = calc_point_light(u_point_light, u_point_depth);
      ambient = vec3(0.0f);
    }

    if (u_is_textured) {
      out_color = vec4((ambient + diffuse) * vec3(texture(u_texture, uv)), 1.0f) * fog;
    } else {
      out_color = vec4((ambient + diffuse) * (color.rgb / 255), 1.0f) * fog;
    }
  } else {
    if (u_is_textured) {
      out_color = texture(u_texture, uv) * fog;
    } else {
      out_color = vec4((color.rgb / 255), 1.0f) * fog;
    }
  }

  //out_color = vec4(vec3(closest_depth / u_far_plane), 1.0);
}