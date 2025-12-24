#include "frustum.hpp"
#include "../ui/primitives/aabb.hpp"
#include <glm/gtc/matrix_access.hpp>

void Frustum::extractFromViewProj(const glm::mat4& vp) {
    glm::vec4 row0 = glm::row(vp, 0);
    glm::vec4 row1 = glm::row(vp, 1);
    glm::vec4 row2 = glm::row(vp, 2);
    glm::vec4 row3 = glm::row(vp, 3);

    glm::vec4 left   = row3 + row0;
    glm::vec4 right  = row3 - row0;
    glm::vec4 bottom = row3 + row1;
    glm::vec4 top    = row3 - row1;
    glm::vec4 near   = row3 + row2;
    glm::vec4 far    = row3 - row2;

    auto normalizePlane = [](const glm::vec4& p) -> Plane {
        float len = glm::length(glm::vec3(p));
        return Plane(glm::vec3(p) / len, p.w / len);
    };

    planes[LEFT]   = normalizePlane(left);
    planes[RIGHT]  = normalizePlane(right);
    planes[BOTTOM] = normalizePlane(bottom);
    planes[TOP]    = normalizePlane(top);
    planes[NEAR]   = normalizePlane(near);
    planes[FAR]    = normalizePlane(far);
}

bool Frustum::testAABB(const AABB& aabb) const {
    for (int i = 0; i < COUNT; ++i) {
        const Plane& plane = planes[i];

        glm::vec3 pVertex(
            plane.normal.x >= 0 ? aabb.max.x : aabb.min.x,
            plane.normal.y >= 0 ? aabb.max.y : aabb.min.y,
            plane.normal.z >= 0 ? aabb.max.z : aabb.min.z
        );

        if (plane.distanceToPoint(pVertex) < 0) {
            return false;
        }
    }
    return true;
}

/*
    compute a world-space AABB by transforming all 8 corners of the local AABB and finding 
    the new axis-aligned bounds. This is conservative but guarantees we never incorrectly cull visible geometry.
*/
bool Frustum::testAABB(const AABB& localAABB, const glm::mat4& modelMatrix) const {
    glm::vec3 corners[8] = {
        {localAABB.min.x, localAABB.min.y, localAABB.min.z},
        {localAABB.max.x, localAABB.min.y, localAABB.min.z},
        {localAABB.min.x, localAABB.max.y, localAABB.min.z},
        {localAABB.max.x, localAABB.max.y, localAABB.min.z},
        {localAABB.min.x, localAABB.min.y, localAABB.max.z},
        {localAABB.max.x, localAABB.min.y, localAABB.max.z},
        {localAABB.min.x, localAABB.max.y, localAABB.max.z},
        {localAABB.max.x, localAABB.max.y, localAABB.max.z},
    };

    glm::vec3 worldMin(FLT_MAX);
    glm::vec3 worldMax(-FLT_MAX);

    for (int i = 0; i < 8; ++i) {
        glm::vec3 worldCorner = glm::vec3(modelMatrix * glm::vec4(corners[i], 1.0f));
        worldMin = glm::min(worldMin, worldCorner);
        worldMax = glm::max(worldMax, worldCorner);
    }

    AABB worldAABB(worldMin, worldMax);
    return testAABB(worldAABB);
}
