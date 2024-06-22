#include "MeshBuilderNew32.h"

#include <iostream>


//
// Define Structs
//

struct MeshBuilderNew32::MeshSize {
	uint16_t uv = 0x41;

	uint16_t GetU() {
		return uv & 0x3F;
	}

	void SetU(uint16_t u) {
		uv = uv & 0xFC0;
		uv |= u;
	}

	uint16_t GetV() {
		return (uv & 0xFC0) >> 6;
	}

	void SetV(uint16_t v) {
		v = v << 6;
		uv = v |= (uv & 0x3F);
	}
};

struct MeshBuilderNew32::LocalPoint {
	uint32_t data = 0;

	void SetFace(uint32_t face) {
		face = face << 25;
		data = face |= (data & 0xF1FFFFFF);
	}

	void SetX(uint32_t x) {
		x = x << 20;
		data = x |= (data & 0xFE0FFFFF);
	}

	void SetY(uint32_t y) {
		y = y << 15;
		data = y |= (data & 0xFFF07FFF);
	}

	void SetZ(uint32_t z) {
		z = z << 10;
		data = z |= (data & 0xFFFF83FF);
	}

	void SetU(uint32_t u) {
		u = u << 5;
		data = u |= (data & 0xFFFFFC1F);
	}

	void SetV(uint32_t v) {
		data = v |= (data & 0xFFFFFFE0);
	}

	void SetAll(uint32_t x, uint32_t y, uint32_t z, uint32_t u, uint32_t v, uint32_t face) {
		SetX(x);
		SetY(y);
		SetZ(z);
		SetU(u);
		SetV(v);

		SetFace(face);
	}
};


//
// Define methods
//

void MeshBuilderNew32::BuildMesh(const uint8_t chunk[], std::vector<VERT_TYPE>& verts) {
	VisibleBlocks = 0;

	uint8_t* faces = FindVisibleFaces(chunk);
	MeshSize* meshedFaces = new MeshSize[CHUNK_CUBED](MeshSize());

	for (Face face = BACK; face < RIGHT + 1; face = static_cast<Face>(face << 1)) {
		GreedyMesh(faces, face, meshedFaces);
		GenerateGSInput(meshedFaces, face, verts);
	}

	delete[] meshedFaces;
	delete[] faces;
}

void MeshBuilderNew32::BuildMesh(Chunk* chunk, WorldManager* world, std::vector<VERT_TYPE>& verts) {
	VisibleBlocks = 0;

	uint8_t* faces = FindVisibleFacesCHUNK(chunk, world);

	/*for (int i = 0; i < CHUNK_SIZE; i++) {
		std::cout << (int)faces[i] << std::endl;
	}*/

	MeshSize* meshedFaces = new MeshSize[CHUNK_CUBED](MeshSize());

	for (Face face = BACK; face < RIGHT + 1; face = static_cast<Face>(face << 1)) {
		GreedyMesh(faces, face, meshedFaces);
		GenerateGSInput(meshedFaces, face, verts);
	}


	//std::cout << "Finished!";

	delete[] meshedFaces;
	delete[] faces;
}

uint8_t* MeshBuilderNew32::FindVisibleFaces(const uint8_t chunk[]) {
	uint8_t* faces = new uint8_t[CHUNK_CUBED]();

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				int index = ConvertCoords(x, z, y);

				if (chunk[index]) {
					if (z == 0) faces[index] |= BACK;
					if (z == CHUNK_SIZE - 1) faces[index] |= FRONT;

					if (y == CHUNK_SIZE - 1) faces[index] |= TOP;
					if (y == 0) faces[index] |= BOTTOM;

					if (x == 0) faces[index] |= LEFT;
					if (x == CHUNK_SIZE - 1) faces[index] |= RIGHT;

					int adjIndex = ConvertCoords(x, z - 1, y);
					if (z != 0 && !chunk[adjIndex]) faces[index] |= BACK;

					adjIndex = ConvertCoords(x, z + 1, y);
					if (z != CHUNK_SIZE - 1 && !chunk[adjIndex]) faces[index] |= FRONT;

					adjIndex = ConvertCoords(x, z, y + 1);
					if (y != CHUNK_SIZE - 1 && !chunk[adjIndex]) faces[index] |= TOP;

					adjIndex = ConvertCoords(x, z, y - 1);
					if (y != 0 && !chunk[adjIndex]) faces[index] |= BOTTOM;

					adjIndex = ConvertCoords(x - 1, z, y);
					if (x != 0 && !chunk[adjIndex]) faces[index] |= LEFT;

					adjIndex = ConvertCoords(x + 1, z, y);
					if (x != CHUNK_SIZE - 1 && !chunk[adjIndex]) faces[index] |= RIGHT;

					if (faces[index] != 0) VisibleBlocks += 1;
				}
			}
		}
	}

	return faces;
}

uint8_t* MeshBuilderNew32::FindVisibleFacesCHUNK(Chunk* chunk, WorldManager* world) {
	uint8_t* faces = new uint8_t[CHUNK_CUBED]();

	int chunkX = chunk->CHUNK_LOCATION.x;
	int chunkY = chunk->CHUNK_LOCATION.y;
	int chunkZ = chunk->CHUNK_LOCATION.z;

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				int index = chunk->ConvertCoords(x, y, z);

				if (chunk->GetBlock(index).ID) {
					uint8_t visibleFaces = 0;

					if (z == 0) {
						Utils::vec3 adjBlockLoc(x, y, 31);
						Utils::vec3 adjChunk(chunkX, chunkY, chunkZ - 1);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= BACK;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x, y, z - 1);
						if (!chunk->GetBlock(index-CHUNK_SIZE).ID) visibleFaces |= BACK;
					}

					if (z == CHUNK_SIZE - 1) {
						Utils::vec3 adjBlockLoc(x, y, 0);
						Utils::vec3 adjChunk(chunkX, chunkY, chunkZ + 1);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= FRONT;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x, y, z + 1);
						if (!chunk->GetBlock(index+CHUNK_SIZE).ID) visibleFaces |= FRONT;
					}

					if (y == CHUNK_SIZE - 1) {
						Utils::vec3 adjBlockLoc(x, 0, z);
						Utils::vec3 adjChunk(chunkX, chunkY + 1, chunkZ);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= TOP;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x, y + 1, z);
						if (!chunk->GetBlock(index+CHUNK_SQUARED).ID) visibleFaces |= TOP;
					}

					if (y == 0) {
						Utils::vec3 adjBlockLoc(x, 31, z);
						Utils::vec3 adjChunk(chunkX, chunkY - 1, chunkZ);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= BOTTOM;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x, y - 1, z);
						if (!chunk->GetBlock(index - CHUNK_SQUARED).ID) visibleFaces |= BOTTOM;
					}

					if (x == 0) {
						Utils::vec3 adjBlockLoc(31, y, z);
						Utils::vec3 adjChunk(chunkX - 1, chunkY, chunkZ);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= LEFT;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x - 1, y, z);
						if (!chunk->GetBlock(index-1).ID) visibleFaces |= LEFT;
					}

					if (x == CHUNK_SIZE - 1) {
						Utils::vec3 adjBlockLoc(0, y, z);
						Utils::vec3 adjChunk(chunkX + 1, chunkY, chunkZ);
						if (!world->GetBlock(adjBlockLoc, adjChunk).ID) visibleFaces |= RIGHT;
					}
					else {
						//int adjIndex = chunk->ConvertCoords(x + 1, y, z);
						if (!chunk->GetBlock(index+1).ID) visibleFaces |= RIGHT;
					}

					faces[index] = visibleFaces;
					if (visibleFaces != 0) VisibleBlocks += 1;
				}
			}
		}
	}

	return faces;
}

/*uint8_t* MeshBuilderNew32::FindVisibleFacesCHUNK(Chunk* chunk, WorldManager* world) {
	uint8_t* faces = new uint8_t[CHUNK_CUBED]();

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				int index = chunk->ConvertCoords(x, y, z);

				if (chunk->GetBlock(index).ID) {


					Utils::vec3 adjBlockLoc = Utils::vec3(x, y, 31);
					Utils::vec3 adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x, chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z - 1);
					if (z == 0 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= BACK;
					adjBlockLoc = Utils::vec3(x, y, 0);
					adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x, chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z + 1);
					if (z == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= FRONT;

					adjBlockLoc = Utils::vec3(x, 0, z);
					adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x, chunk->CHUNK_LOCATION.y+1, chunk->CHUNK_LOCATION.z);
					if (y == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= TOP;
					adjBlockLoc = Utils::vec3(x, 31, z);
					adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x, chunk->CHUNK_LOCATION.y - 1, chunk->CHUNK_LOCATION.z);
					if (y == 0 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= BOTTOM;

					adjBlockLoc = Utils::vec3(31, y, z);
					adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x-1, chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z);
					if (x == 0 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= LEFT;
					adjBlockLoc = Utils::vec3(0, y, z);
					adjChunk = Utils::vec3(chunk->CHUNK_LOCATION.x + 1, chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z);
					if (x == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc, adjChunk).ID) faces[index] |= RIGHT;

					int adjIndex = chunk->ConvertCoords(x, y, z - 1);
					if (z != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= BACK;

					adjIndex = chunk->ConvertCoords(x, y, z + 1);
					if (z != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= FRONT;

					adjIndex = chunk->ConvertCoords(x, y + 1, z);
					if (y != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= TOP;

					adjIndex = chunk->ConvertCoords(x, y - 1, z);
					if (y != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= BOTTOM;

					adjIndex = chunk->ConvertCoords(x - 1, y, z);
					if (x != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= LEFT;

					adjIndex = chunk->ConvertCoords(x + 1, y, z);
					if (x != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= RIGHT;

					if (faces[index] != 0) VisibleBlocks += 1;
				}
			}
		}
	}

	return faces;
}*/

/*uint8_t* MeshBuilderNew32::FindVisibleFacesCHUNK(Chunk* chunk, WorldManager* world) {
	uint8_t* faces = new uint8_t[CHUNK_CUBED]();

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				int index = chunk->ConvertCoords(x, y, z);
				Utils::vec3 worldLoc = chunk->GetWorldLoc(Utils::vec3(x, y, z));

				if (chunk->GetBlock(index).ID) {
					Utils::vec3 adjBlockLoc = Utils::vec3(worldLoc.x, worldLoc.y, worldLoc.z - 1);
					if (z == 0 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= BACK;
					adjBlockLoc = Utils::vec3(worldLoc.x, worldLoc.y, worldLoc.z + 1);
					if (z == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= FRONT;

					adjBlockLoc = Utils::vec3(worldLoc.x, worldLoc.y+1, worldLoc.z);
					if (y == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= TOP;
					adjBlockLoc = Utils::vec3(worldLoc.x, worldLoc.y - 1, worldLoc.z);
					if (y == 0 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= BOTTOM;

					adjBlockLoc = Utils::vec3(worldLoc.x-1, worldLoc.y, worldLoc.z);
					if (x == 0 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= LEFT;
					adjBlockLoc = Utils::vec3(worldLoc.x +1, worldLoc.y, worldLoc.z);
					if (x == CHUNK_SIZE - 1 && !world->GetBlock(adjBlockLoc).ID) faces[index] |= RIGHT;

					int adjIndex = chunk->ConvertCoords(x, y, z - 1);
					if (z != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= BACK;

					adjIndex = chunk->ConvertCoords(x, y, z + 1);
					if (z != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= FRONT;

					adjIndex = chunk->ConvertCoords(x, y + 1, z);
					if (y != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= TOP;

					adjIndex = chunk->ConvertCoords(x, y - 1, z);
					if (y != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= BOTTOM;

					adjIndex = chunk->ConvertCoords(x - 1, y, z);
					if (x != 0 && !chunk->GetBlock(adjIndex).ID) faces[index] |= LEFT;

					adjIndex = chunk->ConvertCoords(x + 1, y, z);
					if (x != CHUNK_SIZE - 1 && !chunk->GetBlock(adjIndex).ID) faces[index] |= RIGHT;

					if (faces[index] != 0) VisibleBlocks += 1;
				}
			}
		}
	}

	return faces;
}*/

void MeshBuilderNew32::GreedyMesh(uint8_t* faces, Face face, MeshSize* meshSizes) {
	std::fill(meshSizes, meshSizes + CHUNK_CUBED, MeshSize());

	uint8_t i, j, k;
	uint8_t* x = nullptr, * y = nullptr, * z = nullptr;

	switch (face) {
	case BACK:
	case FRONT:
		x = &k;
		y = &j;
		z = &i;

		break;
	case TOP:
	case BOTTOM:
		x = &k;
		y = &i;
		z = &j;

		break;
	case LEFT:
	case RIGHT:
		x = &i;
		y = &j;
		z = &k;

		break;
	}

	for (i = 0; i < CHUNK_SIZE; i++) {
		for (j = 1; j < CHUNK_SIZE; j++) {
			for (k = 0; k < CHUNK_SIZE; k++) {
				int index = ConvertCoords(*x, *z, *y);
				j -= 1;
				int pIndex = ConvertCoords(*x, *z, *y);
				j += 1;

				if (faces[index] & face && faces[pIndex] & face) {
					meshSizes[index].SetV(meshSizes[pIndex].GetV() + 1);

					meshSizes[pIndex].uv = 0;
				}

				if (!(faces[index] & face)) meshSizes[index].uv = 0;


				if (j == 1) {
					if (!(faces[pIndex] & face)) meshSizes[pIndex].uv = 0;
				}
			}
		}
	}



	for (i = 0; i < CHUNK_SIZE; i++) {
		for (j = 1; j < CHUNK_SIZE; j++) {
			for (k = 1; k < CHUNK_SIZE; k++) {
				int index = *x + (*z * CHUNK_SIZE) + (*y * CHUNK_SQUARED);
				j -= 1;
				int pV = ConvertCoords(*x, *z, *y);
				j += 1;
				k -= 1;
				int pU = ConvertCoords(*x, *z, *y);
				k += 1;

				if (meshSizes[index].GetU() || meshSizes[index].GetV()) {
					if (!meshSizes[pU].GetU() && !meshSizes[pU].GetV()) {
						continue;
					}
					else {
						if ((faces[index] & face) && (faces[pV] & face)) {
							if (meshSizes[index].GetV() == meshSizes[pU].GetV()) {
								meshSizes[index].SetU(meshSizes[pU].GetU() + 1);
								//meshSizes[index].face = face;

								meshSizes[pU].uv = 0;
							}
						}
					}
				}
			}
		}
	}
}

void MeshBuilderNew32::GenerateGSInput(MeshSize* meshedFaces, Face face, std::vector<uint32_t>& points) {
	uint32_t faceShaderVal = 0;

	switch (face) {
	case BACK:
		faceShaderVal = 0;
		break;

	case FRONT:
		faceShaderVal = 1;
		break;

	case TOP:
		faceShaderVal = 2;
		break;

	case BOTTOM:
		faceShaderVal = 3;
		break;

	case RIGHT:
		faceShaderVal = 4;
		break;

	case LEFT:
		faceShaderVal = 5;
		break;
	}

	for (int i = 0; i < CHUNK_CUBED; i++) {
		if (!meshedFaces[i].uv) {
			continue;
		}

		vec3 facePos(ConvertCoords(i));

		uint16_t x = facePos.x, y = facePos.y, z = facePos.z;
		uint16_t u = meshedFaces[i].GetU(), v = meshedFaces[i].GetV();

		LocalPoint p;

		p.SetAll(x, y, z, static_cast<uint32_t>(u - 1), static_cast<uint32_t>(v - 1), faceShaderVal);
		points.emplace_back(p.data);
	}
}