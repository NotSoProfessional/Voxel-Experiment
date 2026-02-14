#version 450 core

uniform vec3 chunkLocation;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

layout(std430,binding=0)buffer PointData{
	int packedData[];// Local data within this chunk
};

out vec2 texCoord;
out int face;
out int block;

void main(void){
	//int quadId=gl_VertexID/6;
	int cornerId=gl_VertexID%4;
	
	//int aVertData=packedData[quadId];
	int aVertData=packedData[gl_InstanceID];
	
	vec3 pos;
	vec2 uv;
	
	pos.z=float((aVertData>>10)&0x1F);
	pos.y=float((aVertData>>15)&0x1F);
	pos.x=float((aVertData>>20)&0x1F);
	uv.x=float((aVertData>>5)&0x1F);
	uv.y=float((aVertData)&0x1F);
	
	uv=uv+vec2(1,1);
	
	face=int((aVertData>>25)&0x7);
	
	block=int((aVertData>>28)&0x3);
	
	texCoord=uv;
	
	switch(face){
		case 0:
		switch(cornerId){
			case 0:
			texCoord=vec2(1,1)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(-uv.x,0,0);
			texCoord=vec2(0,1)*uv;
			break;
			
			case 1:
			case 5:
			pos=pos+vec3(0,-uv.y,0);
			texCoord=vec2(1,0)*uv;
			break;
			
			case 3:
			pos=pos+vec3(-uv.x,-uv.y,0);
			texCoord=vec2(0,0)*uv;
			break;
		}
		
		break;
		
		case 1:
		pos.z=pos.z+1;
		
		switch(cornerId){
			case 0:
			texCoord=vec2(1,0)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(0,-uv.y,0);
			texCoord=vec2(1,1)*uv;
			break;
			
			case 1:
			case 5:
			pos=pos+vec3(-uv.x,0,0);
			texCoord=vec2(0,0)*uv;
			break;
			
			case 3:
			pos=pos+vec3(-uv.x,-uv.y,0);
			texCoord=vec2(0,1)*uv;
			break;
		}
		
		break;
		
		case 2:
		pos.y=pos.y;
		pos.z=pos.z+1;
		
		switch(cornerId){
			case 0:
			texCoord=vec2(0,0)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(-uv.x,0,0);
			texCoord=vec2(1,0)*uv;
			break;
			
			case 1:
			case 5:
			
			pos=pos+vec3(0,0,-uv.y);
			texCoord=vec2(0,1)*uv;
			break;
			
			case 3:
			pos=pos+vec3(-uv.x,0,-uv.y);
			texCoord=vec2(1,1)*uv;
			break;
		}
		
		break;
		
		case 3:
		pos.z=pos.z+1;
		pos.y=pos.y-1;
		
		switch(cornerId){
			case 0:
			texCoord=vec2(0,0)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(0,0,-uv.y);
			texCoord=vec2(0,1)*uv;
			break;
			
			case 1:
			case 5:
			pos=pos+vec3(-uv.x,0,0);
			texCoord=vec2(1,0)*uv;
			break;
			
			case 3:
			pos=pos+vec3(-uv.x,0,-uv.y);
			texCoord=vec2(1,1)*uv;
			break;
		}
		break;
		
		case 4:
		//pos.x = pos.x-1;
		pos.z=pos.z+1;
		
		switch(cornerId){
			case 0:
			texCoord=vec2(0,0)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(0,0,-uv.x);
			texCoord=vec2(1,0)*uv;
			break;
			
			case 1:
			case 5:
			
			pos=pos+vec3(0,0-uv.y,0);
			texCoord=vec2(0,1)*uv;
			break;
			
			case 3:
			pos=pos+vec3(0,-uv.y,-uv.x);
			texCoord=vec2(1,1)*uv;
			break;
		}
		
		break;
		
		case 5:
		pos.x=pos.x-1;
		pos.z=pos.z+1;
		
		switch(cornerId){
			case 0:
			texCoord=vec2(1,0)*uv;
			break;
			
			case 2:
			case 4:
			pos=pos+vec3(0,0-uv.y,0);
			texCoord=vec2(1,1)*uv;
			break;
			
			case 1:
			case 5:
			pos=pos+vec3(0,0,-uv.x);
			texCoord=vec2(0,0)*uv;
			break;
			
			case 3:
			pos=pos+vec3(0,-uv.y,-uv.x);
			texCoord=vec2(0,1)*uv;
			break;
		}
		
		break;
	}
	
	pos=pos+((chunkLocation)*32);
	gl_Position=projectionMatrix*viewMatrix*modelMatrix*vec4(pos,1.);
}