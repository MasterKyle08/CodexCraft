#include "GreedyMesher.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include "../BlockRegistry.h"

namespace CodexCraft::World::Meshing {

namespace {

struct MaskFace {
    bool exists{ false };
    bool normalPositive{ true };
    BlockId block{ BlockId::Air };
    AtlasCell atlasCell{};
    bool transparent{ false };
};

constexpr int FaceIndexForAxis(int axis, bool normalPositive) noexcept {
    switch (axis) {
    case 0:
        return normalPositive ? 0 : 1;
    case 1:
        return normalPositive ? 2 : 3;
    case 2:
    default:
        return normalPositive ? 4 : 5;
    }
}

int RenderPriority(BlockId id, const BlockDefinition& definition) noexcept {
    if (id == BlockId::Air) {
        return 0;
    }

    if (definition.isOpaque) {
        return 2;
    }

    if (definition.isTransparent) {
        return 1;
    }

    return 0;
}

bool ShouldRenderFace(BlockId current,
                      BlockId neighbor,
                      const BlockDefinition& currentDef,
                      const BlockDefinition& neighborDef,
                      int currentPriority,
                      int neighborPriority) noexcept {
    if (currentPriority == 0) {
        return false;
    }

    if (neighborPriority == 0) {
        return true;
    }

    if (current == neighbor) {
        return false;
    }

    if (currentPriority > neighborPriority) {
        return true;
    }

    if (currentPriority == neighborPriority) {
        if (currentDef.isTransparent && neighborDef.isTransparent) {
            return current != neighbor;
        }
    }

    return false;
}

} // namespace

ChunkMeshData GreedyMesher::BuildMesh(const Chunk& chunk) const {
    ChunkMeshData meshData{};

    const int dimensions[3] = { kChunkWidth, kChunkHeight, kChunkDepth };

    std::vector<MaskFace> mask(static_cast<std::size_t>(dimensions[1]) * static_cast<std::size_t>(dimensions[2]));

    const auto fetchBlock = [&chunk](int x, int y, int z) {
        if (x < 0 || x >= kChunkWidth || y < 0 || y >= kChunkHeight || z < 0 || z >= kChunkDepth) {
            return BlockId::Air;
        }

        return chunk.GetBlockId(x, y, z);
    };

    for (int axis = 0; axis < 3; ++axis) {
        const int uAxis = (axis + 1) % 3;
        const int vAxis = (axis + 2) % 3;

        const int maskWidth = dimensions[uAxis];
        const int maskHeight = dimensions[vAxis];
        mask.assign(static_cast<std::size_t>(maskWidth * maskHeight), MaskFace{});

        for (int q = 0; q <= dimensions[axis]; ++q) {
            // Build a visibility mask for the current slice.
            for (int j = 0; j < maskHeight; ++j) {
                for (int i = 0; i < maskWidth; ++i) {
                    const std::size_t index = static_cast<std::size_t>(i + j * maskWidth);

                    BlockId positive = BlockId::Air;
                    BlockId negative = BlockId::Air;

                    std::array<int, 3> coordinates{};
                    coordinates[uAxis] = i;
                    coordinates[vAxis] = j;

                    if (q < dimensions[axis]) {
                        coordinates[axis] = q;
                        positive = fetchBlock(coordinates[0], coordinates[1], coordinates[2]);
                    }

                    if (q > 0) {
                        coordinates[axis] = q - 1;
                        negative = fetchBlock(coordinates[0], coordinates[1], coordinates[2]);
                    }

                    const BlockDefinition& positiveDef = GetBlockDefinition(positive);
                    const BlockDefinition& negativeDef = GetBlockDefinition(negative);
                    const int positivePriority = RenderPriority(positive, positiveDef);
                    const int negativePriority = RenderPriority(negative, negativeDef);

                    MaskFace face{};

                    if (ShouldRenderFace(negative, positive, negativeDef, positiveDef, negativePriority, positivePriority)) {
                        face.exists = true;
                        face.normalPositive = true;
                        face.block = negative;
                        face.atlasCell = negativeDef.atlasCells[FaceIndexForAxis(axis, true)];
                        face.transparent = negativeDef.isTransparent;
                    } else if (ShouldRenderFace(positive, negative, positiveDef, negativeDef, positivePriority, negativePriority)) {
                        face.exists = true;
                        face.normalPositive = false;
                        face.block = positive;
                        face.atlasCell = positiveDef.atlasCells[FaceIndexForAxis(axis, false)];
                        face.transparent = positiveDef.isTransparent;
                    }

                    mask[index] = face;
                }
            }

            // Greedy merge quads in the mask.
            for (int j = 0; j < maskHeight; ++j) {
                int i = 0;
                while (i < maskWidth) {
                    const std::size_t index = static_cast<std::size_t>(i + j * maskWidth);
                    const MaskFace& face = mask[index];
                    if (!face.exists) {
                        ++i;
                        continue;
                    }

                    int width = 1;
                    while (i + width < maskWidth) {
                        const MaskFace& candidate = mask[static_cast<std::size_t>(i + width + j * maskWidth)];
                        if (!candidate.exists || candidate.normalPositive != face.normalPositive ||
                            candidate.block != face.block || candidate.transparent != face.transparent ||
                            candidate.atlasCell.u != face.atlasCell.u || candidate.atlasCell.v != face.atlasCell.v) {
                            break;
                        }
                        ++width;
                    }

                    int height = 1;
                    bool canExpand = true;
                    while (canExpand && j + height < maskHeight) {
                        for (int x = 0; x < width; ++x) {
                            const MaskFace& candidate =
                                mask[static_cast<std::size_t>(i + x + (j + height) * maskWidth)];
                            if (!candidate.exists || candidate.normalPositive != face.normalPositive ||
                                candidate.block != face.block || candidate.transparent != face.transparent ||
                                candidate.atlasCell.u != face.atlasCell.u ||
                                candidate.atlasCell.v != face.atlasCell.v) {
                                canExpand = false;
                                break;
                            }
                        }

                        if (canExpand) {
                            ++height;
                        }
                    }

                    const auto emitQuad = [&](const std::array<float, 3>& v0,
                                              const std::array<float, 3>& v1,
                                              const std::array<float, 3>& v2,
                                              const std::array<float, 3>& v3) {
                        auto& vertices = face.transparent ? meshData.transparentVertices : meshData.opaqueVertices;
                        auto& indices = face.transparent ? meshData.transparentIndices : meshData.opaqueIndices;

                        const std::uint32_t baseIndex = static_cast<std::uint32_t>(vertices.size());

                        std::array<float, 3> normal{};
                        normal[axis] = face.normalPositive ? 1.0f : -1.0f;

                        const auto pushVertex = [&](const std::array<float, 3>& position,
                                                    std::uint16_t offsetU,
                                                    std::uint16_t offsetV) {
                            ChunkVertex vertex{};
                            vertex.position[0] = position[0];
                            vertex.position[1] = position[1];
                            vertex.position[2] = position[2];
                            vertex.normal[0] = normal[0];
                            vertex.normal[1] = normal[1];
                            vertex.normal[2] = normal[2];
                            vertex.atlasCellU = face.atlasCell.u;
                            vertex.atlasCellV = face.atlasCell.v;
                            vertex.tileWidth = static_cast<std::uint16_t>(width);
                            vertex.tileHeight = static_cast<std::uint16_t>(height);
                            vertex.cornerOffsetU = offsetU;
                            vertex.cornerOffsetV = offsetV;

                            vertices.push_back(vertex);
                        };

                        pushVertex(v0, 0, 0);
                        pushVertex(v1,
                                   face.normalPositive ? 0 : static_cast<std::uint16_t>(width),
                                   face.normalPositive ? static_cast<std::uint16_t>(height) : 0);
                        pushVertex(v2, static_cast<std::uint16_t>(width), static_cast<std::uint16_t>(height));
                        pushVertex(v3,
                                   face.normalPositive ? static_cast<std::uint16_t>(width) : 0,
                                   face.normalPositive ? 0 : static_cast<std::uint16_t>(height));

                        indices.push_back(baseIndex + 0);
                        indices.push_back(baseIndex + 1);
                        indices.push_back(baseIndex + 2);
                        indices.push_back(baseIndex + 0);
                        indices.push_back(baseIndex + 2);
                        indices.push_back(baseIndex + 3);
                    };

                    std::array<float, 3> base{};
                    base[uAxis] = static_cast<float>(i);
                    base[vAxis] = static_cast<float>(j);
                    base[axis] = static_cast<float>(q);

                    std::array<float, 3> du{};
                    du[uAxis] = static_cast<float>(width);

                    std::array<float, 3> dv{};
                    dv[vAxis] = static_cast<float>(height);

                    if (face.normalPositive) {
                        const std::array<float, 3> v0 = base;
                        const std::array<float, 3> v1 = { base[0] + dv[0], base[1] + dv[1], base[2] + dv[2] };
                        const std::array<float, 3> v2 = { base[0] + du[0] + dv[0],
                                                          base[1] + du[1] + dv[1],
                                                          base[2] + du[2] + dv[2] };
                        const std::array<float, 3> v3 = { base[0] + du[0], base[1] + du[1], base[2] + du[2] };
                        emitQuad(v0, v1, v2, v3);
                    } else {
                        const std::array<float, 3> v0 = base;
                        const std::array<float, 3> v1 = { base[0] + du[0], base[1] + du[1], base[2] + du[2] };
                        const std::array<float, 3> v2 = { base[0] + du[0] + dv[0],
                                                          base[1] + du[1] + dv[1],
                                                          base[2] + du[2] + dv[2] };
                        const std::array<float, 3> v3 = { base[0] + dv[0], base[1] + dv[1], base[2] + dv[2] };
                        emitQuad(v0, v1, v2, v3);
                    }

                    // Clear the consumed mask entries.
                    for (int y = 0; y < height; ++y) {
                        for (int x = 0; x < width; ++x) {
                            mask[static_cast<std::size_t>(i + x + (j + y) * maskWidth)].exists = false;
                        }
                    }

                    i += width;
                }
            }
        }
    }

    return meshData;
}

} // namespace CodexCraft::World::Meshing

