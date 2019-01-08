#pragma once

#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <vector>
#include <map>

#include "ShaderStructs.hpp"

namespace sponza
{
	class Context;
	class Material;
}

namespace MLK
{
    /// <summary>
    /// Implementation to convert all sponza::Materials from a given scene into shader compatible structures. The intention is that these materials
    /// can be sent to the GPU once and indexed via uniforms (instead of having to pass materials each draw).
    /// </summary>
	class MaterialManager
	{
	public:
		MaterialManager(const sponza::Context& scene);

		// Allows the user to update the materials from the scene in case some material are not static.
		void updateMaterialDataFromScene();

		// Gets the material data from the scene
		const std::vector<ShaderMaterial>& getMaterialData() const;

		// Converts a given sponza material id into an index which will be used when shading.
		GLuint lookupMaterialId(GLuint sponaMaterialId) const;

	private:
		const sponza::Context& m_scene;

		std::vector<ShaderMaterial> m_materials;
		std::map<GLuint, GLuint> m_materialIdToMaterialIndex;

        void generateMaterialData();
	};
}