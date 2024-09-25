#include "mesher.h"

#include "visual_nurb.h"

#undef min
#undef max
#include "CDT.h"

namespace VN
{
    mesh_uv_coordiante mesher::uv_mesh(VsNurbSurf* srf)
    {

        CDT::Triangulation<double> cdt;
        mesh_uv_coordiante result;

        if (srf->num_loop)
        {
            std::vector<CDT::V2d<double>> vtx_uv = {

            };
        }
        else
        {
            std::vector<CDT::V2d<double>> vtx_uv = {
                {0, 0},
                {1, 0},
                {1, 1},
                {0, 1}
            };

            std::vector<CDT::Edge> edg_uv = {
                {0, 1},
                {1, 2},
                {2, 3},
                {3, 0}
            };

            cdt.insertVertices(vtx_uv);
            cdt.insertEdges(edg_uv);

            for (auto vtx : cdt.vertices)
            {
                result.nodes.emplace_back(VecImpl_t<double, 2>(vtx.x, vtx.y));
            }
            for (auto tri : cdt.triangles)
            {
                result.meshes.emplace_back(std::vector<unsigned int>());
                for (int i = 0; i < 3; i++)
                {
                    result.meshes.back().emplace_back(tri.vertices.at(i));
                }
            }
        }

        return result;
    }

    mesh_global_coordinate mesher::gl_mesh(VsNurbSurf* srf, const mesh_uv_coordiante& uv)
    {
        // TODO:
        return {};
    }

}