#include "mesh.hpp"
#include "commandbuffer.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::Mesh() : totalIndexCount(0) {
}

Mesh::~Mesh() {
}

void Mesh::loadFromFile(const std::string& filepath, VmaAllocator allocator, CommandBuffer* commandBuffer) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

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

    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    uint32_t currentIndexOffset = 0;

    for (const auto& shape : shapes) {
        uint32_t submeshStartIndex = static_cast<uint32_t>(outIndices.size());
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

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
                    outVertices.push_back(vertex);
                }

                outIndices.push_back(uniqueVertices[vertex]);
            }

            index_offset += fv;
        }

        uint32_t submeshIndexCount = static_cast<uint32_t>(outIndices.size()) - submeshStartIndex;

        int materialId = 0;
        if (!shape.mesh.material_ids.empty()) {
            materialId = shape.mesh.material_ids[0];
        }

        submeshes.emplace_back(submeshStartIndex, submeshIndexCount, materialId);

        std::string materialName = (materialId >= 0 && materialId < materials.size()) ? materials[materialId].name : "";
        materialNames.push_back(materialName);
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