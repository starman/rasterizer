#include <vector>
#include <glm/glm.hpp>
#include <sec_api/stdio_s.h>

#include "glm/ext/scalar_constants.hpp"

static const auto SCREEN_WIDTH = 1280;
static const auto SCREEN_HEIGHT = 720;

#define TO_RASTER(v) glm::vec3((SCREEN_WIDTH * (v.x + 1.0f) / 2), (SCREEN_HEIGHT * (v.y + 1.f) / 2), 1.0f)

void output_frame(const std::vector<glm::vec3>& frameBuffer, const char* filename) {
    assert(frameBuffer.size() >= (SCREEN_WIDTH * SCREEN_HEIGHT));

    FILE* pFile = nullptr;
    fopen_s(&pFile, filename, "w");
    fprintf(pFile, "P3\n%d %d\n%d\n ", SCREEN_WIDTH, SCREEN_HEIGHT, 255);
    for (auto i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
    {
        uint32_t r = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].r, 0.0f, 1.0f));
        uint32_t g = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].g, 0.0f, 1.0f));
        uint32_t b = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].b, 0.0f, 1.0f));
        fprintf(pFile, "%d %d %d ", r, g, b);
    }
    fclose(pFile);
}

int main() {
    glm::vec3 v0(-0.5, 0.5, 1.0);
    glm::vec3 v1(0.5, 0.5, 1.0);
    glm::vec3 v2(0.0, -0.5, 1.0);

    v0 = TO_RASTER(v0);
    v1 = TO_RASTER(v1);
    v2 = TO_RASTER(v2);

    glm::vec3 c0(1, 0, 0);
    glm::vec3 c1(0, 1, 0);
    glm::vec3 c2(0, 0, 1);

    glm::mat3 M = {
        { v0.x, v1.x, v2.x},
        { v0.y, v1.y, v2.y},
        { v0.z, v1.z, v2.z},
    };

    M = glm::inverse(M);

    glm::vec3 E0 = M * glm::vec3(1, 0, 0);
    glm::vec3 E1 = M * glm::vec3(0, 1, 0);
    glm::vec3 E2 = M * glm::vec3(0, 0, 1);

    std::vector<glm::vec3> frameBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, glm::vec3(0, 0, 0));

    for (auto y = 0; y < SCREEN_HEIGHT; y++) {
        for (auto x = 0; x < SCREEN_WIDTH; x++) {
            glm::vec3 sample = {x + 0.5f, y + 0.5f, 1.0f};

            float alpha = glm::dot(E0, sample);
            float beta = glm::dot(E1, sample);
            float gamma = glm::dot(E2, sample);

            if ((alpha >= 0.0f) && (beta >= 0.0f) && (gamma >= 0.0f)) {
                assert(((alpha + beta + gamma) - 1.0f) <= glm::epsilon<float>());

                frameBuffer[x + y * SCREEN_WIDTH] = glm::vec3(c0 * alpha + c1 * beta + c2 * gamma);
            }
        }
    }

    output_frame(frameBuffer, "output.ppm");
    return 0;
}
