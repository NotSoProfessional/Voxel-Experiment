#version 460 core
layout (location = 0) in mediump uint aVertData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

vec3 finalPos;
vec2 finalTexCoord;

float face;

void main()
{
    finalTexCoord.s = (aVertData & 1);
    finalTexCoord.t = ((aVertData >> 1) & 1);
    
    finalPos.z = float((aVertData >> 2) & 15);
    finalPos.y = float((aVertData >> 6) & 15);
    finalPos.x = float((aVertData >> 10) & 15);

    face = (aVertData >> 14) & 3;

    //if (face == 1){
    //    finalTexCoord.t /= finalPos.x;
    //    finalTexCoord.s /= finalPos.y;
    //}

    float scaleX = finalPos.x;
    float scaleY = finalPos.y;

    finalTexCoord.s *= scaleX;
    finalTexCoord.t *= scaleY;

    gl_Position = projection * view * model * vec4(finalPos, 1);
    //gl_Position = vec4(finalPos, 1);
    TexCoord = finalTexCoord;
}