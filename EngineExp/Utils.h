#pragma once

#include <stdexcept>
#include <glm/glm.hpp>
#include <iostream>

static class Utils {
public:
    static void PrintProgress(float progress) {
        HideCursor();
        int barWidth = 70;

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "#";
            else if (i == pos) std::cout << "";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        if (progress > 0.96) {
            ShowCursor();
        }
    }

    static void HideCursor() {
        std::cout << "\033[?25l";
    }

    // Function to show the cursor
    static void ShowCursor() {
        std::cout << "\033[?25h";
    }

	static int ConvertCoords() {
		throw std::logic_error("Not implemented");
		//return coords.x + (coords.z * CHUNK_SIZE) + (coords.y * CHUNK_SQUARED);
	}


	uint32_t CoordWorldTo32Chunk(glm::vec3 coords) {
		return coords.x + (coords.z * 32) + (coords.y * 32*32);
	}

    struct vec3 {
        int x;
        int y;
        int z;
        vec3(int x, int y, int z) : x(x), y(y), z(z) {
            coords[0] = x;
            coords[1] = y;
            coords[2] = z;
        }

        float coords[3];

        //vec3() = default;
        vec3(const vec3& other) = default;
        vec3& operator=(const vec3& other) = default;

        bool operator==(const vec3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

};

namespace std {
    template<>
    struct hash<Utils::vec3> {
        std::size_t operator()(const Utils::vec3& k) const {
            std::size_t h1 = std::hash<int>()(k.x);
            std::size_t h2 = std::hash<int>()(k.y);
            std::size_t h3 = std::hash<int>()(k.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}