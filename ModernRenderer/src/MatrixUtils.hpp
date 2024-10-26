#pragma once

#include <glm/glm.hpp>

// Row major matrix utility functions
class MatrixUtils
{
public:
	static glm::mat4x4 Translation(const glm::vec3& translation);

	static glm::mat4x4 RotateX(float angle);
	static glm::mat4x4 RotateY(float angle);
	static glm::mat4x4 RotateZ(float angle);
	static glm::mat4x4 Rotation(const glm::vec3& rotation);
	
	static glm::mat4x4 Scale(const glm::vec3& scale);
	
	static glm::mat4x4 Perspective(float fov, float aspect, float near, float far);
	static glm::mat4x4 Orthographic(const glm::vec2& size, float aspect, float near, float far);

	static glm::mat4x4 Mul(const glm::mat4x4& left, const glm::mat4x4& right);
};