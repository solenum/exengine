#version 330 core

in vec3 frag;
in vec3 normal;
in vec2 uv;
in vec4 color;

out vec4 out_color;

uniform sampler2D uni_texture;
uniform bool uni_is_billboard;
uniform vec3 uni_view_position;

void main()
{
  out_color = texture(uni_texture, uv);

  vec3 ambient          = vec3(0.3f, 0.3f, 0.3f); // * vec3(texture(uni_texture, uv));
  vec3 light_diffuse    = vec3(0.7f, 0.7f, 0.7f);
  vec3 light_direction  = vec3(-0.2f, -1.0f, -0.3f);

  vec3 norm       = normalize(normal);
  vec3 light_dir  = normalize(-light_direction);
  float diff      = max(dot(norm, light_dir), 0.0);
  vec3 diffuse    = light_diffuse * diff; // * vec3(texture(uni_texture, uv));

  out_color = vec4(ambient + diffuse, 1.0f) * (color / 255);
}