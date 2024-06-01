#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 texCoord;

in VS_OUT{
	vec2 uv;
int face;
} gs_in[];

void main(void){
	vec4 pointPos = gl_in[0].gl_Position + vec4(1,1,1,0);
	vec4 vertexPos;
	//if (gs_in[0].offset){
	//	pointPos = pointPos+vec4(0,0,1,0);
	//}

	mat4 rotationMat = mat4(1);

	vec2 uv = gs_in[0].uv + vec2(1, 1);

	switch (gs_in[0].face) {
	case 0:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;

	case 1:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;

	case 2:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y, pointPos.z - uv.y, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z - uv.y, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;

	case 3:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y, pointPos.z - uv.y, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x - uv.x, pointPos.y, pointPos.z - uv.y, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;

	case 4:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y, pointPos.z - uv.x, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z - uv.x, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;

	case 5:
		vertexPos = vec4(0, 0, 0, 0) + pointPos;
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x , pointPos.y, pointPos.z - uv.x, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 0) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(0, 1) * uv;
		EmitVertex();

		vertexPos = vec4(pointPos.x, pointPos.y - uv.y, pointPos.z - uv.x, 1);
		gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPos;
		texCoord = vec2(1, 1) * uv;
		EmitVertex();

		EndPrimitive();

		break;
	}


}