#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 texCoord;

void main(void){
	vec4 pointPos = gl_in[0].gl_Position;
	vec4 vertexPos = vec4(0, 0, 0, 0) + pointPos;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
	texCoord = vec2(0, 1);
	EmitVertex();

	vertexPos = vec4(1, 0, 0, 0) + pointPos;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
	texCoord = vec2(1, 1);
	EmitVertex();
	
	vertexPos = vec4(0, 1, 0, 0) + pointPos;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
	texCoord = vec2(0, 0);
	EmitVertex();
	
	vertexPos = vec4(1, 1, 0, 0) + pointPos;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
	texCoord = vec2(1, 0);
	EmitVertex();

	EndPrimitive();
}