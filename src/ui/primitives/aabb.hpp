#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

struct Vertex;

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

    glm::vec3 getCenter() const { return (min + max) * 0.5f; }
    glm::vec3 getExtents() const { return (max - min) * 0.5f; }

    AABB transform(const glm::mat4& modelMatrix) const;

    static AABB computeFromSubmesh(
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        uint32_t indexOffset,
        uint32_t indexCount);

    // compute AABB from all vertices
    static AABB computeFromVertices(const std::vector<Vertex>& vertices);
};
