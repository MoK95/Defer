#include "MeshManager.hpp"

#include <sponza/Camera.hpp>
#include <sponza/Context.hpp>
#include <sponza/Mesh.hpp>

#include <vector>

namespace MLK
{
    namespace MeshUtils
    {
        std::unique_ptr<const VertexData> generateVertexData(const sponza::Context& scene, const std::vector<sponza::Mesh>& meshArray)
        {
            std::unique_ptr<VertexData> vertexData{ new VertexData() };
            auto& vertexArray = vertexData->VertexArray;
            auto& elementArray = vertexData->ElementArray;
            auto& instanceArray = vertexData->InstanceIdArray;

            GLuint totalInstances = 0;
            // Get all mesh data and store in vertices collection.
            for (const auto& mesh : meshArray)
            {
                // Get current mesh data.
                const auto& positions = mesh.getPositionArray();
                const auto& normals = mesh.getNormalArray();
                const auto& uv0s = mesh.getTextureCoordinateArray();
                const auto& meshElements = mesh.getElementArray();

                const auto& vertexCount = positions.size();

                // Early out if no positions are present.
                if (vertexCount == 0)
                {
                    return std::move(vertexData);
                }

                Mesh currentMesh = MLK::Mesh();

                // Set mesh indexes for elements.
                currentMesh.FirstElement = GLuint(elementArray.size());
                currentMesh.ElementCount = GLuint(meshElements.size());

                // Set element data.
                elementArray.insert(elementArray.end(), meshElements.begin(), meshElements.end());

                // Set mesh indexes for vertices.
                currentMesh.FirstVertex = GLuint(vertexArray.size());

                // Set vertex data.
                vertexArray.reserve(vertexArray.size() + vertexCount);

                // NOTE: Check if the mesh has UVs. If not the data is padded to be glm::vec2(0,0). This comes at the cost of memory
                // but seems nicer than having 2 vaos which would force double the draw calls.
                bool hasUv = uv0s.size() == vertexCount;
                for (size_t j = 0; j < vertexCount; ++j)
                {
                    MLK::FullVertex vertex;
                    vertex.Position = (const glm::vec3&)positions[j];
                    vertex.Normal = (const glm::vec3&)normals[j];
                    if (hasUv)
                    {
                        vertex.UV0 = (const glm::vec2&)uv0s[j];
                    }
                    vertexArray.push_back(vertex);
                }

                GLuint instanceCount = (const GLuint&)scene.getInstancesByMeshId(mesh.getId()).size();

                // Instance count.
                currentMesh.InstanceCount = instanceCount;
                // Base instance.
                currentMesh.BaseInstance = totalInstances;

                // Fill instance array. The instance array is a counter from 0 to N instances used for shading.
                for (GLuint i = 0; i < instanceCount; ++i)
                {
                    instanceArray.push_back(totalInstances);
                    totalInstances++;
                }

                vertexData->MeshArray.push_back(currentMesh);
            }

            return std::move(vertexData);
        }

        std::unique_ptr<const VertexBuffers> generateVertexBuffers(const VertexData& vertexData)
        {
            std::unique_ptr<VertexBuffers> vertexBuffers{ new VertexBuffers };

            const auto& vertices = vertexData.VertexArray;
            const auto& elements = vertexData.ElementArray;
            const auto& instanceIds = vertexData.InstanceIdArray;

            Utils::genBuffer(vertexBuffers->VertexVBO, GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), (void*)vertices.data());
            Utils::genBuffer(vertexBuffers->ElementVBO, GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(elements[0]), (void*)elements.data());
            Utils::genBuffer(vertexBuffers->InstanceIdVBO, GL_ARRAY_BUFFER, instanceIds.size() * sizeof(instanceIds[0]), (void*)instanceIds.data());

            return std::move(vertexBuffers);
        }

        GLuint generateVertexArrayObject(const VertexBuffers& vertexBuffers)
        {
            GLuint id = 0;
            glGenVertexArrays(1, &id);
            glBindVertexArray(id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffers.ElementVBO);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers.VertexVBO);
            glEnableVertexAttribArray(AttribLocation::Position);
            glVertexAttribPointer(AttribLocation::Position, 3, GL_FLOAT, GL_FALSE, sizeof(MLK::FullVertex), TGL_BUFFER_OFFSET_OF(MLK::FullVertex, Position));
            glEnableVertexAttribArray(AttribLocation::Normal);
            glVertexAttribPointer(AttribLocation::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(MLK::FullVertex), TGL_BUFFER_OFFSET_OF(MLK::FullVertex, Normal));
            glEnableVertexAttribArray(AttribLocation::UV0);
            glVertexAttribPointer(AttribLocation::UV0, 2, GL_FLOAT, GL_FALSE, sizeof(MLK::FullVertex), TGL_BUFFER_OFFSET_OF(MLK::FullVertex, UV0));
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers.InstanceIdVBO);
            glEnableVertexAttribArray(AttribLocation::InstanceID);
            glVertexAttribIPointer(AttribLocation::InstanceID, 1, GL_UNSIGNED_INT, 0, (GLvoid*)0);
            glVertexAttribDivisor(AttribLocation::InstanceID, 1);
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            return id;
        }

        GLuint generateDrawCommandBuffer(const VertexData& vertexData)
        {
            // Hacked command buffer.
            const auto& commands = vertexData.MeshArray;

            GLuint id = 0;
            glGenBuffers(1, &id);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id);
            glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(commands[0]), (void*)commands.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

            return id;
        }
        
        DrawData generateDrawData(const VertexBuffers& vertexBuffers, const VertexData& vertexData)
        {
            DrawData drawData;

            drawData.VaoId = generateVertexArrayObject(vertexBuffers);

            drawData.DrawCommandCount = (const GLuint&)vertexData.MeshArray.size();
            
            drawData.DrawCommandBufferId = generateDrawCommandBuffer(vertexData);

            return drawData;
        }
    }
}