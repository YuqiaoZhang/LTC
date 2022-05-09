#pragma once

struct RenderList 
{
    struct Model* models;
    uint64_t count;
};

namespace PrimitiveDemo
{
    void init();
    void shutDown();

    RenderList renderListScene();
    RenderList renderListLights();
	LightData* lightSettings();
}

namespace SponzaDemo
{
    void init();
    void shutDown();

    RenderList renderListScene();
    RenderList renderListLights();
	LightData* lightSettings();
}