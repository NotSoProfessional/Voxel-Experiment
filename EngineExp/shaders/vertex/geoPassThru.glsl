#version 460 core

layout (binding = 0) buffer OutputBuffer {
    float data[4];
};

layout (location = 0) in mediump uint aVertData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 finalPos;
vec2 finalTexCoord;

out vec3 gsPosition;
out vec2 gsMeshSize;

void main()
{
    finalTexCoord.s = (aVertData & 7);
    finalTexCoord.t = ((aVertData >> 3) & 7);
    
    finalPos.z = float((aVertData >> 6) & 7);
    finalPos.y = float((aVertData >> 9) & 7);
    finalPos.x = float((aVertData >> 12) & 7);

    gsPosition = vec3(finalPos.x,finalPos.y,finalPos.z);
    gsMeshSize = finalTexCoord;
    gl_Position = model*vec4(finalPos, 1.0);
}