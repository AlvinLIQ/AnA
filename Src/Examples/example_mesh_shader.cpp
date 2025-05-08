#include "Headers/example_mesh_shader.hpp"
#include "FastNoise.h"  // Include FastNoise library

using namespace AnA;
using namespace Examples;

std::vector<MeshInfo> meshInfos = 
{
    {"Models/cube.obj", {}, 4},
    {"Models/bunny.obj", {{0.0f, -10.0f, 0.0f}, {30.0f, 30.0f, 30.0f}, {glm::pi<float>(), 0.0f, 0.0f}}}
};


// Function to generate terrain height using Perlin noise
float getHeight(float x, float z, float scale = 0.1f, int octaves = 6, float lacunarity = 2.0f, int seed = 42) {
    // Create a FastNoise instance
    FastNoiseLite noise;
    
    // Set the noise type to Perlin
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    
    // Set the seed for randomization
    noise.SetSeed(seed);
    
    // Set frequency scaling
    noise.SetFrequency(scale);
    
    // Set octaves, persistence, and lacunarity
    noise.SetFractalType(FastNoiseLite::FractalType_FBm); // Using Fractal Brownian Motion (FBM)
    noise.SetFractalOctaves(octaves);
    noise.SetFractalLacunarity(lacunarity);
    
    // Generate the height using Perlin noise for the (x, z) coordinates
    float height = noise.GetNoise(x, z) * 15.0f;

    return height;
}

glm::vec3 calculateNormal(int x, int z) {
    // Get the height of the current point (x, z)
//    float heightCenter = getHeight(x, z);

    // Get the height of the neighboring points
    float heightLeft = getHeight(x - 1, z);
    float heightRight = getHeight(x + 1, z);
    float heightUp = getHeight(x, z + 1);
    float heightDown = getHeight(x, z - 1);

    // Calculate the two vectors on the surface
    glm::vec3 v1(2.0f, heightRight - heightLeft, 0.0f);  // Horizontal vector (x-direction)
    glm::vec3 v2(0.0f, heightUp - heightDown, 2.0f);    // Vertical vector (z-direction)

    // Compute the cross product to get the normal vector
    glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

    return normal;
}

void GenTerrain(std::vector<Model::Vertex>& vertices, std::vector<uint32_t>& indices, float width, float height)
{
    for (float y = 0, x, z, ax, ay; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            ax = x / width * 1000.0f;
            ay = y / height * 1000.0f;
            z = getHeight(x, y);
            vertices.push_back({{ax, z, ay}, calculateNormal(ax, ay), 
                {(float)(x * 0.25), (float)(y * 0.25)}});
        }
    }
    for (uint32_t y = 0, x; y < static_cast<uint32_t>(height) - 1; y += 1)
    {
        for (x = 0; x < static_cast<uint32_t>(width) - 1; x += 1)
        {
            indices.push_back(x + y * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + y * static_cast<uint32_t>(width));
            indices.push_back(x + (y + 1) * static_cast<uint32_t>(width));
            indices.push_back(x + (y + 1) * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + y * static_cast<uint32_t>(width));
            indices.push_back(x + 1 + (y + 1) * static_cast<uint32_t>(width));
        }
    }
}

int main()
{
    example_mesh_shader app;
    app.Init();
    Resource::ResourceManager::GetCurrent()->MainScene.Append(meshInfos);
    app.Run();
    return 0;
}