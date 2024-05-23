#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 gsPosition[];
in vec2 gsMeshSize[]; // Receive additional data from vertex shader

out vec2 TexCoord;

void main() {
    // Use gsAdditionalData to help calculate texture coordinates
    // Modify this part based on your requirements

    float s = gsMeshSize[0].s;
    float t = gsMeshSize[0].t;
    float x = gsPosition[0].x;
    float y = gsPosition[0].y;
    float z = gsPosition[0].z;

    TexCoord = gsAdditionalData[0] + vec2(0.0, 0.0); gl_Position = vec4(x,y,z, 1.0); EmitVertex();
    TexCoord = gsAdditionalData[0] + vec2(1.0*s, 0.0); gl_Position = vec4(x-s,y,z, 1.0); EmitVertex();
    TexCoord = gsAdditionalData[0] + vec2(0.0, 1.0*t); gl_Position = vec4(x-s, y-t, z, 1.0); EmitVertex();
    TexCoord = gsAdditionalData[0] + vec2(1.0*s, 1.0*t); gl_Position = vec4(x, y-t, z, 1.0); EmitVertex();
    EndPrimitive();
}