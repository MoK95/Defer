#include "MeshManager.hpp"

#include <sponza/Mesh.hpp>
#include <sponza/Camera.hpp>
#include <sponza/Context.hpp>
#include <sponza/GeometryBuilder.hpp>
#include <tsl/tsl.hpp>

#include <memory>

namespace MLK
{
    namespace MU = MeshUtils;

    MeshManager::MeshManager(const sponza::Context& scene) :
		m_scene(scene)
	{
		auto sponzaData = createVaoFromMeshCollection(sponza::GeometryBuilder().getAllMeshes(), MeshGroup::Sponza);
		sponzaData.drawcall = [sponzaData]()
		{
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, sponzaData.DrawCommandCount, 0);
		};
		m_meshGroups[MeshGroup::Sponza] = sponzaData;

		auto quadData = createQuadVao();
		quadData.drawcall = []()
		{
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		};
		m_meshGroups[MeshGroup::Quad] = quadData;

        auto sphereData = createSphereVao();
        sphereData.drawcall = [sphereData]()
        {
            glDrawElements(GL_TRIANGLES, sphereData.ElementCount, GL_UNSIGNED_INT, 0);
        };
        m_meshGroups[MeshGroup::Sphere] = sphereData;

        auto coneData = createConeVao();
        coneData.drawcall = [coneData]()
        {
            glDrawElements(GL_TRIANGLES, coneData.ElementCount, GL_UNSIGNED_INT, 0);
        };
        m_meshGroups[MeshGroup::Cone] = coneData;
	}

    MeshManager::~MeshManager()
    {
        // Ensure all generated vertex buffers are deleted.
        for (const auto bufferId : m_buffers)
        {
            glDeleteBuffers(1, &bufferId);
        }

        // Ensure all generated VAO and command buffers are deleted.
        for (const auto drawSet : m_meshGroups)
        {
            glDeleteVertexArrays(1, &drawSet.second.VaoId);
            glDeleteBuffers(1, &drawSet.second.DrawCommandBufferId);
        }
    }

	void MeshManager::drawMeshGroup(MeshGroup id)
	{
		// If the id has changed the data has to be set.
		if (id != m_currentMeshGroup)
		{
			updateMeshGroup(id);
		}

		m_meshGroups.at(m_currentMeshGroup).drawcall();
	}

	void MeshManager::updateMeshGroup(MeshGroup id)
	{
		m_currentMeshGroup = id;
		auto data = m_meshGroups.at(id);
		glBindVertexArray(data.VaoId);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, data.DrawCommandBufferId);
	}

	DrawData MeshManager::createQuadVao()
	{
		DrawData data; 
		
        std::vector<glm::vec4> posUV(4);
        posUV[0] = glm::vec4(-1, -1, 0, 0);
        posUV[1] = glm::vec4(1, -1, 1, 0);
        posUV[2] = glm::vec4(1, 1, 1, 1);
        posUV[3] = glm::vec4(-1, 1, 0, 1);

		GLuint vertexBuffer = 0;
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER,
            posUV.size() * sizeof(glm::vec4),
            posUV.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &data.VaoId);
		glBindVertexArray(data.VaoId);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glEnableVertexAttribArray(AttribLocation::Position);
		glVertexAttribPointer(AttribLocation::Position, 2, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec4), TGL_BUFFER_OFFSET(0));
        glEnableVertexAttribArray(AttribLocation::UV0);
        glVertexAttribPointer(AttribLocation::UV0, 2, GL_FLOAT, GL_FALSE,
            sizeof(glm::vec4), TGL_BUFFER_OFFSET_OF(glm::vec4, z));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_buffers.push_back(vertexBuffer);

		return data;
	}
    
    DrawData MeshManager::createVaoFromMeshCollection(const std::vector<sponza::Mesh>& meshCollection, MeshGroup id)
    {
        auto vertexData = MU::generateVertexData(m_scene, meshCollection);
        auto vertexBuffers = MU::generateVertexBuffers(*vertexData);
		auto drawData = MU::generateDrawData(*vertexBuffers, *vertexData);

        // Store buffer IDs. Not ideal but required in order to delete at the end.
        m_buffers.push_back(vertexBuffers->ElementVBO);
        m_buffers.push_back(vertexBuffers->VertexVBO);
        m_buffers.push_back(vertexBuffers->InstanceIdVBO);

		return drawData;
    }

    DrawData MeshManager::createSphereVao()
    {
        DrawData data;

        tsl::IndexedMeshPtr mesh = tsl::createSpherePtr(1.f, 12);
        mesh = tsl::cloneIndexedMeshAsTriangleListPtr(mesh.get());

        data.ElementCount= mesh->indexCount();

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            mesh->vertexCount() * sizeof(glm::vec3),
            mesh->positionArray(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLuint elementBuffer = 0;
        glGenBuffers(1, &elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            mesh->indexCount() * sizeof(unsigned int),
            mesh->indexArray(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &data.VaoId);
        glBindVertexArray(data.VaoId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(glm::vec3), 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_buffers.push_back(vbo);
        m_buffers.push_back(elementBuffer);

        return data;
    }

    DrawData MeshManager::createConeVao()
    {
        DrawData data;

        tsl::IndexedMeshPtr mesh = tsl::createConePtr(1.f, 1.f, 5);
        mesh = tsl::cloneIndexedMeshAsTriangleListPtr(mesh.get());

        data.ElementCount = mesh->indexCount();

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            mesh->vertexCount() * sizeof(glm::vec3),
            mesh->positionArray(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLuint elementBuffer = 0;
        glGenBuffers(1, &elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            mesh->indexCount() * sizeof(unsigned int),
            mesh->indexArray(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &data.VaoId);
        glBindVertexArray(data.VaoId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(glm::vec3), 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_buffers.push_back(vbo);
        m_buffers.push_back(elementBuffer);

        return data;
    }
}