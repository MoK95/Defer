#pragma once

#include "MeshUtils.hpp"

#include <tgl/tgl.h>

#include <unordered_map>
#include <vector>

namespace sponza
{
    class Mesh;
    class Context;
	class GeometryBuilder;
}

namespace MLK
{
    struct Mesh;

    /// <summary>
    /// Enum to specify collections of meshes to be drawn. This could be a string or integer allowing for new
    /// groups to be added without having to edit an enum, but this is fine for the current purposes.
    /// </summary>
    enum MeshGroup
    {
        None,
        Sponza,
		Quad,
        Sphere,
        Cone
    };

    /// <summary>
    /// A class that is responsible for mesh and instance data. This includes vertex buffers, vertex array objects
    /// and draw command buffers required to draw the given data.
    /// </summary>
	class MeshManager
	{
	public:
        MeshManager(const sponza::Context& scene);
        ~MeshManager();

		void drawMeshGroup(MeshGroup id);

	private:
		const sponza::Context& m_scene;
		void updateMeshGroup(MeshGroup id);

		// Creates a VAO from a given sponza mesh collection and stores the resulting VAO under the given id.
		DrawData createVaoFromMeshCollection(const std::vector<sponza::Mesh>& meshCollection, MeshGroup id);

		DrawData createQuadVao();
        DrawData createSphereVao();
        DrawData createConeVao();

        MeshGroup m_currentMeshGroup = MeshGroup::None;
		std::unordered_map<MeshGroup, DrawData> m_meshGroups;
        std::vector<GLuint> m_buffers;
	};
}