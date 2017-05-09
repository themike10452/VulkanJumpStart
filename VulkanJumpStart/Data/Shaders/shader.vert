#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

vec3 colors[3] = {
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),
	vec3( 0.0, 0.0, 1.0 )
};

void main()
{
	//gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
	gl_Position = vec4(inPosition, 1.0);
	fragColor = colors[gl_VertexIndex];
}