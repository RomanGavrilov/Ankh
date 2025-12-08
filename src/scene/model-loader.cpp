#include "scene/model-loader.hpp"

#include "scene/material.hpp"
#include "scene/mesh.hpp"
#include "utils/logging.hpp"
#include "utils/types.hpp"

#include <tiny_gltf.h>

namespace ankh
{
    namespace
    {
        const unsigned char *get_buffer_data(const tinygltf::Model &model,
                                             const tinygltf::Accessor &accessor)
        {
            const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[view.buffer];
            return buffer.data.data() + view.byteOffset + accessor.byteOffset;
        }

        glm::mat4 node_transform(const tinygltf::Node &node)
        {
            // If full 4x4 matrix present, use it
            if (!node.matrix.empty())
            {
                glm::mat4 mat(1.0f);
                for (int i = 0; i < 16; ++i)
                {
                    mat[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
                }
                return mat;
            }

            glm::mat4 M(1.0f);

            // translation
            if (!node.translation.empty())
            {
                glm::vec3 t(static_cast<float>(node.translation[0]),
                            static_cast<float>(node.translation[1]),
                            static_cast<float>(node.translation[2]));
                M = glm::translate(M, t);
            }

            // (ignore rotation for now)

            // scale
            if (!node.scale.empty())
            {
                glm::vec3 s(static_cast<float>(node.scale[0]),
                            static_cast<float>(node.scale[1]),
                            static_cast<float>(node.scale[2]));
                M = glm::scale(M, s);
            }

            return M;
        }
    } // namespace

    Model ModelLoader::load_gltf(const std::string &path,
                                 MeshPool &mesh_pool,
                                 MaterialPool &material_pool)
    {
        ANKH_LOG_INFO("ModelLoader::load_gltf(\"" + path + "\")");

        tinygltf::Model gltf;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ok = false;
        if (path.size() >= 5 &&
            (path.substr(path.size() - 5) == ".gltf" || path.substr(path.size() - 5) == ".GLTF"))
        {
            ok = loader.LoadASCIIFromFile(&gltf, &err, &warn, path);
        }
        else
        {
            ok = loader.LoadBinaryFromFile(&gltf, &err, &warn, path);
        }

        if (!warn.empty())
        {
            ANKH_LOG_WARN("TinyGLTF warning: " + warn);
        }
        if (!ok)
        {
            ANKH_LOG_ERROR("Failed to load glTF: " + path + " error: " + err);
            return Model(path);
        }

        Model model(path);

        // 1) Materials → MaterialPool
        std::vector<MaterialHandle> material_handles;
        material_handles.reserve(gltf.materials.size());

        for (const auto &gm : gltf.materials)
        {
            glm::vec4 baseColor(1.0f);
            auto it = gm.values.find("baseColorFactor");
            if (it != gm.values.end() && it->second.number_array.size() >= 3)
            {
                const auto &arr = it->second.number_array;
                baseColor = glm::vec4(static_cast<float>(arr[0]),
                                      static_cast<float>(arr[1]),
                                      static_cast<float>(arr[2]),
                                      arr.size() > 3 ? static_cast<float>(arr[3]) : 1.0f);
            }

            Material mat(baseColor);
            MaterialHandle mh = material_pool.create(mat);
            material_handles.push_back(mh);
        }

        // 2) Nodes / meshes / primitives → MeshPool + ModelNode
        for (size_t node_idx = 0; node_idx < gltf.nodes.size(); ++node_idx)
        {
            const auto &node = gltf.nodes[node_idx];
            
            if (node.mesh < 0 || node.mesh >= static_cast<int>(gltf.meshes.size()))
            {
                continue;
            }

            const auto &gltf_mesh = gltf.meshes[node.mesh];
            glm::mat4 local = node_transform(node);

            for (const auto &prim : gltf_mesh.primitives)
            {
                // POSITION (required)
                auto pos_it = prim.attributes.find("POSITION");
                if (pos_it == prim.attributes.end())
                {
                    ANKH_LOG_WARN("Primitive missing POSITION, skipping");
                    continue;
                }

                const tinygltf::Accessor &pos_acc = gltf.accessors[pos_it->second];
                if (pos_acc.type != TINYGLTF_TYPE_VEC3 ||
                    pos_acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    ANKH_LOG_WARN("Unsupported POSITION format, skipping primitive");
                    continue;
                }

                const unsigned char *pos_data = get_buffer_data(gltf, pos_acc);
                size_t vertex_count = pos_acc.count;

                // COLOR_0 (optional)
                const tinygltf::Accessor *color_acc = nullptr;
                const unsigned char *color_data = nullptr;
                auto color_it = prim.attributes.find("COLOR_0");
                if (color_it != prim.attributes.end())
                {
                    color_acc = &gltf.accessors[color_it->second];
                    color_data = get_buffer_data(gltf, *color_acc);
                }

                // TEXCOORD_0 (optional)
                const tinygltf::Accessor *uv_acc = nullptr;
                const unsigned char *uv_data = nullptr;
                auto uv_it = prim.attributes.find("TEXCOORD_0");
                if (uv_it != prim.attributes.end())
                {
                    uv_acc = &gltf.accessors[uv_it->second];
                    uv_data = get_buffer_data(gltf, *uv_acc);
                }

                std::vector<Vertex> vertices;
                vertices.reserve(vertex_count);

                for (size_t v = 0; v < vertex_count; ++v)
                {
                    const float *p =
                        reinterpret_cast<const float *>(pos_data + v * sizeof(float) * 3);

                    Vertex vert{};
                    vert.pos = glm::vec3(p[0], p[1], p[2]);

                    glm::vec3 color(1.0f);
                    if (color_acc && color_data)
                    {
                        if (color_acc->type == TINYGLTF_TYPE_VEC3)
                        {
                            const float *c =
                                reinterpret_cast<const float *>(color_data + v * sizeof(float) * 3);
                            color = glm::vec3(c[0], c[1], c[2]);
                        }
                        else if (color_acc->type == TINYGLTF_TYPE_VEC4)
                        {
                            const float *c =
                                reinterpret_cast<const float *>(color_data + v * sizeof(float) * 4);
                            color = glm::vec3(c[0], c[1], c[2]);
                        }
                    }
                    vert.color = color;

                    glm::vec2 uv(0.0f);
                    if (uv_acc && uv_data)
                    {
                        const float *u =
                            reinterpret_cast<const float *>(uv_data + v * sizeof(float) * 2);
                        uv = glm::vec2(u[0], u[1]);
                    }
                    vert.uv = uv;

                    vertices.push_back(vert);
                }

                // indices
                std::vector<uint16_t> indices;
                if (prim.indices >= 0)
                {
                    const tinygltf::Accessor &idx_acc = gltf.accessors[prim.indices];
                    const unsigned char *idx_data = get_buffer_data(gltf, idx_acc);

                    indices.reserve(idx_acc.count);
                    for (size_t i = 0; i < idx_acc.count; ++i)
                    {
                        uint32_t value = 0;
                        switch (idx_acc.componentType)
                        {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            value = reinterpret_cast<const uint16_t *>(idx_data)[i];
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            value = reinterpret_cast<const uint32_t *>(idx_data)[i];
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            value = reinterpret_cast<const uint8_t *>(idx_data)[i];
                            break;
                        default:
                            ANKH_LOG_WARN("Unsupported index type, skipping indices");
                            break;
                        }
                        indices.push_back(static_cast<uint16_t>(value));
                    }
                }
                else
                {
                    indices.resize(vertex_count);
                    for (uint32_t i = 0; i < static_cast<uint32_t>(vertex_count); ++i)
                        indices[i] = i;
                }

                Mesh mesh(std::move(vertices), std::move(indices));
                MeshHandle mesh_handle = mesh_pool.create(std::move(mesh));

                MaterialHandle mat_handle = INVALID_MATERIAL_HANDLE;
                if (prim.material >= 0 && prim.material < static_cast<int>(material_handles.size()))
                {
                    mat_handle = material_handles[prim.material];
                }

                ModelNode node_entry{};
                node_entry.mesh = mesh_handle;
                node_entry.material = mat_handle;
                node_entry.local_transform = local;

                model.nodes().push_back(node_entry);
            }
        }

        ANKH_LOG_INFO("ModelLoader::load_gltf loaded " + std::to_string(model.nodes().size()) +
                      " nodes.");

        return model;
    }

} // namespace ankh
