#pragma once

#include <vector>
#include <string>

class MeshBuilder {
public:
    using VERT_TYPE = uint32_t;

    const uint16_t CHUNK_SIZE;
    const uint32_t CHUNK_SQUARED;
    const uint32_t CHUNK_CUBED;

    const std::string* SHADER;

    int VisibleBlocks = 0;

    MeshBuilder(uint16_t size, const std::string* shader) : CHUNK_SIZE(size), CHUNK_SQUARED(size* size), CHUNK_CUBED(size* size* size),
        SHADER(shader){}

    virtual void BuildMesh(const uint8_t chunk[], std::vector<VERT_TYPE>&) = 0;

    uint32_t ConvertCoords(uint16_t x, uint16_t z, uint16_t y) {
        return x + (z * CHUNK_SIZE) + (y * CHUNK_SQUARED);
    }
protected:
    enum Face {
        BACK = 1,
        FRONT = 2,
        TOP = 4,
        BOTTOM = 8,
        LEFT = 16,
        RIGHT = 32
    };

    struct vec3 {
        uint16_t x;
        uint16_t z;
        uint16_t y;
    };

    uint32_t ConvertCoords(vec3 coords) {
        return coords.x + (coords.z * CHUNK_SIZE) + (coords.y * CHUNK_SQUARED);
    }

    vec3 ConvertCoords(uint16_t index) {
        vec3 vec;
        vec.x = index % CHUNK_SIZE;
        vec.z = (index / CHUNK_SIZE) % CHUNK_SIZE;
        vec.y = ((index / CHUNK_SIZE) / CHUNK_SIZE) % CHUNK_SIZE;

        return vec;
    }
};