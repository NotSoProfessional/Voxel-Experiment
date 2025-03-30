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

	float scaler = 1;

	float offset = (1 * scaler);

	switch (face) {
	case 1:
		pos.z = pos.z + offset;

		break;

	case 2:
		pos.y = pos.y;
		pos.z = pos.z + offset;

		break;

	case 3:
		pos.z = pos.z + offset;
		pos.y = pos.y - offset;
		break;

	case 4:
		//pos.x = pos.x-1;
		pos.z = pos.z + offset;

		break;

	case 5:
		pos.x = pos.x - offset;
		pos.z = pos.z + offset;

		break;
	}

	pos = pos + ((chunkLocation) * 32);

	//pos = pos * vec3(0.25, 0.25, 0.25);

	gl_Position = vec4(pos, 1.0); //* vec4(0.5, 0.5, 0.5, 1.0);
}