#version 330

out vec4 FragColor;

in vec3 colour;
in vec2 texCoord;

uniform sampler2D ourTexture;


void main(void) {

	FragColor = texture(ourTexture, texCoord);// *vec4(1, 1, 0, 0.5);

}