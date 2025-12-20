#include "mesh.hpp"
#include "commandbuffer.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace {
    void calculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        // start tangents at zero
        std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
        std::vector<glm::vec3> bitangents(vertices.size(), glm::vec3(0.0f));

        // calculate tangent and bitangent for each triangle
        for (size_t i = 0; i < indices.size(); i += 3) {
            uint32_t i0 = indices[i + 0];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            const glm::vec3& p0 = vertices[i0].pos;
            const glm::vec3& p1 = vertices[i1].pos;
            const glm::vec3& p2 = vertices[i2].pos;

            const glm::vec2& uv0 = vertices[i0].texCoord;
            const glm::vec2& uv1 = vertices[i1].texCoord;
            const glm::vec2& uv2 = vertices[i2].texCoord;

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;

            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
            float f = (std::abs(denom) > 1e-6f) ? 1.0f / denom : 0.0f;

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            glm::vec3 bitangent;
            bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;

            bitangents[i0] += bitangent;
            bitangents[i1] += bitangent;
            bitangents[i2] += bitangent;
        }

        // orthonormalize
        for (size_t i = 0; i < vertices.size(); ++i) {
            const glm::vec3& n = vertices[i].normal;
            glm::vec3 t = tangents[i];
            glm::vec3 b = bitangents[i];

            t = glm::normalize(t - n * glm::dot(n, t));

            float w = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;

            vertices[i].tangent = glm::vec4(t, w);
        }
    }
}

Mesh::Mesh() : totalIndexCount(0) {
}

Mesh::~Mesh() {
}

void Mesh::loadFromFile(const std::string& filepath, VmaAllocator allocator, CommandBuffer* commandBuffer) {
    // clear any existing data
    vertices.clear();
    indices.clear();

    processObjFile(filepath, vertices, indices);

    totalIndexCount = static_cast<uint32_t>(indices.size());

    // vertx -> gpu
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
    vertexBuffer.create(allocator, vertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        vertices.data(), commandBuffer);

    // index -> gpu
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();
    indexBuffer.create(allocator, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        indices.data(), commandBuffer);

    std::cout << "mesh loaded from " << filepath << ": "
        << vertices.size() << " vertices, "
        << indices.size() << " indices, "
        << submeshes.size() << " submeshes" << std::endl;
}

void Mesh::processObjFile(const std::string& filepath,
    std::vector<Vertex>& outVertices,
    std::vector<uint32_t>& outIndices) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string basedir;
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basedir = filepath.substr(0, lastSlash + 1);
    }

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), basedir.c_str())) {
        throw std::runtime_error("failed to load obj file: " + filepath + "\n" + warn + err);
    }

    if (!warn.empty()) {
        std::cout << "tinyobj warning: " << warn << std::endl;
    }

    // first pass: load raw vertices without deduplication to calculate tangents
    std::vector<Vertex> rawVertices;
    std::vector<uint32_t> rawIndices;
    std::vector<std::pair<uint32_t, uint32_t>> submeshRanges; // start index, count

    for (const auto& shape : shapes) {
        uint32_t submeshStartIndex = static_cast<uint32_t>(rawIndices.size());
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };

                if (idx.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                }
                else {
                    vertex.normal = { 0.0f, 1.0f, 0.0f };
                }

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (idx.texcoord_index >= 0) {
                    vertex.texCoord = {
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                    };
                }
                else {
                    vertex.texCoord = { 0.0f, 0.0f };
                }

                vertex.tangent = glm::vec4(0.0f);

                rawIndices.push_back(static_cast<uint32_t>(rawVertices.size()));
                rawVertices.push_back(vertex);
            }

            index_offset += fv;
        }

        uint32_t submeshIndexCount = static_cast<uint32_t>(rawIndices.size()) - submeshStartIndex;
        submeshRanges.emplace_back(submeshStartIndex, submeshIndexCount);

        int materialId = 0;
        if (!shape.mesh.material_ids.empty()) {
            materialId = shape.mesh.material_ids[0];
        }

        std::string materialName = (materialId >= 0 && materialId < materials.size()) ? materials[materialId].name : "";
        materialNames.push_back(materialName);
    }

    // calculate tangents on raw vertices
    calculateTangents(rawVertices, rawIndices);

    // second pass deduplicate vertices
    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    size_t shapeIndex = 0;

    for (const auto& range : submeshRanges) {
        uint32_t submeshStartIndex = static_cast<uint32_t>(outIndices.size());

        for (uint32_t i = range.first; i < range.first + range.second; ++i) {
            const Vertex& vertex = rawVertices[rawIndices[i]];

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
                outVertices.push_back(vertex);
            }

            outIndices.push_back(uniqueVertices[vertex]);
        }

        uint32_t submeshIndexCount = static_cast<uint32_t>(outIndices.size()) - submeshStartIndex;

        int materialId = 0;
        if (shapeIndex < shapes.size() && !shapes[shapeIndex].mesh.material_ids.empty()) {
            materialId = shapes[shapeIndex].mesh.material_ids[0];
        }

        submeshes.emplace_back(submeshStartIndex, submeshIndexCount, materialId);
        shapeIndex++;
    }

    if (submeshes.empty()) {
        throw std::runtime_error("No geometry found in OBJ file: " + filepath);
    }
}

void Mesh::destroy(VmaAllocator allocator) {
    vertexBuffer.destroy(allocator);
    indexBuffer.destroy(allocator);
    submeshes.clear();
    materialNames.clear();
    vertices.clear();
    indices.clear();
    totalIndexCount = 0;
}

void Mesh::bind(VkCommandBuffer commandBuffer) const {
    VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::draw(VkCommandBuffer commandBuffer, uint32_t submeshIndex) const {
    if (submeshIndex >= submeshes.size()) {
        throw std::runtime_error("Invalid submesh index");
    }

    const SubMesh& submesh = submeshes[submeshIndex];
    vkCmdDrawIndexed(commandBuffer, submesh.indexCount, 1, submesh.indexOffset, 0, 0);
}

void Mesh::drawAll(VkCommandBuffer commandBuffer) const {
    for (uint32_t i = 0; i < submeshes.size(); ++i) {
        draw(commandBuffer, i);
    }
}

const std::string& Mesh::getMaterialName(uint32_t submeshIndex) const {
    if (submeshIndex >= materialNames.size()) {
        throw std::runtime_error("Invalid submesh index for getMaterialName");
    }
    return materialNames[submeshIndex];
}