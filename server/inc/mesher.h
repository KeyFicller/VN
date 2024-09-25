#pragma once

#include "vector_matrix.h"

namespace VN
{
    class VsNurbSurf;

    template <unsigned DIM = 2>
    struct mesh
    {
        VecImpl_t<double, DIM> nodes;
        std::vector<std::vector<int>> meshes;
    };

    using mesh_uv_coordiante = mesh<2>;
    using mesh_global_coordinate = mesh<3>;

    class mesher
    {
    public:
        static mesh_uv_coordiante uv_mesh(VsNurbSurf* srf);
        static mesh_global_coordinate gl_mesh(VsNurbSurf* srf, const mesh_uv_coordiante& uv);
    };
}