#version 330

layout (location = 0) in mediump int aVertData;

in vec3 position;

out VS_OUT {
	vec2 uv;
	bool offset;
} vs_out;

void main(void) {
	vec3 pos;

	pos.z = float((aVertData >> 6) & 7);
	pos.y = float((aVertData >> 9) & 7);
	pos.x = float((aVertData >> 12) & 7);
	vs_out.uv.x = float((aVertData >> 3) & 7);
	vs_out.uv.y = float((aVertData) & 7);

	if (float((aVertData >> 15) & 1) == 1){
		pos.z = pos.z+1;
	}else{
		vs_out.offset = false;
	}

	gl_Position = vec4(pos, 1.0);
}