#pragma once

#include "MeshBuilder.h"

class MeshBuilderNew8 : public MeshBuilder
{
public:
	using VERT_TYPE = uint32_t;

	static const uint32_t CHUNK_SIZE = 8;
	const std::string SHADER = "pQuad";

	int VisibleBlocks = 0;

	MeshBuilderNew8() : MeshBuilder(CHUNK_SIZE, &SHADER) {}

	void BuildMesh(const uint8_t chunk[], std::vector<VERT_TYPE>&);

private:
	struct MeshSize;
	struct LocalPoint;

	uint8_t* FindVisibleFaces(const uint8_t chunk[]);
	void GreedyMesh(uint8_t*, Face, MeshSize*);
	void GenerateGSInput(MeshSize*, Face, std::vector<VERT_TYPE>&);
};

