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
            
        }
        else
        {
            std::vector<CDT::V2d<double>> vtx_uv = {
                {0, 0},
                {1, 0},
                {1, 1},
                {0, 1},
                {0.25, 0.25},
                {0.75, 0.25},
                {0.75, 0.75},
                {0.25, 0.75},
                {0.4, 0.4},
                {0.6, 0.4},
                {0.6, 0.6},
                {0.4, 0.6}
            };

            std::vector<CDT::Edge> edg_uv = {
                {0, 1},
                {1, 2},
                {2, 3},
                {3, 0},
                {4, 5},
                {5, 6},
                {6, 7},
                {7, 4},
                {8, 9},
                {9, 10},
                {10, 11},
                {11, 8}
            };

            cdt.insertVertices(vtx_uv);
            cdt.insertEdges(edg_uv);

            cdt.eraseOuterTrianglesAndHoles();

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