#pragma once

#include <vector>
#include "Mesh.hpp"
#include "Material.hpp"

class Model
{
public:
	struct MaterialMeshPair
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
	};

	std::vector<MaterialMeshPair> parts;

	Model() = default;
	~Model() = default;
};