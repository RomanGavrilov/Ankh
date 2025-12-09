#pragma once

#include "material-pool.hpp"
#include "mesh-pool.hpp"
#include "model.hpp"
#include <string>

namespace ankh
{
    class ModelLoader
    {
      public:
        // Loads .gltf or .glb, creates Mesh/Material in pools,
        // returns a Model whose nodes reference them via handles.
        static Model
        load_gltf(const std::string &path, MeshPool &mesh_pool, MaterialPool &material_pool);
    };

} // namespace ankh
