#version 330

layout (location = 0) in mediump int aVertData;

in vec3 position;

out VS_OUT {
	vec2 uv;
	int face;
} vs_out;

void main(void) {
	vec3 pos;

	pos.z = float((aVertData >> 6) & 7);
	pos.y = float((aVertData >> 9) & 7);
	pos.x = float((aVertData >> 12) & 7);
	vs_out.uv.x = float((aVertData >> 3) & 7);
	vs_out.uv.y = float((aVertData) & 7);

	int face = int((aVertData >> 15) & 7);
	vs_out.face = face;

	switch (face) {
	case 1:
		pos.z = pos.z + 1;

		break;

	case 2:
		pos.y = pos.y;
		pos.z = pos.z +1;

		break;

	case 3:
		pos.z = pos.z + 1;
		pos.y = pos.y - 1;
		break;

	case 4:
		//pos.x = pos.x-1;
		pos.z = pos.z + 1;

		break;

	case 5:
		pos.x = pos.x-1;
		pos.z = pos.z + 1;

		break;
	}

	gl_Position = vec4(pos, 1.0);
}