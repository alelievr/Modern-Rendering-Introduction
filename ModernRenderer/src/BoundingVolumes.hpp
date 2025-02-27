#pragma once

#include <glm/glm.hpp>

class AABB
{
public:
	glm::vec3 min;
	glm::vec3 max;
};

class OBB
{
public:
	glm::vec3 right;
	float extentRight;
	glm::vec3 up;
	float extentUp;
	glm::vec3 center;
	float extentForward;

	OBB() = default;
	OBB(const AABB& aabb, const glm::mat4x4& transform);
};
