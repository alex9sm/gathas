#include "aabb.hpp"
#include "../../renderer/vertex.hpp"
#include <limits>
#include <glm/glm.hpp>

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

AABB AABB::transform(const glm::mat4& modelMatrix) const {
    glm::vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z},
    };

    glm::vec3 newMin(std::numeric_limits<float>::max());
    glm::vec3 newMax(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; ++i) {
        glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(corners[i], 1.0f));
        newMin = glm::min(newMin, worldCorner);
        newMax = glm::max(newMax, worldCorner);
    }

    return AABB(newMin, newMax);
}
