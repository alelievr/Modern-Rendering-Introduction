#pragma once

#include <vector>
#include "Mesh.hpp"
#include "Material.hpp"

class Model
{
public:
	struct MaterialMeshPair
	{
		Mesh mesh;
		Material material;
	};

	std::vector<MaterialMeshPair> parts;

	Model() = default;
	~Model() = default;
};