#pragma once

#include <glm/glm.hpp>
#include <array>

struct AABB;

struct Plane {
    glm::vec3 normal;
    float distance;

    Plane() : normal(0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}

    float distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

class Frustum {
public:
    enum Side {
        LEFT = 0,
        RIGHT,
        TOP,
        BOTTOM,
        NEAR,
        FAR,
        COUNT
    };

    Frustum() = default;

    void extractFromViewProj(const glm::mat4& viewProj);
    bool testAABB(const AABB& aabb) const;
    bool testAABB(const AABB& localAABB, const glm::mat4& modelMatrix) const;

    const Plane& getPlane(Side side) const { return planes[side]; }

private:
    std::array<Plane, COUNT> planes;
};
