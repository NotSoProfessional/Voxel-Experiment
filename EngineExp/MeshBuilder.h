#pragma once
#include <cstdint>
#include <vector>
#include <iostream>

class MeshBuilder
{
	static const uint8_t CHUNK_SIZE = 8;
	static const uint16_t CHUNK_SQUARED = CHUNK_SIZE * CHUNK_SIZE;
	static const uint16_t CHUNK_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

public:
	void BuildMesh(uint8_t chunk[CHUNK_CUBED], std::vector<uint32_t>& verts) {
		uint8_t* faces = FindVisibleFaces(chunk);

		//std::vector<uint16_t> verts;

		meshSize* meshedFaces = new meshSize[CHUNK_CUBED](meshSize());

		for (Face face = BACK; face < RIGHT+1; face = static_cast<Face>(face << 1)) {
			//std::cout << face << std::endl;

			GreedyMesh(faces, face, meshedFaces);

			/*std::cout << "=======================================================================" << "\n";

			for (int i = 0; i < CHUNK_CUBED; i++) {
				vec3 loc = ConvertCoords(i);

			std::cout << (int) loc.x << ", " << (int)loc.y << ", " << (int)loc.z << ": "
				<< (int)meshedFaces[i].GetU() << ", " << (int)meshedFaces[i].GetV() << "\n";
			}*/

			//GenerateVertices(meshedFaces, face, verts);
			GenerateGeometryShaderInput(meshedFaces, face, verts);		
		}

		/*for (int i = 0; i < CHUNK_CUBED; i++) {
			std::cout << meshedFaces[i].GetU() << ", " << meshedFaces[i].GetV() << "\n";
		}*/
		delete[] meshedFaces;
		delete[] faces;
	}

	uint16_t ConvertCoords(uint8_t x, uint8_t z, uint8_t y) {
		return x + (z * CHUNK_SIZE) + (y * CHUNK_SQUARED);
	}

private:
	enum Face {
		BACK = 1,
		FRONT = 2,
		TOP = 4,
		BOTTOM = 8,
		LEFT = 16,
		RIGHT = 32
	};

	struct vec3 {
		uint8_t x;
		uint8_t z;
		uint8_t y;
	};

	struct meshSize {
	    uint8_t uv = 17;
		//uint8_t face = 0;

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

	uint16_t ConvertCoords(vec3 coords) {
		return coords.x + (coords.z * CHUNK_SIZE) + (coords.y * CHUNK_SQUARED);
	}

	vec3 ConvertCoords(uint16_t index) {
		vec3 vec;
		vec.x = index % CHUNK_SIZE;
		vec.z = (index / CHUNK_SIZE) % CHUNK_SIZE;
		vec.y = ((index / CHUNK_SIZE) / CHUNK_SIZE) % CHUNK_SIZE;

		return vec;
	}

	uint8_t* FindVisibleFaces(uint8_t chunk[CHUNK_CUBED]) {
		uint8_t* faces = new uint8_t[CHUNK_CUBED]();

		int visibleBlocks = 0;

		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				for (int x = 0; x < CHUNK_SIZE; x++) {
					//int index = x + (z * CHUNK_SIZE) + (y * CHUNK_SQUARED);

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

						if (faces[index] != 0) visibleBlocks += 1;
					}
				}
			}
		}

		std::cout << "Visible blocks in chunk: " << visibleBlocks << std::endl;

		return faces;
	}


	void GreedyMesh(uint8_t* faces, Face face, meshSize* meshSizes) {
		std::fill(meshSizes, meshSizes + CHUNK_CUBED, meshSize());
		//meshSize* meshSizes = new meshSize[CHUNK_CUBED](meshSize());

		uint8_t i, j, k;
		uint8_t* x = nullptr, *y = nullptr, *z = nullptr;

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

					//std::cout << (int)*x << ", " << (int)*y << ", " << (int)*z << "(" << index << ") P: " << (int)meshSizes[index].GetU() << ", " << (int)meshSizes[index].GetV();
					//std::cout << " || (" << pIndex << ") PI: " << (int)meshSizes[pIndex].GetU() << ", " << (int)meshSizes[pIndex].GetV();

					if (faces[index] & face && faces[pIndex] & face) {
						meshSizes[index].SetV(meshSizes[pIndex].GetV() + 1);
						//meshSizes[index].face = face;

						meshSizes[pIndex].uv = 0;
					}

					if (!(faces[index] & face)) meshSizes[index].uv = 0;


					if (j == 1) {
						//j = 0;
						//pIndex = ConvertCoords(*x, *z, *y);
						//j = 1;

						if (!(faces[pIndex] & face)) meshSizes[pIndex].uv = 0;
					}

					//std::cout << " A: " << (int)meshSizes[index].GetU() << ", " << (int)meshSizes[index].GetV() << "\n";
				}
			}
		}

		/*for (int i = 0; i < CHUNK_CUBED; i++) {
			vec3 loc = ConvertCoords(i);

			std::cout << (int)loc.x << ", " << (int)loc.y << ", " << (int)loc.z << ": "
				<< (int)meshSizes[i].GetU() << ", " << (int)meshSizes[i].GetV() << "\n";
		}*/


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

					//std::cout << (int)*x << ", " << (int)*y << ", " << (int)*z << "(" << index << ") P: " << (int)meshSizes[index].GetU() << ", " << (int)meshSizes[index].GetV();
					//std::cout << " || (" << pU << ") PI: " << (int)meshSizes[pU].GetU() << ", " << (int)meshSizes[pU].GetV();

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

					//std::cout << " A: " << (int)meshSizes[index].GetU() << ", " << (int)meshSizes[index].GetV() << "\n";
				}
			}
		}

		/*for (int i = 0; i < CHUNK_CUBED; i++) {
			std::cout << (int)meshSizes[i].GetU() << ", " << (int)meshSizes[i].GetV() << "\n";
		}*/

		/*std::cout << "=======================================================================" << "\n";

		for (int i = 0; i < CHUNK_CUBED; i++) {
			vec3 loc = ConvertCoords(i);

			std::cout << (int)loc.x << ", " << (int)loc.y << ", " << (int)loc.z << ": "
				<< (int)meshSizes[i].GetU() << ", " << (int)meshSizes[i].GetV() << "\n";
		}*/

		//return meshSizes;
	}

	struct vec5 {
		uint8_t x, y, z, u, v;
	};

	struct LocalVert {
		uint16_t data = 0;

		void SetFace(uint16_t face) {
			face = face << 14;
			data = face |= (data & 16383);
		}

		void SetX(uint16_t x) {
			x = x << 10;
			data = x |= (data & 50175);
		}

		void SetY(uint16_t y) {
			y = y << 6;
			data = y |= (data & 64575);
		}

		void SetZ(uint16_t z) {
			z = z << 2;
			data = z |= (data & 65475);
		}

		void SetU(uint16_t u) {
			u = u << 1;
			data = u |= (data & 65533);
		}

		void SetV(uint16_t v) {
			data = v |= (data & 65534);
		}

		void SetAll(vec5 vec) {
			SetX(vec.x);
			SetY(vec.y);
			SetZ(vec.z);
			SetU(vec.u);
			SetV(vec.v);
		}
	};

	void GenerateVertices(meshSize* meshedFaces, Face face, std::vector<uint16_t>& verts) {
		for (int i = 0; i < CHUNK_CUBED; i++) {
			if (!meshedFaces[i].uv) {
				continue;
			}

			vec3 facePos(ConvertCoords(i));

			uint8_t x = facePos.x, y = facePos.y, z = facePos.z;
			uint8_t u = meshedFaces[i].GetU(), v = meshedFaces[i].GetV();

			vec5 v1, v2, v3, v4, v5, v6;
			LocalVert lv1, lv2, lv3, lv4, lv5, lv6;

			switch (face) {
			case FRONT:
				z++;
			case BACK:
				x++;
				y++;

				v1 = { x, y, z, 1, 1 };
				v2 = { static_cast<uint8_t>(x - u), y, z, 0, 1 };
				v6 = { static_cast<uint8_t>(x - u), static_cast<uint8_t>(y - v), z, 0, 0 };
				v3 = { x, static_cast<uint8_t>(y - v), z, 1, 0 };

				break;

			case BOTTOM:
				y--;
			case TOP:


				break;

			case LEFT:
				x--;
			case RIGHT:


				break;
			}

			lv1.SetAll(v1);
			//lv1.SetFace(1);
			lv2.SetAll(v2);
			//lv2.SetFace(1);
			lv3.SetAll(v3);
			//lv3.SetFace(1);
			lv6.SetAll(v6);
			//lv6.SetFace(1);

			verts.reserve(verts.capacity() + 6);

			verts.emplace_back(lv1.data);
			verts.emplace_back(lv2.data);
			verts.emplace_back(lv6.data);
			verts.emplace_back(lv1.data);
			verts.emplace_back(lv6.data);
			verts.emplace_back(lv3.data);
		}
	}

	struct LocalPoint {
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

		/*void SetAll(vec5 vec) {
			SetX(vec.x);
			SetY(vec.y);
			SetZ(vec.z);
			SetU(vec.u);
			SetV(vec.v);
		}*/

		void SetAll(uint32_t x, uint32_t y, uint32_t z, uint32_t u, uint32_t v, uint32_t face) {
			SetX(x);
			SetY(y);
			SetZ(z);
			SetU(u);
			SetV(v);

			SetFace(face);
		}
	};

	void GenerateGeometryShaderInput(meshSize* meshedFaces, Face face, std::vector<uint32_t>& points) {
		for (int i = 0; i < CHUNK_CUBED; i++) {
			if (!meshedFaces[i].uv) {
				continue;
			}

			vec3 facePos(ConvertCoords(i));

			uint8_t x = facePos.x, y = facePos.y, z = facePos.z;
			uint8_t u = meshedFaces[i].GetU(), v = meshedFaces[i].GetV();

			uint32_t faceShaderVal = 0;

			LocalPoint p;

			switch (face) {
			case FRONT:
				faceShaderVal = 1;
				break;
			case BACK:
				faceShaderVal = 0;
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
};