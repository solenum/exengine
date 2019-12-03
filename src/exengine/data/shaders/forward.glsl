#START VS
#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normals;
layout (location = 3) in vec4 in_tangents;
layout (location = 4) in vec4 in_color;
layout (location = 5) in vec4 in_boneindex;
layout (location = 6) in vec4 in_boneweights;
layout (location = 7) in mat4 in_instancematrix;

out vec3 frag;
out vec3 normal;
out vec2 uv;
out mat3 TBN;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;

void main()
{
  mat4 transform = in_instancematrix;
  if (u_has_skeleton == true) {
    vec4 boneindex = in_boneindex*255.0;
    vec4 boneweights = in_boneweights*255.0;
    mat4 skeleton = u_bone_matrix[int(boneindex.x)] * boneweights.x +
                    u_bone_matrix[int(boneindex.y)] * boneweights.y +
                    u_bone_matrix[int(boneindex.z)] * boneweights.z +
                    u_bone_matrix[int(boneindex.w)] * boneweights.w;

    transform = transform * skeleton;
  }

  vec4 v = u_projection * u_view * transform * vec4(in_position, 1.0);

  gl_Position    = v;
  normal         = mat3(transpose(inverse(u_view * transform))) * in_normals;
  frag           = vec3(u_view * in_instancematrix * vec4(in_position, 1.0f));
  uv             = in_uv;

  // calculate tbn matrix for normal mapping
  vec3 T = normalize(vec3(u_view * transform * vec4(in_tangents.xyz, 0.0)));
  vec3 B = normalize(vec3(u_view * transform * in_tangents));
  vec3 N = normalize(vec3(u_view * transform * vec4(in_normals, 0.0)));
  TBN    = mat3(T, B, N);
}
#END VS


#START FS
#version 330 core

out vec4 color;

in vec3 frag;
in vec3 normal;
in vec2 uv;
in mat3 TBN;

uniform sampler2D u_texture;
uniform sampler2D u_spec;
uniform sampler2D u_norm;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_inverse_view;

uniform bool u_ambient_pass;

uniform samplerCube u_reflection;

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

  vec3 fragpos = frag;
  vec3 normals = texture(u_norm, uv).rgb;
  normals = normalize(normals * 2.0 - 1.0);
  normals = normalize(TBN * normals);
  
  vec3 diff    = texture(u_texture, uv).rgb;
  vec3 spec   = texture(u_spec, uv).rgb;

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
  float attenuation = 1.0f / (1.0f + 0.14f * dist + 0.07f * (dist * dist));
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

void main()
{
  vec3 diffuse = vec3(0.0f);

  if (u_ambient_pass) {
    diffuse += texture(u_texture, uv).rgb * 0.025;
  } else {
    // shadow casters
    if (u_point_active && u_point_count <= 0)
      diffuse += calc_point_light(u_point_light);
  }
    
  // non shadow casters
  if (u_point_count > 0)
    for (int i=0; i<u_point_count; i++)
      diffuse += calc_point_light(u_point_lights[i]);

  color = vec4(diffuse, 1.0);
}
#END FS