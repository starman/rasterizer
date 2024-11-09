#include <vector>
#include <glm/glm.hpp>
#include <sec_api/stdio_s.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/matrix_clip_space.hpp>

static const auto SCREEN_WIDTH = 1280;
static const auto SCREEN_HEIGHT = 720;

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define TO_RASTER(v) glm::vec4((SCREEN_WIDTH  * (v.x + v.w) / 2), (SCREEN_HEIGHT * (v.w - v.y) / 2), v.z, v.w)

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

void initialize_scene_objects(std::vector<glm::mat4>& objects) {
    const glm::mat4 identity(1.0f);

    glm::mat4 M0 = glm::translate(identity, glm::vec3(0, 0, 2.0f));
    M0 = glm::rotate(M0, glm::radians(45.0f), glm::vec3(0, 1, 0));

    glm::mat4 M1 = glm::translate(identity, glm::vec3(-3.75f, 0, 0));
    M1 = glm::rotate(M1, glm::radians(30.0f), glm::vec3(1, 0, 0));

    glm::mat4 M2 = glm::translate(identity, glm::vec3(3.75f, 0, 0));
    M2 = glm::rotate(M2, glm::radians(60.0f), glm::vec3(0, 1, 0));

    glm::mat4 M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
    M3 = glm::rotate(M3, glm::radians(90.0f), glm::vec3(0, 0, 1));

    objects.push_back(M0);
    objects.push_back(M1);
    objects.push_back(M2);
    objects.push_back(M3);
}

glm::vec4 vs(const glm::vec3& pos, const glm::mat4& M, const glm::mat4& V, const glm::mat4& P) {
    return (P * V * M * glm::vec4(pos, 1.0f));
}

bool evaluate_edge_function(const glm::vec3& E, const glm::vec3& sample) {
    float result = (E.x * sample.x) + (E.y * sample.y) + E.z;

    if (result != 0.0f) {
        return result > 0.0f;
    }

    if (E.x != 0.0f) {
        return E.x > 0.0f;
    }

    return E.y >= 0.0f;
}

int main() {
    glm::vec3 vertices[] = {
        { 1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, 1.0f },
        { -1.0f, -1.0f, 1.0f },
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, 1.0f, -1.0f },
        {  1.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, -1.0f },
    };

    uint32_t indices[] = {
        1,3,0, 7,5,4, 4,1,0, 5,2,1, 2,7,3, 0,7,4, 1,2,3, 7,6,5, 4,5,1, 5,6,2, 2,6,7, 0,3,7
    };

    glm::vec3 colors[] = {
        glm::vec3(0, 0, 1),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 1, 1),
        glm::vec3(1, 1, 1),
        glm::vec3(1, 0, 1),
        glm::vec3(1, 1, 0)
    };

    std::vector<glm::vec3> frameBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, glm::vec3(0, 0, 0));

    std::vector<float> depthBuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0.0);

    std::vector<glm::mat4> objects;
    initialize_scene_objects(objects);

    float nearPlane = 0.1f;
    float farPlane = 100.f;
    glm::vec3 eye(0, 3.75, 6.5);
    glm::vec3 lookat(0, 0, 0);
    glm::vec3 up(0, 1, 0);

    glm::mat4 view = glm::lookAt(eye, lookat, up);
    glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), nearPlane, farPlane);

    for (size_t n = 0; n < objects.size(); n++) {
        for (uint32_t idx = 0; idx < ARR_SIZE(indices) / 3; idx++) {
            const glm::vec3& v0 = vertices[indices[idx * 3]];
            const glm::vec3& v1 = vertices[indices[idx * 3 + 1]];
            const glm::vec3& v2 = vertices[indices[idx * 3 + 2]];

            glm::vec4 v0Clip = vs(v0, objects[n], view, proj);
            glm::vec4 v1Clip = vs(v1, objects[n], view, proj);
            glm::vec4 v2Clip = vs(v2, objects[n], view, proj);

            glm::vec4 v0Homogen = TO_RASTER(v0Clip);
            glm::vec4 v1Homogen = TO_RASTER(v1Clip);
            glm::vec4 v2Homogen = TO_RASTER(v2Clip);

            glm::mat3 M = {
                { v0Homogen.x, v1Homogen.x, v2Homogen.x},
                { v0Homogen.y, v1Homogen.y, v2Homogen.y},
                { v0Homogen.w, v1Homogen.w, v2Homogen.w},
            };

            float det = glm::determinant(M);
            if (det >= 0.0f)
                continue;

            M = inverse(M);

            glm::vec3 E0 = M[0];
            glm::vec3 E1 = M[1];
            glm::vec3 E2 = M[2];

            glm::vec3 C = M * glm::vec3(1, 1, 1);

            for (auto y = 0; y < SCREEN_HEIGHT; y++) {
                for (auto x = 0; x < SCREEN_WIDTH; x++) {
                    glm::vec3 sample = { x + 0.5f, y + 0.5f, 1.0f };

                    bool inside0 = evaluate_edge_function(E0, sample);
                    bool inside1 = evaluate_edge_function(E1, sample);
                    bool inside2 = evaluate_edge_function(E2, sample);

                    if (inside0 && inside1 && inside2) {
                        float oneOverW = (C.x * sample.x) + (C.y * sample.y) + C.z;

                        if (oneOverW >= depthBuffer[x + y * SCREEN_WIDTH]) {
                            depthBuffer[x + y * SCREEN_WIDTH] = oneOverW;

                            frameBuffer[x + y * SCREEN_WIDTH] = colors[indices[3 * idx] % 6];
                        }
                    }
                }
            }
        }
    }

    remove("output.ppm");
    output_frame(frameBuffer, "output.ppm");
    return 0;
}
