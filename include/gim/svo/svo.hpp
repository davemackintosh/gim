#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vendor/fastnoiselite.hpp>

class SimplexNoise {
  private:
    FastNoiseLite noise;

  public:
    SimplexNoise();

    [[nodiscard]] float getNoise(float x, float y, float z) const;
};

struct Voxel {
    glm::ivec3 position; // Voxel position in 3D space
    float elevation;     // Elevation or height of the terrain at this voxel
    // Add more voxel data as needed for terrain representation
};

struct Node {
    bool isLeaf;
    Voxel voxel;
    int childrenOffset; // Offset to children in the array
};

class SparseVoxelOctree {
  private:
    std::vector<Node> nodes;

  public:
    SparseVoxelOctree() = default;

    // Function to insert a voxel into the octree
    void insertVoxel(const Voxel &voxel);
};

class TerrainGenerator {
  private:
    SimplexNoise noise;

  public:
    [[nodiscard]] float generateTerrainElevation(int x, int y, int z) const;
};
