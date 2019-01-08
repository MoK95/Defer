#pragma once

#include "Utils.hpp"

#include <memory>
#include <functional>
#include <tgl/tgl.h>

namespace sponza
{
    class Mesh;
    class Context;
	class GeometryBuilder;
}

namespace MLK
{
    /// <summary>
    /// Mesh structure built to match the openGL DrawElementsIndirectCommand so the mesh information can double as the draw command.
    /// </summary>
    struct Mesh
    {
        GLuint ElementCount = 0;
        GLuint InstanceCount = 0;
        GLuint FirstElement = 0;
        GLuint FirstVertex = 0;
        GLuint BaseInstance = 0;
    };

    /// <summary>
    /// Vertex structure for interleaving vertex buffer objects.
    /// </summary>
    struct FullVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 UV0;
    };

    /// <summary>
    /// Structure to hold all relevant data to drawing (VAO and DrawCommandBuffer).
    /// </summary>
    struct DrawData
    {
		std::function<void()> drawcall;
        GLuint DrawCommandCount = 0;
        GLuint DrawCommandBufferId = 0;
        GLuint VaoId = 0;
        GLuint ElementCount = 0;
    };

    /// <summary>
    /// Structure to store all vertex data required to draw a given mesh or collection of meshes.
    /// </summary>
    struct VertexData
    {
        std::vector<Mesh> MeshArray;
        std::vector<FullVertex> VertexArray;
        std::vector<GLuint> ElementArray;
        std::vector<GLuint> InstanceIdArray;
    };

    /// <summary>
    /// Structure to store buffers IDs for a set of VertexData.
    /// </summary>
    struct VertexBuffers
    {
        GLuint VertexVBO = 0;
        GLuint ElementVBO = 0;
        GLuint InstanceIdVBO = 0;
    };

    namespace MeshUtils
    {
        // Generates a set of vertex data for the given sponza::Context and sponza::Mesh collection.
        std::unique_ptr<const VertexData> generateVertexData(const sponza::Context& scene, const std::vector<sponza::Mesh>& meshArray);

        // Creates and fills buffers for the given VertexData.
        std::unique_ptr<const VertexBuffers> generateVertexBuffers(const VertexData& vertexData);

        // Creates a draw command buffer for the given VertexData.
        GLuint generateDrawCommandBuffer(const VertexData& vertexData);

        // Creates a vertex array object from the given VertexBuffers.
        GLuint generateVertexArrayObject(const VertexBuffers& vertexBuffers);

        // Generates draw commands for the given VertexBuffers and VertexData.
        DrawData generateDrawData(const VertexBuffers& vertexBuffers, const VertexData& vertexData);
    }
}