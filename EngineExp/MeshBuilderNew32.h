#pragma once

#include "MeshBuilder.h"

class MeshBuilderNew32 : public MeshBuilder
{
public:
	using VERT_TYPE = uint32_t;

	static const uint32_t CHUNK_SIZE = 32;
	const std::string SHADER = "chunk32";

	//int VisibleBlocks = 0;

	MeshBuilderNew32() : MeshBuilder(CHUNK_SIZE, &SHADER) {}

	void BuildMesh(const uint8_t chunk[], std::vector<VERT_TYPE>&);

private:
	struct MeshSize;
	struct LocalPoint;

	uint8_t* FindVisibleFaces(const uint8_t chunk[]);
	void GreedyMesh(uint8_t*, Face, MeshSize*);
	void GenerateGSInput(MeshSize*, Face, std::vector<VERT_TYPE>&);
};
