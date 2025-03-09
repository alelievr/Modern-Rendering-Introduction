#include "Model.hpp"

Model::Model(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	MaterialMeshPair pair;
	pair.mesh = mesh;
	pair.material = material;
	parts.push_back(pair);
}
