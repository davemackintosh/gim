#include <gim/svo/svo.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

// Simplex noise implementation (simplified for demonstration)
SimplexNoise::SimplexNoise() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
}

[[nodiscard]] float SimplexNoise::getNoise(float x, float y, float z) const {
    // Implement simplex noise here (e.g., use a library or your own
    // implementation) For simplicity, this is a placeholder returning zero
    // noise
    return this->noise.GetNoise(x, y, z);
};

void SparseVoxelOctree::insertVoxel(const Voxel &voxel) {
    Node node{};
    node.isLeaf = true;
    node.voxel = voxel;
    node.childrenOffset = -1; // No children for a leaf node
    this->nodes.push_back(node);
};

[[nodiscard]] float TerrainGenerator::generateTerrainElevation(int x, int y,
                                                               int z) const {
    // Use Simplex noise to generate terrain elevation
    // Adjust parameters for more varied or smoother terrain
    return noise.getNoise(static_cast<float>(x) * 0.1f,
                          static_cast<float>(y) * 0.1f,
                          static_cast<float>(z) * 0.1f) *
           30.0f;
};

int main() {
    SparseVoxelOctree svo;
    TerrainGenerator terrainGenerator;

    // Generate terrain and insert voxels into the octree
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            for (int z = 0; z < 10; ++z) {
                glm::ivec3 position(x, y, z);
                float elevation =
                    terrainGenerator.generateTerrainElevation(x, y, z);
                Voxel voxel{position, elevation};
                svo.insertVoxel(voxel);
            }
        }
    }

    return 0;
}
