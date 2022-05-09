#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include "arealight_common.h"
#include "demos.h"

enum 
{
    eModelCount = 1,
    eMaxLightCount = 2,
};

static Model s_models[eModelCount];
static Model s_lights[eMaxLightCount];
static LightData s_lightData[eMaxLightCount];

namespace SponzaDemo
{

// Horizontal/vertical plane geometry
static PosNormalTexcoordVertex s_vplaneVertices[] =
{
    { -1.0f,  1.0f, 0.0f, packF4u(0.0f, 0.0f, -1.0f), 0.0f, 0.0f },
    {  1.0f,  1.0f, 0.0f, packF4u(0.0f, 0.0f, -1.0f), 1.0f, 0.0f },
    { -1.0f, -1.0f, 0.0f, packF4u(0.0f, 0.0f, -1.0f), 0.0f, 1.0f },
    {  1.0f, -1.0f, 0.0f, packF4u(0.0f, 0.0f, -1.0f), 1.0f, 1.0f },
};

static const uint16_t s_planeIndices[] =
{
    0, 1, 2,
    1, 3, 2,
};

static void setDefaultLightState()
{
    // Setup the lights
    s_lightData[0].rotation = glm::vec3(0.0f, 90.0f, 0.0f);
    s_lightData[0].scale = glm::vec2(29.0f);
    s_lightData[0].intensity = 15.0f;
    s_lightData[0].position = glm::vec3(-104.0f, 16.0f, -3.5f);
    s_lightData[0].twoSided = false;
    
    s_lightData[1].rotation.y = -90.0f;
    s_lightData[1].scale = glm::vec2(29.0f);
    s_lightData[1].intensity = 10.0f;
    s_lightData[1].position = glm::vec3(90.0f, 16.0f, -3.5f);
    s_lightData[1].textureIdx = 1;
}

void init()
{
    // Vertex declarations.
    bgfx::VertexDecl PosNormalTexcoordDecl;
    PosNormalTexcoordDecl.begin()
        .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    // Mesh loading
    s_models[0].loadModel("meshes/morgan-sponza.bin");
    for (int i = 0; i < eMaxLightCount; ++i)
        s_lights[i].loadModel(s_vplaneVertices, BX_COUNTOF(s_vplaneVertices), PosNormalTexcoordDecl, s_planeIndices, BX_COUNTOF(s_planeIndices));
    
    setDefaultLightState();
}

RenderList renderListScene()
{
    RenderList rlist;
    rlist.models = &s_models[0];
    rlist.count = eModelCount;
    return rlist;
}

RenderList renderListLights()
{
    RenderList rlist;
    rlist.models = &s_lights[0];
    rlist.count = 2;
    return rlist;
}

LightData* lightSettings()
{
    return &s_lightData[0];
}

void shutDown()
{
    for (int i = 0; i < eModelCount; ++i)
        s_models[i].unload();
}

} // namespace SponzaDemo1