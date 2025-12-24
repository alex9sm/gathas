#include "aabb.hpp"
#include "../../renderer/vertex.hpp"
#include <limits>

AABB AABB::computeFromSubmesh(
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices,
    uint32_t indexOffset,
    uint32_t indexCount)
{
    if (indexCount == 0 || vertices.empty()) {
        return AABB();
    }

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    for (uint32_t i = 0; i < indexCount; ++i) {
        uint32_t idx = indices[indexOffset + i];
        const glm::vec3& pos = vertices[idx].pos;

        minBounds = glm::min(minBounds, pos);
        maxBounds = glm::max(maxBounds, pos);
    }

    return AABB(minBounds, maxBounds);
}

AABB AABB::computeFromVertices(const std::vector<Vertex>& vertices)
{
    if (vertices.empty()) {
        return AABB();
    }

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    for (const auto& vertex : vertices) {
        minBounds = glm::min(minBounds, vertex.pos);
        maxBounds = glm::max(maxBounds, vertex.pos);
    }

    return AABB(minBounds, maxBounds);
}
