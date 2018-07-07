#version 330 core

in vec2 uv;

out float frag_color;

uniform sampler2D u_gposition;
uniform sampler2D u_gnormal;
uniform sampler2D u_noise;

uniform vec3 u_samples[32];
uniform mat4 u_projection;
uniform mat4 u_view;
uniform vec2 u_screensize;

int kernel_size = 32;
float radius = 0.8;
float bias = 0.2;

void main()
{
  vec2 noise_scale = vec2(u_screensize.x/4.0, u_screensize.y/4.0);

  // ssao input
  vec3 fragpos = texture(u_gposition, uv).xyz;
  vec3 normal = normalize(texture(u_gnormal, uv).rgb);
  vec3 randomvec = normalize(texture(u_noise, uv * noise_scale).xyz);

  // generate TBN
  vec3 tangent = normalize(randomvec - normal * dot(randomvec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);

  // calculate occlusion factor
  float occlusion = 0.0;
  for (int i=0; i<kernel_size; i++) {
    // convert from tangent to view space
    vec3 sample = TBN * u_samples[i];
    // get sample pos
    sample = fragpos + sample * radius;

    vec4 offset = vec4(sample, 1.0);
    offset = u_projection * offset;
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;

    float sampledepth = texture(u_gposition, offset.xy).z;

    float rangecheck = smoothstep(0.0, 1.0, radius / abs(fragpos.z - sampledepth));
    occlusion += (sampledepth >= sample.z + bias ? 1.0 : 0.0) * rangecheck;
  }
  occlusion = 1.0 - (occlusion / kernel_size);
  frag_color = pow(occlusion, 1.0);
}