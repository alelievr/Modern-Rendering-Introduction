#include "BoundingVolumes.hpp"

OBB::OBB(const AABB& aabb, const glm::mat4x4& transform)
{
    glm::vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    glm::vec3 halfExtents = (aabb.max - aabb.min) * 0.5f;

    center = glm::vec3(glm::vec4(localCenter, 1.0f) * transform);

    glm::vec3 right = glm::vec3(transform[0]);
    glm::vec3 up = glm::vec3(transform[1]);
    glm::vec3 forward = glm::vec3(transform[2]);

    extentRight = glm::length(right * halfExtents.x);
    extentUp = glm::length(up * halfExtents.y);
    extentForward = glm::length(forward * halfExtents.z);

    this->right = glm::normalize(right);
    this->up = glm::normalize(up);
}
