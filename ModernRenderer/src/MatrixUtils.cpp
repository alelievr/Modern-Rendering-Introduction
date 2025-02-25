#include "MatrixUtils.hpp"

glm::mat4x4 MatrixUtils::Translation(const glm::vec3& translation)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	// Bottom row stores the translation
	result[3][0] = translation[0];
	result[3][1] = translation[1];
	result[3][2] = translation[2];

	return result;
}

glm::mat4x4 MatrixUtils::RotateX(float angle)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	// Rotate around X axis
	result[1][1] = cos(angle);
	result[1][2] = -sin(angle);
	result[2][1] = sin(angle);
	result[2][2] = cos(angle);

	return result;
}

glm::mat4x4 MatrixUtils::RotateY(float angle)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	// Rotate around Y axis
	result[0][0] = cos(angle);
	result[0][2] = sin(angle);
	result[2][0] = -sin(angle);
	result[2][2] = cos(angle);

	return result;
}

glm::mat4x4 MatrixUtils::RotateZ(float angle)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	// Rotate around Z axis
	result[0][0] = cos(angle);
	result[0][1] = -sin(angle);
	result[1][0] = sin(angle);
	result[1][1] = cos(angle);

	return result;
}

glm::mat4x4 MatrixUtils::Rotation(const glm::vec3& rotation)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	float angleX = glm::radians(rotation.x);
	float angleY = glm::radians(rotation.y);
	float angleZ = glm::radians(rotation.z);

	// Rotate around X axis
	const auto& rotationX = RotateX(angleX);
	const auto& rotationY = RotateY(angleY);
	const auto& rotationZ = RotateZ(angleZ);

	return rotationX * rotationY * rotationZ;
}

glm::mat4x4 MatrixUtils::Scale(const glm::vec3& scale)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	result[0][0] = scale[0];
	result[1][1] = scale[1];
	result[2][2] = scale[2];

	return result;
}

glm::mat4x4 MatrixUtils::Perspective(float fov, float aspect, float near, float far)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	float halfTanFov = tan(glm::radians(fov) / 2.0f);

	result[0][0] = 1.0f / (aspect * halfTanFov);
	result[1][1] = 1.0f / halfTanFov;
	result[2][2] = far / (far - near);
	result[2][3] = 1.0f;
	result[3][2] = -(far * near) / (far - near);
	result[3][3] = 0.0f;

	return result;
}

glm::mat4x4 MatrixUtils::Orthographic(const glm::vec2& size, float aspect, float near, float far)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	result[0][0] = 2.0f / (size.x * aspect);
	result[1][1] = 2.0f / size.y;
	result[2][2] = 1.0f / (far - near);
	result[3][2] = near / (near - far);
	result[3][3] = 1.0f;

	return result;
}

glm::mat4x4 MatrixUtils::Mul(const glm::mat4x4& left, const glm::mat4x4& right)
{
	glm::mat4x4 result(1.0f); // Identity matrix

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			result[row][col] = left[row][0] * right[0][col] +
				left[row][1] * right[1][col] +
				left[row][2] * right[2][col] +
				left[row][3] * right[3][col];
		}
	}

	return result;
}

void MatrixUtils::GetFrustumPlanes(const glm::mat4x4& viewProjectionMatrix, glm::vec4 planes[6])
{
	// Left plane
	planes[0] = glm::vec4(viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0],
		viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0],
		viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0],
		viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0]);

	// Right plane
	planes[1] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0],
		viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0],
		viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0],
		viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0]);

	// Bottom plane
	planes[2] = glm::vec4(viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1],
		viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1],
		viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1],
		viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1]);

	// Top plane
	planes[3] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1],
		viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1],
		viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1],
		viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1]);

	// Near plane
	planes[4] = glm::vec4(viewProjectionMatrix[0][2],
		viewProjectionMatrix[1][2],
		viewProjectionMatrix[2][2],
		viewProjectionMatrix[3][2]);

	// Far plane
	planes[5] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2],
		viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2],
		viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2],
		viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2]);

	// Normalize the planes
	for (int i = 0; i < 6; i++)
	{
		float length = glm::length(glm::vec3(planes[i]));
		planes[i] /= length;
	}
}
