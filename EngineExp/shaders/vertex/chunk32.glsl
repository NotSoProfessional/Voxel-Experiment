#version 330

uniform vec3 chunkLocation;

layout(location = 0) in mediump int aVertData;


in vec3 position;

out VS_OUT{
	vec2 uv;
	int face;
} vs_out;

void main(void) {
	vec3 pos;

	pos.z = float((aVertData >> 10) & 0x1F);
	pos.y = float((aVertData >> 15) & 0x1F);
	pos.x = float((aVertData >> 20) & 0x1F);
	vs_out.uv.x = float((aVertData >> 5) & 0x1F);
	vs_out.uv.y = float((aVertData) & 0x1F);

	int face = int((aVertData >> 25) & 0xF);
	vs_out.face = face;

	switch (face) {
	case 1:
		pos.z = pos.z + 1;

		break;

	case 2:
		pos.y = pos.y;
		pos.z = pos.z + 1;

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
		pos.x = pos.x - 1;
		pos.z = pos.z + 1;

		break;
	}

	pos = pos + ((chunkLocation) * 32);

	gl_Position = vec4(pos, 1.0);
}