#version 330 core

uniform vec3 someColor;

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    //someColor = vec3(1.0, 1.0, 1.0);

    FragColor = texture(ourTexture, TexCoord) * vec4(someColor, 1.0);
}