// src/scene/model-loader.cpp

#include "scene/model-loader.hpp"

#include "scene/material.hpp"
#include "scene/mesh.hpp"
#include "utils/logging.hpp"
#include "utils/types.hpp"

#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <tiny-gltf.h>

namespace ankh
{
    namespace
    {
        // -----------------------------
        // Node transform helpers
        // -----------------------------

        glm::mat4 node_local_transform(const tinygltf::Node &node)
        {
            // 1) Full 4x4 matrix (highest priority)
            if (!node.matrix.empty() && node.matrix.size() == 16)
            {
                glm::mat4 M(1.0f);
                for (int i = 0; i < 16; ++i)
                {
                    M[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
                }
                return M;
            }

            glm::mat4 M(1.0f);

            // 2) Translation
            if (node.translation.size() == 3)
            {
                glm::vec3 t(static_cast<float>(node.translation[0]),
                            static_cast<float>(node.translation[1]),
                            static_cast<float>(node.translation[2]));
                M = glm::translate(M, t);
            }

            // 3) Rotation (quaternion, x,y,z,w)
            if (node.rotation.size() == 4)
            {
                glm::quat q(static_cast<float>(node.rotation[3]), // w
                            static_cast<float>(node.rotation[0]),
                            static_cast<float>(node.rotation[1]),
                            static_cast<float>(node.rotation[2]));
                M *= glm::mat4_cast(q);
            }

            // 4) Scale
            if (node.scale.size() == 3)
            {
                glm::vec3 s(static_cast<float>(node.scale[0]),
                            static_cast<float>(node.scale[1]),
                            static_cast<float>(node.scale[2]));
                M = glm::scale(M, s);
            }

            return M;
        }

        // -----------------------------
        // Attribute view helper
        // -----------------------------

        struct FloatAttributeView
        {
            const unsigned char *base{nullptr};
            size_t stride{0};
            size_t count{0};
        };

        FloatAttributeView make_float_view(const tinygltf::Model &model,
                                           const tinygltf::Accessor &accessor,
                                           size_t expectedComponents,
                                           const char *debugName)
        {
            FloatAttributeView view{};

            if (accessor.bufferView < 0 ||
                accessor.bufferView >= static_cast<int>(model.bufferViews.size()))
            {
                ANKH_LOG_WARN(std::string("Accessor for ") + debugName +
                              " has invalid bufferView; skipping attribute");
                return view;
            }

            const tinygltf::BufferView &bufView = model.bufferViews[accessor.bufferView];

            if (bufView.buffer < 0 || bufView.buffer >= static_cast<int>(model.buffers.size()))
            {
                ANKH_LOG_WARN(std::string("BufferView for ") + debugName +
                              " has invalid buffer; skipping attribute");
                return view;
            }

            const tinygltf::Buffer &buffer = model.buffers[bufView.buffer];

            const unsigned char *data =
                buffer.data.data() + bufView.byteOffset + accessor.byteOffset;

            size_t stride = accessor.ByteStride(bufView);
            if (stride == 0)
            {
                stride = sizeof(float) * expectedComponents;
            }

            view.base = data;
            view.stride = stride;
            view.count = accessor.count;
            return view;
        }

        // -----------------------------
        // Material helpers
        // -----------------------------

        glm::vec4 load_base_color_factor(const tinygltf::Material &gm)
        {
            glm::vec4 baseColor(1.0f);

            const auto &pbr = gm.pbrMetallicRoughness;
            if (!pbr.baseColorFactor.empty() && pbr.baseColorFactor.size() >= 3)
            {
                baseColor.r = static_cast<float>(pbr.baseColorFactor[0]);
                baseColor.g = static_cast<float>(pbr.baseColorFactor[1]);
                baseColor.b = static_cast<float>(pbr.baseColorFactor[2]);
                baseColor.a = (pbr.baseColorFactor.size() > 3)
                                  ? static_cast<float>(pbr.baseColorFactor[3])
                                  : 1.0f;
            }

            return baseColor;
        }

        std::shared_ptr<CpuImage> load_base_color_image(const tinygltf::Model &gltf,
                                                        const tinygltf::Material &gm)
        {
            const auto &pbr = gm.pbrMetallicRoughness;

            if (pbr.baseColorTexture.index < 0 ||
                pbr.baseColorTexture.index >= static_cast<int>(gltf.textures.size()))
            {
                return nullptr;
            }

            const tinygltf::Texture &tex = gltf.textures[pbr.baseColorTexture.index];

            if (tex.source < 0 || tex.source >= static_cast<int>(gltf.images.size()))
            {
                ANKH_LOG_WARN("Texture source index out of range for baseColorTexture; ignoring");
                return nullptr;
            }

            const tinygltf::Image &img = gltf.images[tex.source];

            if (img.width <= 0 || img.height <= 0 || img.image.empty())
            {
                ANKH_LOG_WARN("baseColorTexture image has no data; ignoring");
                return nullptr;
            }

            auto cpuImg = std::make_shared<CpuImage>();
            cpuImg->width = img.width;
            cpuImg->height = img.height;
            cpuImg->components = img.component;
            cpuImg->pixels = img.image; // copy byte data

            return cpuImg;
        }

        // -----------------------------
        // Mesh / primitive builder
        // -----------------------------

        Mesh build_mesh_from_primitive(const tinygltf::Model &gltf, const tinygltf::Primitive &prim)
        {
            // --- POSITION (required) ---
            auto posIt = prim.attributes.find("POSITION");
            if (posIt == prim.attributes.end())
            {
                ANKH_THROW_MSG("Primitive missing POSITION");
            }

            const tinygltf::Accessor &posAcc = gltf.accessors[posIt->second];
            if (posAcc.type != TINYGLTF_TYPE_VEC3 ||
                posAcc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                ANKH_THROW_MSG("Unsupported POSITION format (only FLOAT VEC3 supported)");
            }

            FloatAttributeView posView = make_float_view(gltf, posAcc, 3, "POSITION");
            if (!posView.base)
            {
                ANKH_THROW_MSG("Failed to create POSITION view");
            }

            size_t vertexCount = posView.count;

            // --- NORMAL (optional, float VEC3) ---
            const tinygltf::Accessor *normalAcc = nullptr;
            FloatAttributeView normalView{};
            auto normIt = prim.attributes.find("NORMAL");
            if (normIt != prim.attributes.end())
            {
                normalAcc = &gltf.accessors[normIt->second];

                if (normalAcc->type == TINYGLTF_TYPE_VEC3 &&
                    normalAcc->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    normalView = make_float_view(gltf, *normalAcc, 3, "NORMAL");
                }
                else
                {
                    ANKH_LOG_WARN("NORMAL has unsupported format; using default (0,0,1)");
                    normalAcc = nullptr;
                }
            }

            // --- COLOR_0 (optional, float VEC3/VEC4) ---
            const tinygltf::Accessor *colorAcc = nullptr;
            FloatAttributeView colorView{};
            auto colorIt = prim.attributes.find("COLOR_0");
            if (colorIt != prim.attributes.end())
            {
                colorAcc = &gltf.accessors[colorIt->second];

                if ((colorAcc->type == TINYGLTF_TYPE_VEC3 ||
                     colorAcc->type == TINYGLTF_TYPE_VEC4) &&
                    colorAcc->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    colorView = make_float_view(gltf,
                                                *colorAcc,
                                                (colorAcc->type == TINYGLTF_TYPE_VEC3) ? 3 : 4,
                                                "COLOR_0");
                }
                else
                {
                    ANKH_LOG_WARN("COLOR_0 has unsupported format; using white");
                    colorAcc = nullptr;
                }
            }

            // --- TEXCOORD_0 (optional, float VEC2) ---
            const tinygltf::Accessor *uvAcc = nullptr;
            FloatAttributeView uvView{};
            auto uvIt = prim.attributes.find("TEXCOORD_0");
            if (uvIt != prim.attributes.end())
            {
                uvAcc = &gltf.accessors[uvIt->second];

                if (uvAcc->type == TINYGLTF_TYPE_VEC2 &&
                    uvAcc->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    uvView = make_float_view(gltf, *uvAcc, 2, "TEXCOORD_0");
                }
                else
                {
                    ANKH_LOG_WARN("TEXCOORD_0 has unsupported format; using (0,0)");
                    uvAcc = nullptr;
                }
            }

            // Build vertex array
            std::vector<Vertex> vertices;
            vertices.resize(vertexCount);

            for (size_t v = 0; v < vertexCount; ++v)
            {
                Vertex vert{};

                // Position
                const float *p = reinterpret_cast<const float *>(posView.base + v * posView.stride);
                vert.pos = glm::vec3(p[0], p[1], p[2]);

                glm::vec3 normal(0.0f, 0.0f, 1.0f);
                if (normalAcc && normalView.base && v < normalView.count)
                {
                    const float *n =
                        reinterpret_cast<const float *>(normalView.base + v * normalView.stride);
                    normal = glm::vec3(n[0], n[1], n[2]);
                }
                vert.normal = normal;

                // Color
                glm::vec3 color(1.0f);
                if (colorAcc && colorView.base && v < colorView.count)
                {
                    const float *c =
                        reinterpret_cast<const float *>(colorView.base + v * colorView.stride);

                    if (colorAcc->type == TINYGLTF_TYPE_VEC3)
                    {
                        color = glm::vec3(c[0], c[1], c[2]);
                    }
                    else // VEC4
                    {
                        color = glm::vec3(c[0], c[1], c[2]);
                    }
                }
                vert.color = color;

                // UV
                glm::vec2 uv(0.0f);
                if (uvAcc && uvView.base && v < uvView.count)
                {
                    const float *u =
                        reinterpret_cast<const float *>(uvView.base + v * uvView.stride);
                    uv = glm::vec2(u[0], u[1]);
                }
                vert.uv = uv;

                vertices[v] = vert;
            }

            // Indices
            std::vector<uint16_t> indices;

            if (prim.indices >= 0)
            {
                const tinygltf::Accessor &idxAcc = gltf.accessors[prim.indices];

                if (idxAcc.bufferView < 0 ||
                    idxAcc.bufferView >= static_cast<int>(gltf.bufferViews.size()))
                {
                    ANKH_THROW_MSG("Index accessor has invalid bufferView");
                }

                const tinygltf::BufferView &idxView = gltf.bufferViews[idxAcc.bufferView];
                if (idxView.buffer < 0 || idxView.buffer >= static_cast<int>(gltf.buffers.size()))
                {
                    ANKH_THROW_MSG("Index bufferView has invalid buffer");
                }

                const tinygltf::Buffer &idxBuffer = gltf.buffers[idxView.buffer];

                const unsigned char *idxBase =
                    idxBuffer.data.data() + idxView.byteOffset + idxAcc.byteOffset;

                indices.reserve(idxAcc.count);

                for (size_t i = 0; i < idxAcc.count; ++i)
                {
                    uint32_t value = 0;

                    switch (idxAcc.componentType)
                    {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        value = reinterpret_cast<const uint8_t *>(idxBase)[i];
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        value = reinterpret_cast<const uint16_t *>(idxBase)[i];
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        value = reinterpret_cast<const uint32_t *>(idxBase)[i];
                        break;
                    default:
                        ANKH_LOG_WARN("Unsupported index component type; skipping primitive");
                        indices.clear();
                        i = idxAcc.count;
                        break;
                    }

                    if (value > std::numeric_limits<uint16_t>::max())
                    {
                        ANKH_THROW_MSG("Index value exceeds uint16 range; upgrade Mesh "
                                       "to uint32 to support this model");
                    }

                    indices.push_back(static_cast<uint16_t>(value));
                }
            }
            else
            {
                indices.resize(vertexCount);
                for (uint32_t i = 0; i < static_cast<uint32_t>(vertexCount); ++i)
                {
                    indices[i] = i;
                }
            }

            return Mesh(std::move(vertices), std::move(indices));
        }

    } // namespace

    // -----------------------------
    // Public API
    // -----------------------------

    Model ModelLoader::load_gltf(const std::string &path,
                                 MeshPool &mesh_pool,
                                 MaterialPool &material_pool)
    {
        ANKH_LOG_DEBUG("[ModelLoader] LoadGltf(\"" + path + "\")");

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
            ANKH_LOG_WARN("[ModelLoader] TinyGLTF warning: " + warn);
        }
        if (!ok)
        {
            ANKH_LOG_ERROR("[ModelLoader] Failed to load glTF: " + path + " error: " + err);
            return Model(path);
        }

        Model model(path);

        // --- Materials ---
        std::vector<MaterialHandle> material_handles;
        material_handles.reserve(gltf.materials.size());

        for (const auto &gm : gltf.materials)
        {
            glm::vec4 baseColor = load_base_color_factor(gm);
            Material mat(baseColor);

            auto cpuImg = load_base_color_image(gltf, gm);
            if (cpuImg)
            {
                mat.set_base_color_image(cpuImg);
            }
            else
            {
                ANKH_LOG_DEBUG("[ModelLoader] Material has NO baseColorTexture");
            }

            MaterialHandle mh = material_pool.create(mat);
            material_handles.push_back(mh);
        }

        // --- Scene traversal ---
        int sceneIndex = gltf.defaultScene >= 0 ? gltf.defaultScene : 0;

        auto processNode = [&](auto &&self, int nodeIndex, const glm::mat4 &parentTransform) -> void
        {
            if (nodeIndex < 0 || nodeIndex >= static_cast<int>(gltf.nodes.size()))
            {
                return;
            }

            const tinygltf::Node &node = gltf.nodes[nodeIndex];

            glm::mat4 local = node_local_transform(node);
            glm::mat4 world = parentTransform * local;

            // Mesh + primitives
            if (node.mesh >= 0 && node.mesh < static_cast<int>(gltf.meshes.size()))
            {
                const tinygltf::Mesh &gltfMesh = gltf.meshes[node.mesh];

                for (const auto &prim : gltfMesh.primitives)
                {
                    try
                    {
                        Mesh mesh = build_mesh_from_primitive(gltf, prim);
                        MeshHandle mesh_handle = mesh_pool.create(std::move(mesh));

                        MaterialHandle mat_handle = INVALID_MATERIAL_HANDLE;
                        if (prim.material >= 0 &&
                            prim.material < static_cast<int>(material_handles.size()))
                        {
                            mat_handle = material_handles[prim.material];
                        }

                        ModelNode nodeEntry{};
                        nodeEntry.mesh = mesh_handle;
                        nodeEntry.material = mat_handle;
                        nodeEntry.local_transform = world; // baked world transform

                        model.nodes().push_back(nodeEntry);
                    }
                    catch (const std::exception &e)
                    {
                        ANKH_LOG_WARN("[ModelLoader] Skipping primitive: " + std::string(e.what()));
                    }
                }
            }

            // Children
            for (int childIndex : node.children)
            {
                self(self, childIndex, world);
            }
        };

        glm::mat4 identity(1.0f);

        if (sceneIndex >= 0 && sceneIndex < static_cast<int>(gltf.scenes.size()))
        {
            const tinygltf::Scene &scene = gltf.scenes[sceneIndex];
            for (int nodeIndex : scene.nodes)
            {
                processNode(processNode, nodeIndex, identity);
            }
        }
        else
        {
            ANKH_LOG_WARN("[ModelLoader] glTF has no valid scenes; traversing all nodes as roots");
            for (int nodeIndex = 0; nodeIndex < static_cast<int>(gltf.nodes.size()); ++nodeIndex)
            {
                processNode(processNode, nodeIndex, identity);
            }
        }

        ANKH_LOG_DEBUG("[ModelLoader] LoadGltf loaded " + std::to_string(model.nodes().size()) +
                       " model nodes.");

        return model;
    }

} // namespace ankh
