#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

vec3 colors[] = {
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),
	vec3( 0.0, 0.0, 1.0 )
};

void main()
{
	gl_Position = projection * view * model * vec4(inPosition, 1.0);
	fragColor = colors[gl_VertexIndex % 3];
}
