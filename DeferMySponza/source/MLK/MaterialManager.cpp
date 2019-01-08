#include "MaterialManager.hpp"

// Camera not used but required for Context.hpp to compile.
#include <sponza/Camera.hpp>
#include <sponza/Context.hpp>
#include <sponza/Material.hpp>

namespace MLK
{
	MaterialManager::MaterialManager(const sponza::Context& scene) :
		m_scene(scene)
	{
		generateMaterialData();
	}

	void MaterialManager::updateMaterialDataFromScene()
	{
		generateMaterialData();
	}

	const std::vector<ShaderMaterial>& MaterialManager::getMaterialData() const
	{
		return m_materials;
	}

	void MaterialManager::generateMaterialData()
	{
		const auto& sponzaMaterials = m_scene.getAllMaterials();
		const auto materialCount = sponzaMaterials.size();
		m_materials.clear();
		m_materials.reserve(materialCount);
		for (unsigned int i = 0; i < materialCount; ++i)
		{
			const auto& sponzaMaterial = sponzaMaterials[i];
			m_materialIdToMaterialIndex[sponzaMaterial.getId()] = i;
			m_materials.push_back(ShaderMaterial(sponzaMaterial));
		}

		// Copy default material for SSR.
		sponza::Material reflection(sponzaMaterials[0]);
		m_materialIdToMaterialIndex[100] = materialCount;
		m_materials.push_back(reflection);
	}

	GLuint MaterialManager::lookupMaterialId(GLuint sponzaId) const
	{
		return m_materialIdToMaterialIndex.at(sponzaId);
	}
}