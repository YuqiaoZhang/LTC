#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include "arealight_common.h"
#include "demos.h"

// meshes for demo
enum 
{
    eBunny,
    eCube,
    eHollowCube,
    eHPlane,
    eModelCount,

    eLightCount = 1,
};

static Model s_models[eModelCount];
static Model s_lights[eLightCount];
static LightData s_lightData[eLightCount];

namespace PrimitiveDemo
{

// horizontal/vertical plane geometry
static const float s_texcoord = 50.0f;
static PosNormalTangentTexcoordVertex s_hplaneVertices[] =
{
    { -1.0f, 0.0f,  1.0f, packF4u(0.0f, 1.0f, 0.0f), packF4u(0.0f, 0.0f, 1.0f), s_texcoord, s_texcoord },
    {  1.0f, 0.0f,  1.0f, packF4u(0.0f, 1.0f, 0.0f), packF4u(0.0f, 0.0f, 1.0f), s_texcoord, 0.0f       },
    { -1.0f, 0.0f, -1.0f, packF4u(0.0f, 1.0f, 0.0f), packF4u(0.0f, 0.0f, 1.0f), 0.0f,       s_texcoord },
    {  1.0f, 0.0f, -1.0f, packF4u(0.0f, 1.0f, 0.0f), packF4u(0.0f, 0.0f, 1.0f), 0.0f,       0.0f       },
};

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

void init()
{
    // Vertex declarations.
    bgfx::VertexDecl PosNormalTexcoordDecl;
    PosNormalTexcoordDecl.begin()
        .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    bgfx::VertexDecl PosNormalTangentTexcoordDecl;
    PosNormalTangentTexcoordDecl.begin()
        .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
        .add(bgfx::Attrib::Tangent,   4, bgfx::AttribType::Uint8, true, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    // mesh loading
    s_models[eBunny].loadModel("meshes/bunny.bin");
    s_models[eCube].loadModel("meshes/cube.bin");
    s_models[eHollowCube].loadModel("meshes/hollowcube.bin");
    s_models[eHPlane].loadModel(s_hplaneVertices, BX_COUNTOF(s_hplaneVertices), PosNormalTangentTexcoordDecl, s_planeIndices, BX_COUNTOF(s_planeIndices));
    
    s_lights[0].loadModel(s_vplaneVertices, BX_COUNTOF(s_vplaneVertices), PosNormalTexcoordDecl, s_planeIndices, BX_COUNTOF(s_planeIndices));

    // Setup instance matrices.   
    using namespace glm;
    const float floorScale = 550.0f;
    s_models[eHPlane].transform = scale(s_models[eHPlane].transform, vec3(floorScale, floorScale, floorScale));
    
    s_models[eBunny].transform = translate(s_models[eBunny].transform, vec3(15.0f, 5.0f, 0.0f));
    s_models[eBunny].transform = scale(s_models[eBunny].transform, vec3(5.0f, 5.0f, 5.0f));

    s_models[eHollowCube].transform = translate(s_models[eHollowCube].transform, vec3(0.0f, 10.0f, 0.0f));
    s_models[eHollowCube].transform = scale(s_models[eHollowCube].transform, vec3(2.5f, 2.5f, 2.5f));

    s_models[eCube].transform = translate(s_models[eCube].transform, vec3(-15.0f, 5.0f, 0.0f));
    s_models[eCube].transform = scale(s_models[eCube].transform, vec3(2.5f, 2.5f, 2.5f));

    // setup the lights
    s_lightData[0].rotation.z = 0.0f;
    s_lightData[0].scale = glm::vec2(64.0f);
    s_lightData[0].intensity = 4.0f;
    s_lightData[0].position = glm::vec3(0.0f, 0.0f, 40.0f);
    s_lightData[0].textureIdx = 2;
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
    rlist.count = eLightCount;
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

} // namespace PrimitiveDemo