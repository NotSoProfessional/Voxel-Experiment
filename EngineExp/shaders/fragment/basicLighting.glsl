#version 330

out vec4 FragColor;

in vec2 texCoord;
flat in int face;

uniform sampler2D ourTexture;


void main(void) {
	vec4 brightness = vec4(1,1,1,1);

	switch (face) {
	case 0:
		brightness = vec4(1.1, 1.1, 1.1, 1);
		break;

	case 1:
		brightness = vec4(0.5, 0.5, 0.5, 1);
		break;

	case 2:
		brightness = vec4(1.2, 1.2, 1.2, 1);
		break;

	case 3:
		brightness = vec4(0.35, 0.35, 0.35, 1);
		break;

	case 4:
		brightness = vec4(0.75, 0.75, 0.75, 1);
		break;

	case 5:
		brightness = vec4(0.75, 0.75, 0.75, 1);
		break;
	}


	FragColor = texture(ourTexture, texCoord) * brightness;

}