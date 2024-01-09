#include "tangents.h"

#include <iostream>

#include "glm/gtx/string_cast.hpp"
#include "spdlog/spdlog.h"


namespace mas::details
{
namespace
{
constexpr bool debug_tangents{ false };
}

TangentCalculator::TangentCalculator()
{
    inter_face.m_getNumFaces = get_num_faces;
    inter_face.m_getNumVerticesOfFace = get_num_vertices_of_face;

    inter_face.m_getNormal = get_normal;
    inter_face.m_getPosition = get_position;
    inter_face.m_getTexCoord = get_tex_coords;
    inter_face.m_setTSpaceBasic = set_tspace_basic;

    context.m_pInterface = &inter_face;
}

void TangentCalculator::calculate(IntermediateMeshData* mesh)
{
    context.m_pUserData = mesh;

    if constexpr (debug_tangents)
        spdlog::debug("Calculating tangents for: {}", mesh->path);

    genTangSpaceDefault(&context);
}

i32 TangentCalculator::get_vertex_index(const SMikkTSpaceContext* context, const i32 i_face, const i32 i_vert)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const auto face_size = get_num_vertices_of_face(context, i_face);
    const auto indices_index = (i_face * face_size) + i_vert;

    assert(working_mesh->indices[indices_index] < static_cast<gfx::VertexIndexType>(INT32_MAX));

    const i32 index = static_cast<i32>(working_mesh->indices[indices_index]);
    return index;
}

i32 TangentCalculator::get_num_faces(const SMikkTSpaceContext* context)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const i32 i_size = static_cast<i32>(working_mesh->indices.size()) / 3;
    if constexpr (debug_tangents)
    {
        const f32 f_size = static_cast<f32>(working_mesh->indices.size()) / 3.0f;

        if (f_size - static_cast<f32>(i_size) != 0.0f)
        {
            spdlog::error("Indices is not divisible by 3");
            throw std::runtime_error("f_size and i_size differs");
        }
    }

    return i_size;
}

i32 TangentCalculator::get_num_vertices_of_face(const SMikkTSpaceContext* context, i32 i_face)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);
    if (working_mesh->type != FaceType::Triangles)
    {
        spdlog::error("Engine only supports triangle faces for primitives");
        throw std::runtime_error("Mastodon only supports primitive of triangles!");
    }

    return 3;
}

void TangentCalculator::get_position(const SMikkTSpaceContext* context, f32 out_pos[], const i32 i_face, const i32 i_vert)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const auto index = get_vertex_index(context, i_face, i_vert);
    const auto pos = working_mesh->vertices[index].pos;

    if constexpr (debug_tangents)
        spdlog::debug("[Tangent calculator] get_position(): Index: {0} Position: {1}", index, glm::to_string(pos));

    out_pos[0] = pos.x;
    out_pos[1] = pos.y;
    out_pos[2] = pos.z;
}

void TangentCalculator::get_normal(const SMikkTSpaceContext* context, f32 out_normal[], const i32 i_face, const i32 i_vert)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const auto index = get_vertex_index(context, i_face, i_vert);
    const auto normal = working_mesh->vertices[index].normal;

    if constexpr (debug_tangents)
        spdlog::debug("[Tangent calculator] get_normal(): Index: {0} Normal: {1}", index, glm::to_string(normal));

    out_normal[0] = normal.x;
    out_normal[1] = normal.y;
    out_normal[2] = normal.z;
}

void TangentCalculator::get_tex_coords(const SMikkTSpaceContext* context, f32 out_uv[], const i32 i_face, const i32 i_vert)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const auto index = get_vertex_index(context, i_face, i_vert);
    const auto uv = working_mesh->vertices[index].uv;

    if constexpr (debug_tangents)
        spdlog::debug("[Tangent calculator] get_tex_coords(): Index: {0} TexCoord: {1}", index, glm::to_string(uv));

    out_uv[0] = uv.x;
    out_uv[1] = uv.y;
}

void TangentCalculator::set_tspace_basic(const SMikkTSpaceContext* context, const f32 tangents[], const f32 f_sign, const i32 i_face, const i32 i_vert)
{
    const auto working_mesh = static_cast<IntermediateMeshData*>(context->m_pUserData);

    const auto index = get_vertex_index(context, i_face, i_vert);
    auto& tangent = working_mesh->vertices[index].tangent;

    tangent.x = tangents[0];
    tangent.y = tangents[1];
    tangent.z = tangents[2];
    tangent.w = f_sign;

    if constexpr (debug_tangents)
        spdlog::debug("[Tangent calculator] set_tspace_basic(): Index: {0} Tangent: {1}", index, glm::to_string(tangent));
}
}
