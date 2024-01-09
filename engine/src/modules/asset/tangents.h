#pragma once
#include "common.h"
#include "modules/render/render_module.h"

#include "mikkt/mikktspace.h"

#include <string>

namespace mas::details
{
enum class FaceType
{
    Triangles = 0,
};

struct IntermediateMeshData
{
    std::vector<gfx::VertexP3N3U2T4> vertices;
    std::vector<gfx::VertexIndexType> indices;
    std::string path;
    FaceType type;
};

class TangentCalculator
{
public:
    TangentCalculator();
    void calculate(IntermediateMeshData* mesh);

private:
    SMikkTSpaceInterface inter_face{};
    SMikkTSpaceContext context{};

    static i32 get_vertex_index(const SMikkTSpaceContext* context, i32 i_face, i32 i_vert);

    static i32 get_num_faces(const SMikkTSpaceContext* context);
    static i32 get_num_vertices_of_face(const SMikkTSpaceContext* context, i32 i_face);
    static void get_position(const SMikkTSpaceContext* context, f32 out_pos[],
                             i32 i_face, i32 i_vert);

    static void get_normal(const SMikkTSpaceContext* context, f32 out_normal[],
                           i32 i_face, i32 i_vert);

    static void get_tex_coords(const SMikkTSpaceContext* context, f32 out_uv[],
                               i32 i_face, i32 i_vert);

    static void set_tspace_basic(const SMikkTSpaceContext* context,
                                 const f32 tangents[],
                                 f32 f_sign, i32 i_face, i32 i_vert);
};
}
