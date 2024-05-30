#include "MeshBuilderNew8.h"

//
// Define Structs
//

struct MeshBuilderNew8::MeshSize {
	uint8_t uv = 17;

	uint8_t GetU() {
		return uv & 15;
	}

	void SetU(uint8_t u) {
		uv = uv & 240;
		uv |= u;
	}

	uint8_t GetV() {
		return (uv & 240) >> 4;
	}

	void SetV(uint8_t v) {
		v = v << 4;
		uv = v |= (uv & 15);
	}
};

struct MeshBuilderNew8::LocalPoint {
	uint32_t data = 0;

	void SetFace(uint32_t face) {
		face = face << 15;
		data = face |= (data & 4294737919);
	}

	void SetX(uint32_t x) {
		x = x << 12;
		data = x |= (data & 4294938623);
	}

	void SetY(uint32_t y) {
		y = y << 9;
		data = y |= (data & 4294963711);
	}

	void SetZ(uint32_t z) {
		z = z << 6;
		data = z |= (data & 4294966847);
	}

	void SetU(uint32_t u) {
		u = u << 3;
		data = u |= (data & 4294967239);
	}

	void SetV(uint32_t v) {
		data = v |= (data & 4294967288);
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

void MeshBuilderNew8::BuildMesh(const uint8_t chunk[], std::vector<VERT_TYPE>& verts) {
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

uint8_t* MeshBuilderNew8::FindVisibleFaces(const uint8_t chunk[]) {
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

void MeshBuilderNew8::GreedyMesh(uint8_t* faces, Face face, MeshSize* meshSizes) {
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

void MeshBuilderNew8::GenerateGSInput(MeshSize* meshedFaces, Face face, std::vector<uint32_t>& points) {
	for (int i = 0; i < CHUNK_CUBED; i++) {
		if (!meshedFaces[i].uv) {
			continue;
		}

		vec3 facePos(ConvertCoords(i));

		uint16_t x = facePos.x, y = facePos.y, z = facePos.z;
		uint16_t u = meshedFaces[i].GetU(), v = meshedFaces[i].GetV();

		uint32_t faceShaderVal = 0;

		LocalPoint p;

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

		p.SetAll(x, y, z, static_cast<uint32_t>(u - 1), static_cast<uint32_t>(v - 1), faceShaderVal);
		points.emplace_back(p.data);
	}
}