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

Frustum MatrixUtils::GetFrustum(const glm::mat4x4& vp)
{
	Frustum frustum;

	// Extract planes from the view-projection matrix
	frustum.normal0 = glm::vec3(vp[0][3] + vp[0][0],
		vp[1][3] + vp[1][0],
		vp[2][3] + vp[2][0]);
	frustum.dist0 = vp[3][3] + vp[3][0];

	frustum.normal1 = glm::vec3(vp[0][3] - vp[0][0],
		vp[1][3] - vp[1][0],
		vp[2][3] - vp[2][0]);
	frustum.dist1 = vp[3][3] - vp[3][0];

	frustum.normal2 = glm::vec3(vp[0][3] + vp[0][1],
		vp[1][3] + vp[1][1],
		vp[2][3] + vp[2][1]);
	frustum.dist2 = vp[3][3] + vp[3][1];

	frustum.normal3 = glm::vec3(vp[0][3] - vp[0][1],
		vp[1][3] - vp[1][1],
		vp[2][3] - vp[2][1]);
	frustum.dist3 = vp[3][3] - vp[3][1];

	frustum.normal4 = glm::vec3(vp[0][3] + vp[0][2],
		vp[1][3] + vp[1][2],
		vp[2][3] + vp[2][2]);
	frustum.dist4 = vp[3][3] + vp[3][2];

	frustum.normal5 = glm::vec3(vp[0][3] - vp[0][2],
		vp[1][3] - vp[1][2],
		vp[2][3] - vp[2][2]);
	frustum.dist5 = vp[3][3] - vp[3][2];

	// Normalize the plane normals
	auto normalizePlane = [](glm::vec3& normal, float& distance) {
		float length = glm::length(normal);
		normal /= length;
		distance /= length;
		};

	normalizePlane(frustum.normal0, frustum.dist0);
	normalizePlane(frustum.normal1, frustum.dist1);
	normalizePlane(frustum.normal2, frustum.dist2);
	normalizePlane(frustum.normal3, frustum.dist3);
	normalizePlane(frustum.normal4, frustum.dist4);
	normalizePlane(frustum.normal5, frustum.dist5);

	// Compute frustum corners
	glm::mat4 invVP = glm::inverse(vp);
	glm::vec4 ndcCorners[8] = {
		{-1, -1, -1, 1}, {1, -1, -1, 1}, {-1, 1, -1, 1}, {1, 1, -1, 1},
		{-1, -1, 1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1}, {1, 1, 1, 1}
	};

	frustum.corner0 = invVP * ndcCorners[0];
	frustum.corner1 = invVP * ndcCorners[1];
	frustum.corner2 = invVP * ndcCorners[2];
	frustum.corner3 = invVP * ndcCorners[3];
	frustum.corner4 = invVP * ndcCorners[4];
	frustum.corner5 = invVP * ndcCorners[5];
	frustum.corner6 = invVP * ndcCorners[6];
	frustum.corner7 = invVP * ndcCorners[7];

	// Convert to 3D homogeneous coordinates
	for (auto& corner : { &frustum.corner0, &frustum.corner1, &frustum.corner2, &frustum.corner3,
						  &frustum.corner4, &frustum.corner5, &frustum.corner6, &frustum.corner7 })
	{
		*corner /= corner->w;
	}

	return frustum;
}
