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

uniform mat4 u_bone_matrix[200];
uniform bool u_has_skeleton;

void main()
{
	mat4 transform = in_instancematrix;
	if (u_has_skeleton == true) {
		mat4 skeleton = u_bone_matrix[int(in_boneindex.x*255)] * in_boneweights.x +
										u_bone_matrix[int(in_boneindex.y*255)] * in_boneweights.y +
										u_bone_matrix[int(in_boneindex.z*255)] * in_boneweights.z +
										u_bone_matrix[int(in_boneindex.w*255)] * in_boneweights.w;

		transform = in_instancematrix * skeleton;
	}

	gl_Position = transform * vec4(in_position, 1.0);
}
#END VS


#START FS
#version 330 core

in vec4 frag;

uniform vec3 u_light_pos;
uniform float u_far_plane;
uniform bool u_is_lit;

void main()
{
  if (!u_is_lit) {
    discard;
  }

  float light_distance = length(frag.xyz - u_light_pos);

  light_distance = light_distance / u_far_plane;

  gl_FragDepth = light_distance;
}
#END FS


#START GS
#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_shadow_matrices[6];

out vec4 frag;

void main()
{
  for (int face=0; face<6; ++face) {
    gl_Layer = face;
    for (int i=0; i<3; ++i) {
      frag = gl_in[i].gl_Position;
      gl_Position = u_shadow_matrices[face] * frag;
      EmitVertex();
    }
    EndPrimitive();
  }
}
#END GS