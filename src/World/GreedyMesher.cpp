#include "GreedyMesher.hpp"

#include "AtlasUV.hpp"
#include "BlockRegistry.hpp"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>

namespace world
{
namespace
{
constexpr int UAxis[3] = {2, 0, 0};
constexpr int VAxis[3] = {1, 2, 1};

struct MaskCell
{
    BlockID block = BlockAir;
    bool filled = false;
};

std::uint32_t pack_normal(const glm::vec3& n)
{
    const auto encode = [](float value) {
        const float scaled = std::clamp((value * 0.5f + 0.5f) * 1023.0f, 0.0f, 1023.0f);
        return static_cast<std::uint32_t>(scaled);
    };
    const std::uint32_t x = encode(n.x);
    const std::uint32_t y = encode(n.y);
    const std::uint32_t z = encode(n.z);
    return (x & 0x3FFu) | ((y & 0x3FFu) << 10) | ((z & 0x3FFu) << 20);
}

BlockID sample_block(const Chunk& chunk, const NeighborSet& neighbors, int x, int y, int z)
{
    if (y < 0 || y >= ChunkHeight)
        return BlockAir;

    if (x >= 0 && x < ChunkWidth && z >= 0 && z < ChunkDepth)
    {
        return chunk.get(x, y, z);
    }

    if (x < 0)
    {
        if (!neighbors.negX)
            return BlockAir;
        return neighbors.negX->get(x + ChunkWidth, y, z);
    }
    if (x >= ChunkWidth)
    {
        if (!neighbors.posX)
            return BlockAir;
        return neighbors.posX->get(x - ChunkWidth, y, z);
    }

    if (z < 0)
    {
        if (!neighbors.negZ)
            return BlockAir;
        return neighbors.negZ->get(x, y, z + ChunkDepth);
    }
    if (z >= ChunkDepth)
    {
        if (!neighbors.posZ)
            return BlockAir;
        return neighbors.posZ->get(x, y, z - ChunkDepth);
    }

    return BlockAir;
}

bool is_opaque(BlockID id)
{
    if (id == BlockAir)
        return false;
    return has_flag(registry().flags(id), BlockFlags::Opaque) && !has_flag(registry().flags(id), BlockFlags::Transparent);
}

bool is_transparent(BlockID id)
{
    if (id == BlockAir)
        return false;
    return has_flag(registry().flags(id), BlockFlags::Transparent) && !has_flag(registry().flags(id), BlockFlags::Fluid);
}

bool is_fluid(BlockID id)
{
    if (id == BlockAir)
        return false;
    return has_flag(registry().flags(id), BlockFlags::Fluid);
}

BlockFace axis_face(int axis, bool positive)
{
    switch (axis)
    {
    case 0: return positive ? BlockFace::PosX : BlockFace::NegX;
    case 1: return positive ? BlockFace::PosY : BlockFace::NegY;
    default: return positive ? BlockFace::PosZ : BlockFace::NegZ;
    }
}

void emit_quad(std::vector<renderer::ChunkVertex>& vertices,
               std::vector<std::uint32_t>& indices,
               const glm::vec3& origin,
               const glm::vec3& du,
               const glm::vec3& dv,
               const glm::vec3& normal,
               const AtlasUV& uv,
               int w,
               int h,
               bool flip)
{
    const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
    renderer::ChunkVertex v0{};
    renderer::ChunkVertex v1{};
    renderer::ChunkVertex v2{};
    renderer::ChunkVertex v3{};

    auto write_vertex = [](renderer::ChunkVertex& v, const glm::vec3& pos, const glm::vec2& uv, std::uint32_t packedNormal) {
        v.position[0] = pos.x;
        v.position[1] = pos.y;
        v.position[2] = pos.z;
        v.normalPacked = packedNormal;
        v.uv[0] = uv.x;
        v.uv[1] = uv.y;
        v.light = 255;
    };

    const glm::vec2 uvSize = uv.uv1 - uv.uv0;
    const glm::vec2 uvU = glm::vec2(uvSize.x * static_cast<float>(w), 0.0f);
    const glm::vec2 uvV = glm::vec2(0.0f, uvSize.y * static_cast<float>(h));

    const std::uint32_t packedNormal = pack_normal(glm::normalize(normal));

    if (!flip)
    {
        write_vertex(v0, origin, uv.uv0, packedNormal);
        write_vertex(v1, origin + dv, uv.uv0 + uvV, packedNormal);
        write_vertex(v2, origin + dv + du, uv.uv0 + uvV + uvU, packedNormal);
        write_vertex(v3, origin + du, uv.uv0 + uvU, packedNormal);
    }
    else
    {
        write_vertex(v0, origin, uv.uv0, packedNormal);
        write_vertex(v1, origin + du, uv.uv0 + uvU, packedNormal);
        write_vertex(v2, origin + du + dv, uv.uv0 + uvU + uvV, packedNormal);
        write_vertex(v3, origin + dv, uv.uv0 + uvV, packedNormal);
    }

    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);

    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

} // namespace

// GreedyMesher collapses coplanar faces within a chunk section by building a 2D mask per
// axis (positive and negative). Each mask entry stores the block ID that should contribute
// a face; spans of identical blocks are merged into a single quad. This dramatically reduces
// triangle counts compared to naive voxel meshing, especially for large flat surfaces.
void GreedyMesher::build(const Chunk& chunk,
                         const NeighborSet& neighbors,
                         std::uint8_t lod,
                         bool opaquePass,
                         std::vector<renderer::ChunkVertex>& vertices,
                         std::vector<std::uint32_t>& indices)
{
    vertices.clear();
    indices.clear();

    const int step = 1 << lod;
    const int dims[3] = {ChunkWidth / step, ChunkHeight / step, ChunkDepth / step};
    const float stepF = static_cast<float>(step);

    std::vector<MaskCell> mask(static_cast<std::size_t>(dims[UAxis[0]] * dims[VAxis[0]]));

    auto sampleAgg = [&](int ax, int ay, int az) {
        const int x = ax * step;
        const int y = ay * step;
        const int z = az * step;
        return sample_block(chunk, neighbors, x, y, z);
    };

    auto process_orientation = [&](int axis, bool positive) {
        const int uAxis = UAxis[axis];
        const int vAxis = VAxis[axis];
        const int maskWidth = dims[uAxis];
        const int maskHeight = dims[vAxis];
        mask.assign(static_cast<std::size_t>(maskWidth * maskHeight), {});

        for (int slice = 0; slice <= dims[axis]; ++slice)
        {
            // Build mask for current slice.
            for (int j = 0; j < maskHeight; ++j)
            {
                for (int i = 0; i < maskWidth; ++i)
                {
                    int coord[3];
                    coord[axis] = slice + (positive ? -1 : 0);
                    coord[uAxis] = i;
                    coord[vAxis] = j;

                    int neighborCoord[3] = {coord[0], coord[1], coord[2]};
                    neighborCoord[axis] = coord[axis] + (positive ? 1 : -1);

                    const BlockID front = sampleAgg(coord[0], coord[1], coord[2]);
                    const BlockID back = sampleAgg(neighborCoord[0], neighborCoord[1], neighborCoord[2]);

                    const std::size_t index = static_cast<std::size_t>(i + j * maskWidth);
                    mask[index].filled = false;

                    if (positive)
                    {
                        if (!front || front == BlockAir)
                            continue;

                        if (opaquePass)
                        {
                            if (is_opaque(front) && !is_opaque(back) && !is_fluid(back))
                            {
                                mask[index].filled = true;
                                mask[index].block = front;
                            }
                        }
                        else
                        {
                            if ((is_transparent(front) || is_fluid(front)) && !(is_transparent(back) || is_fluid(back)))
                            {
                                mask[index].filled = true;
                                mask[index].block = front;
                            }
                        }
                    }
                    else
                    {
                        if (!back || back == BlockAir)
                            continue;

                        if (opaquePass)
                        {
                            if (is_opaque(back) && !is_opaque(front) && !is_fluid(front))
                            {
                                mask[index].filled = true;
                                mask[index].block = back;
                            }
                        }
                        else
                        {
                            if ((is_transparent(back) || is_fluid(back)) && !(is_transparent(front) || is_fluid(front)))
                            {
                                mask[index].filled = true;
                                mask[index].block = back;
                            }
                        }
                    }
                }
            }

            // Greedy merge over mask.
            for (int j = 0; j < maskHeight; ++j)
            {
                for (int i = 0; i < maskWidth;)
                {
                    const std::size_t idx = static_cast<std::size_t>(i + j * maskWidth);
                    if (!mask[idx].filled)
                    {
                        ++i;
                        continue;
                    }

                    const BlockID block = mask[idx].block;
                    int width = 1;
                    while (i + width < maskWidth)
                    {
                        const std::size_t next = static_cast<std::size_t>(i + width + j * maskWidth);
                        if (!mask[next].filled || mask[next].block != block)
                            break;
                        ++width;
                    }

                    int height = 1;
                    bool done = false;
                    while (j + height < maskHeight && !done)
                    {
                        for (int k = 0; k < width; ++k)
                        {
                            const std::size_t next = static_cast<std::size_t>(i + k + (j + height) * maskWidth);
                            if (!mask[next].filled || mask[next].block != block)
                            {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                        {
                            ++height;
                        }
                    }

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            const std::size_t clearIdx = static_cast<std::size_t>(i + x + (j + y) * maskWidth);
                            mask[clearIdx].filled = false;
                        }
                    }

                    const auto& def = registry().definition(block);
                    const auto& faceUV = def.faces[static_cast<int>(axis_face(axis, positive))];
                    const AtlasUV uv = atlas_uv(faceUV);

                    glm::vec3 origin(0.0f);
                    glm::vec3 du(0.0f);
                    glm::vec3 dv(0.0f);
                    glm::vec3 normal(0.0f);

                    origin[axis] = static_cast<float>(slice) * stepF;
                    origin[uAxis] = static_cast<float>(i) * stepF;
                    origin[vAxis] = static_cast<float>(j) * stepF;

                    du[uAxis] = static_cast<float>(width) * stepF;
                    dv[vAxis] = static_cast<float>(height) * stepF;

                    normal[axis] = positive ? 1.0f : -1.0f;

                    const bool flip = !positive;
                    emit_quad(vertices, indices, origin, du, dv, normal, uv, width, height, flip);
                }
            }
        }
    };

    for (int axis = 0; axis < 3; ++axis)
    {
        process_orientation(axis, true);
        process_orientation(axis, false);
    }
}

} // namespace world
