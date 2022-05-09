// standard includes
#include <string>
#include <vector>
#include <algorithm>

#include "common.h"

// external libraries
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace glm;

// core includes
#include <bgfx/bgfx.h>
#include <bx/timer.h>
#include <bx/fpumath.h>

// project includes
#include "arealight_common.h"
#include "demos.h"

#define RENDERVIEW_DRAWSCENE_0_ID 1

static bool s_flipV = false;

static bgfx::FrameBufferHandle s_rtColorBuffer;

static GlobalRenderingData s_gData;

std::map<std::string, bgfx::TextureHandle> Mesh::m_textureCache;

static RenderState s_renderStates[RenderState::Count] =
{
    { // Default
        0
        | BGFX_STATE_RGB_WRITE
        | BGFX_STATE_ALPHA_WRITE
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_DEPTH_WRITE
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        , UINT32_MAX
        , BGFX_STENCIL_NONE
        , BGFX_STENCIL_NONE
    },
    { // ZPass
        0
        | BGFX_STATE_DEPTH_WRITE
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        , UINT32_MAX
        , BGFX_STENCIL_NONE
        , BGFX_STENCIL_NONE
    },
    { // ZTwoSidedPass
        0
        | BGFX_STATE_DEPTH_WRITE
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_MSAA
        , UINT32_MAX
        , BGFX_STENCIL_NONE
        , BGFX_STENCIL_NONE
    },
    { // ColorPass
        0
        | BGFX_STATE_RGB_WRITE
        | BGFX_STATE_ALPHA_WRITE
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE)
        | BGFX_STATE_DEPTH_TEST_EQUAL
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        , UINT32_MAX
        , BGFX_STENCIL_NONE
        , BGFX_STENCIL_NONE
    },
    { // ColorAlphaPass
        0
        | BGFX_STATE_RGB_WRITE
        | BGFX_STATE_ALPHA_WRITE
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE)
        | BGFX_STATE_DEPTH_TEST_EQUAL
        | BGFX_STATE_MSAA
        , UINT32_MAX
        , BGFX_STENCIL_NONE
        , BGFX_STENCIL_NONE
    }
};

bgfx::VertexDecl PosColorTexCoord0Vertex::ms_decl;

struct Programs
{
    void init()
    {
        // Misc.
        m_colorConstant = loadProgram("vs_arealights_color", "fs_arealights_color_constant");
        m_colorTextured = loadProgram("vs_arealights_textured", "fs_arealights_color_textured");

        // Pack depth.
        m_packDepth      = loadProgram("vs_arealights_packdepth", "fs_arealights_packdepth");
        m_packDepthLight = loadProgram("vs_arealights_packdepth", "fs_arealights_packdepth_light");
        
        m_blit = loadProgram("vs_arealights_blit", "fs_arealights_blit");

        // Color lighting.
        m_diffuseLTC      = loadProgram("vs_arealights_lighting", "fs_arealights_diffuse_ltc");
        m_diffuseGT       = loadProgram("vs_arealights_lighting", "fs_arealights_diffuse_gt");
        m_specularLTC     = loadProgram("vs_arealights_lighting", "fs_arealights_specular_ltc");
        m_specularGT      = loadProgram("vs_arealights_lighting", "fs_arealights_specular_gt");
    }

    void destroy()
    {
        // Color lighting.
        bgfx::destroyProgram(m_diffuseLTC);
        bgfx::destroyProgram(m_diffuseGT);
        bgfx::destroyProgram(m_specularLTC);
        bgfx::destroyProgram(m_specularGT);

        // Pack depth.
        bgfx::destroyProgram(m_packDepth);
        bgfx::destroyProgram(m_packDepthLight);

        // Blit.
        bgfx::destroyProgram(m_blit);

        // Misc.
        bgfx::destroyProgram(m_colorConstant);
        bgfx::destroyProgram(m_colorTextured);
    }

    bgfx::ProgramHandle m_colorConstant;
    bgfx::ProgramHandle m_colorTextured;
    bgfx::ProgramHandle m_packDepth;
    bgfx::ProgramHandle m_packDepthLight;
    bgfx::ProgramHandle m_blit;
    bgfx::ProgramHandle m_diffuseLTC;
    bgfx::ProgramHandle m_diffuseGT;
    bgfx::ProgramHandle m_specularLTC;
    bgfx::ProgramHandle m_specularGT;
};
static Programs s_programs;

void screenSpaceQuad(bool _originBottomLeft = true, float zz = 0.0f, float _width = 1.0f, float _height = 1.0f)
{
    if (bgfx::checkAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_decl))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_decl);
        PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

        const float minx = -_width;
        const float maxx =  _width;
        const float miny = 0.0f;
        const float maxy = _height*2.0f;

        const float minu = -1.0f;
        const float maxu =  1.0f;

        float minv = 0.0f;
        float maxv = 2.0f;

        if (_originBottomLeft)
        {
            std::swap(minv, maxv);
            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_rgba = 0xffffffff;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_rgba = 0xffffffff;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_rgba = 0xffffffff;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        bgfx::setVertexBuffer(&vb);
    }
}

static float Halton(int index, float base)
{
    float result = 0.0f;
    float f = 1.0f/base;
    float i = float(index);
    for (;;)
    {
        if (i <= 0.0f)
            break;

        result += f*fmodf(i, base);
        i = floorf(i/base);
        f = f/base;
    }

    return result;
}

static void Halton4D(glm::vec4 s[NUM_SAMPLES], int offset)
{
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        s[i][0] = Halton(i + offset, 2.0f);
        s[i][1] = Halton(i + offset, 3.0f);
        s[i][2] = Halton(i + offset, 5.0f);
        s[i][3] = Halton(i + offset, 7.0f);
    }
}

glm::mat4 SetLightUniforms(const LightData& l)
{
    // setup light transform
    glm::mat4 identity(1.0f);

    glm::mat4 scale = glm::scale(identity, glm::vec3(0.5f*l.scale, 1.0f));
    glm::mat4 translate = glm::translate(identity, l.position);
    glm::mat4 rotateZ = glm::rotate(identity, glm::radians(l.rotation.z), glm::vec3(0, 0, -1));
    glm::mat4 rotateY = glm::rotate(identity, glm::radians(l.rotation.y), glm::vec3(0, -1, 0));
    glm::mat4 rotateX = glm::rotate(identity, glm::radians(l.rotation.x), glm::vec3(-1, 0, 0));

    glm::mat4 lightTransform = translate*rotateX*rotateY*rotateZ*scale;

    s_gData.s_uniforms.m_quadPoints[0] = lightTransform * glm::vec4(-1,  1, 0, 1);
    s_gData.s_uniforms.m_quadPoints[1] = lightTransform * glm::vec4( 1,  1, 0, 1);
    s_gData.s_uniforms.m_quadPoints[2] = lightTransform * glm::vec4( 1, -1, 0, 1);
    s_gData.s_uniforms.m_quadPoints[3] = lightTransform * glm::vec4(-1, -1, 0, 1);

    // set the light state
    s_gData.s_uniforms.m_lightIntensity = l.intensity;
    s_gData.s_uniforms.m_twoSided       = l.twoSided;
    
	s_gData.s_uniforms.submitPerLightUniforms();

	return lightTransform;
}

LightMaps LightColorMaps(uint32_t textureIdx)
{
    // TODO: stick everything into an array?
    switch (textureIdx)
    {
        case 0:  return s_gData.s_texStainedGlassMaps; break;
        case 1:  return s_gData.s_texWhiteMaps;        break;
        default: return s_gData.s_texWhiteMaps;        break;
    }
}

glm::vec2 jitterProjMatrix(float proj[16], int sampleCount, float jitterAASigma, float width, float height)
{
    // Per-frame jitter to camera for AA
    const int frameNum = sampleCount + 1; // Add 1 since otherwise first sample is an outlier

    float u1 = Halton(frameNum, 2.0f);
    float u2 = Halton(frameNum, 3.0f);
    
    // Gaussian sample
    float phi = 2.0f*pi<float>()*u2;
    float r = jitterAASigma*sqrtf(-2.0f*log(std::max(u1, 1e-7f)));
    float x = r*cos(phi);
    float y = r*sin(phi);

    proj[8] += x*2.0f/width;
    proj[9] += y*2.0f/height;

    return glm::vec2(x, y);
}

bgfx::ProgramHandle GetLightProgram(bool diffuse, bool groundTruth)
{
    if (diffuse)
    {
        if (groundTruth)
            return s_programs.m_diffuseGT;
        
        return s_programs.m_diffuseLTC;
    }
    else
    {
        if (groundTruth)
            return s_programs.m_specularGT;
        
        return s_programs.m_specularLTC;
    }
}

int _main_(int _argc, char** _argv)
{
    Args args(_argc, _argv);

    uint32_t debug = BGFX_DEBUG_TEXT;
    uint32_t reset = BGFX_RESET_MAXANISOTROPY | BGFX_RESET_VSYNC;

    ViewState viewState(1280, 720);
    ClearValues clearValues(0x00000000, 1.0f, 0);

    bgfx::init(args.m_type, args.m_pciId);

    bgfx::reset(viewState.m_width, viewState.m_height, reset);

    // Enable debug text.
    bgfx::setDebug(debug);

    bgfx::RendererType::Enum type = bgfx::getRendererType();

    // Setup root path for binary shaders. Shader binaries are different
    // for each renderer.
    switch (type)
    {
    case bgfx::RendererType::OpenGL:
    case bgfx::RendererType::OpenGLES:
        s_flipV = true;
        break;

    default:
        break;
    }

    // Imgui.
    imguiCreate();

    // Uniforms.
    s_gData.s_uniforms.init();
    s_gData.s_uLTCMat           = bgfx::createUniform("s_texLTCMat",      bgfx::UniformType::Int1);
    s_gData.s_uLTCAmp           = bgfx::createUniform("s_texLTCAmp",      bgfx::UniformType::Int1);
    s_gData.s_uColorMap         = bgfx::createUniform("s_texColorMap",    bgfx::UniformType::Int1);
    s_gData.s_uFilteredMap      = bgfx::createUniform("s_texFilteredMap", bgfx::UniformType::Int1);

    s_gData.s_uDiffuseUniform   = bgfx::createUniform("s_albedoMap",      bgfx::UniformType::Int1);
    s_gData.s_uNmlUniform       = bgfx::createUniform("s_nmlMap",         bgfx::UniformType::Int1);
    s_gData.s_uRoughnessUniform = bgfx::createUniform("s_roughnessMap",   bgfx::UniformType::Int1);
    s_gData.s_umetallicUniform  = bgfx::createUniform("s_metallicMap",    bgfx::UniformType::Int1);

    // Programs.
    s_programs.init();

    bgfx::VertexDecl posDecl;
    posDecl.begin();
    posDecl.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float);
    posDecl.end();

    PosColorTexCoord0Vertex::init();

    const uint32_t samplerFlags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;

    // Textures.
    s_gData.s_texLTCMat = loadTexture("ltc_mat.dds", samplerFlags);
    s_gData.s_texLTCAmp = loadTexture("ltc_amp.dds", samplerFlags);

    s_gData.s_texStainedGlassMaps.colorMap    = loadTexture("textures_lights/stained_glass.zlib",          samplerFlags | BGFX_TEXTURE_MIN_ANISOTROPIC);
    s_gData.s_texStainedGlassMaps.filteredMap = loadTexture("textures_lights/stained_glass_filtered.zlib", samplerFlags);
    s_gData.s_texWhiteMaps.colorMap           = loadTexture("white.png", samplerFlags);
    s_gData.s_texWhiteMaps.filteredMap        = loadTexture("white.png", samplerFlags);
    
    // initialize demos
    PrimitiveDemo::init();
    SponzaDemo::init();

    struct SceneSettings
    {
        float    m_diffColor[3];
        float    m_roughness;
        float    m_reflectance;
        float    m_jitterAASigma;
        uint32_t m_sampleCount;
        uint32_t m_demoIdx;
        uint32_t m_currLightIdx;
        bool     m_useGT;
        bool     m_showDiffColor;
    };

    SceneSettings settings;
    settings.m_diffColor[0]    =  1.0f;
    settings.m_diffColor[1]    =  1.0f;
    settings.m_diffColor[2]    =  1.0f;
    settings.m_roughness       =  1.0f;
    settings.m_reflectance     = 0.04f;
    settings.m_currLightIdx    =     0;
    settings.m_jitterAASigma   =  0.3f;
    settings.m_sampleCount     =     0;
    settings.m_demoIdx         =     1;
    settings.m_useGT           = false;
    settings.m_showDiffColor   = false;
    
    float prevDiffColor[3] = { 0 };

    s_rtColorBuffer.idx = bgfx::invalidHandle;

    // Setup camera.
    float initialPrimPos[3] = { 0.0f, 60.0f, -60.0f };
    float initialSponzaPos[3] = { 0.0f, 20.0f, 0.0f };
    float initialPrimVertAngle = -0.45f;
    float initialSponzaVertAngle = 0.0f;
    float initialPrimHAngle = 0.0f;
    float initialSponzaHAngle = -1.54f;
    cameraCreate();
    cameraSetPosition(initialSponzaPos);
    cameraSetHorizontalAngle(initialSponzaHAngle);
    cameraSetVerticalAngle(initialSponzaVertAngle);

    // Set view and projection matrices.
    const float camFovy    = 60.0f;
    const float camAspect  = float(int32_t(viewState.m_width)) / float(int32_t(viewState.m_height));
    const float camNear    = 0.1f;
    const float camFar     = 2000.0f;
    bx::mtxProj(viewState.m_proj, camFovy, camAspect, camNear, camFar);
    cameraGetViewMtx(viewState.m_view);

    bool demoChanged = false;

    entry::MouseState mouseState;
    while (!entry::processEvents(viewState.m_width, viewState.m_height, debug, reset, &mouseState))
    {

        // pick which demo to render
        RenderList rlist;
        RenderList llist;
        LightData* lsettings;
        switch (settings.m_demoIdx)
        {
            default:
            case 0:
                rlist     = PrimitiveDemo::renderListScene();
                llist     = PrimitiveDemo::renderListLights();
                lsettings = PrimitiveDemo::lightSettings();
                break;

            case 1:
                rlist     = SponzaDemo::renderListScene();
                llist     = SponzaDemo::renderListLights();
                lsettings = SponzaDemo::lightSettings();
				break;
        }

        bool rtRecreated = false;
        bool uiChanged = demoChanged;
        demoChanged = false;

        if (viewState.m_oldWidth  != viewState.m_width
        ||  viewState.m_oldHeight != viewState.m_height
        ||  viewState.m_oldReset  != reset)
        {
            viewState.m_oldWidth  = viewState.m_width;
            viewState.m_oldHeight = viewState.m_height;
            viewState.m_oldReset  = reset;

            if (bgfx::isValid(s_rtColorBuffer))
            {
                bgfx::destroyFrameBuffer(s_rtColorBuffer);
            }

            int w = viewState.m_width;
            int h = viewState.m_height;

            // Note: bgfx will cap the quality to the maximum supported
			uint32_t rtFlags = BGFX_TEXTURE_RT;

            bgfx::TextureHandle colorTextures[] =
            {
                bgfx::createTexture2D(w, h, 1, bgfx::TextureFormat::RGBA32F, rtFlags),
                bgfx::createTexture2D(w, h, 1, bgfx::TextureFormat::D24S8,   rtFlags)
            };
            s_rtColorBuffer = bgfx::createFrameBuffer(BX_COUNTOF(colorTextures), colorTextures, true);
            rtRecreated = true;
        }

        {
            // Imgui.
            imguiBeginFrame(mouseState.m_mx
                , mouseState.m_my
                , (mouseState.m_buttons[entry::MouseButton::Left]   ? IMGUI_MBUT_LEFT   : 0)
                | (mouseState.m_buttons[entry::MouseButton::Right]  ? IMGUI_MBUT_RIGHT  : 0)
                | (mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
                , mouseState.m_mz
                , viewState.m_width
                , viewState.m_height
                );

#define IMGUI_FLOAT_SLIDER(_name, _val) \
                imguiSlider(_name \
                        , _val \
                        , *( ((float*)&_val)+1) \
                        , *( ((float*)&_val)+2) \
                        , *( ((float*)&_val)+3) \
                        )

            static int32_t leftScrollArea = 0;
            static bool editSceneParams = false;
            const uint32_t debugWindHeight = 700;
            imguiBeginScrollArea("", 10, 70, 256, editSceneParams ? debugWindHeight : 32, &leftScrollArea);

            imguiBool("Scene Settings", editSceneParams);

            if (editSceneParams)
            {
                imguiColorWheel("Diffuse Color:", &settings.m_diffColor[0], settings.m_showDiffColor);

                // -- disable material editing in sponza
                if(settings.m_demoIdx != 1) 
                {
                    uiChanged |= imguiSlider("Reflectance:", settings.m_reflectance, 0.01f, 1.0f, 0.001f);

                    uiChanged |= imguiSlider("Roughness:", settings.m_roughness, float(MIN_ROUGHNESS), 1.0f, 0.001f);
                }

                imguiSeparatorLine();

                uiChanged |= imguiBool("Ground Truth", settings.m_useGT);

                imguiSeparator();
                imguiLabel("Demo Scene");
                {
                    imguiIndent(16);
                    uint32_t newIdx = imguiChoose(settings.m_demoIdx, "Simple Primitives", "Sponza");
                    if (newIdx != settings.m_demoIdx) 
                    {
                        if (newIdx == 0)
                        {
                            cameraSetPosition(initialPrimPos);
                            cameraSetHorizontalAngle(initialPrimHAngle);
                            cameraSetVerticalAngle(initialPrimVertAngle);
                            settings.m_diffColor[0] = 0.5f;
                            settings.m_diffColor[1] = 0.5f;
                            settings.m_diffColor[2] = 0.5f;
                            settings.m_roughness = 0.15f;
                        }
                        else 
                        {
                            cameraSetPosition(initialSponzaPos);
                            cameraSetHorizontalAngle(initialSponzaHAngle);
                            cameraSetVerticalAngle(initialSponzaVertAngle);
                            settings.m_diffColor[0] = 1.0f;
                            settings.m_diffColor[1] = 1.0f;
                            settings.m_diffColor[2] = 1.0f;
                            settings.m_roughness = 1.0f;
                        }
                        demoChanged = true;
                    }
                    settings.m_demoIdx = newIdx;
                    imguiUnindent(16);
                }

#undef IMGUI_FLOAT_SLIDER
            }

            imguiEndScrollArea();

            // light debug UI
            {
                static int32_t rightScrollArea = 0;
                static bool editLights = false;

                uint32_t startY = (editSceneParams ? debugWindHeight : 32) + 70;
                imguiBeginScrollArea("", 10, startY, 256, editLights ? debugWindHeight : 32, &rightScrollArea);
                imguiBool("Light Parameters", editLights);
                if (editLights)
                {
                    if (llist.count > 1)
                    {
                        float idx = (float)settings.m_currLightIdx;
                        uiChanged |= imguiSlider("Index", idx, 0.0f, (float)llist.count - 1, 1.0f);
                        settings.m_currLightIdx = (uint32_t)idx;
                        imguiSeparatorLine();
                    }

                    uint32_t lidx = settings.m_currLightIdx;

                    uiChanged |= imguiSlider("Intensity", lsettings[lidx].intensity, 0.0f, 30.0f, 0.01f);

                    uiChanged |= imguiSlider("Width",  lsettings[lidx].scale.x, 1.0f, 100.0f, 1.0f);
                    uiChanged |= imguiSlider("Height", lsettings[lidx].scale.y, 1.0f, 100.0f, 1.0f);

                    uiChanged |= imguiBool("Two Sided", lsettings[lidx].twoSided);

                    imguiSeparatorLine();

                    uiChanged |= imguiSlider("Position (X)", lsettings[lidx].position.x, -150.0f, 150.0f, 0.1f);
                    uiChanged |= imguiSlider("Position (Y)", lsettings[lidx].position.y, -150.0f, 150.0f, 0.1f);
                    uiChanged |= imguiSlider("Position (Z)", lsettings[lidx].position.z, -150.0f, 150.0f, 0.1f);

                    imguiSeparatorLine();

                    uiChanged |= imguiSlider("Rotation (X)", lsettings[lidx].rotation.x, -180, 179.0f, 1.0f);
                    uiChanged |= imguiSlider("Rotation (Y)", lsettings[lidx].rotation.y, -180, 179.0f, 1.0f);
                    uiChanged |= imguiSlider("Rotation (Z)", lsettings[lidx].rotation.z, -180, 179.0f, 1.0f);

                    imguiSeparator();
                    
                    imguiLabel("Texture");
                    {
                        imguiIndent(16);
                        uint32_t newIdx = imguiChoose(lsettings[lidx].textureIdx, "Stained Glass", "White");
                        if (newIdx != lsettings[lidx].textureIdx)
                            uiChanged = true;
                        lsettings[lidx].textureIdx = newIdx;
                        imguiUnindent(16);
                    }
                }
                imguiEndScrollArea();
            }

            imguiEndFrame();
        }

        // Time.
        int64_t now = bx::getHPCounter();
        static int64_t last = now;
        const int64_t frameTime = now - last;
        last = now;
        const double freq = double(bx::getHPFrequency());
        const double toMs = 1000.0/freq;
        const float deltaTime = float(frameTime/freq);

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/xx-arealights");
        bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Area lights example.");
        bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

        // Update camera.
        cameraUpdate(deltaTime, mouseState);
        
        // Update view mtx.
        cameraGetViewMtx(viewState.m_view);

        if (memcmp(viewState.m_oldView, viewState.m_view, sizeof(viewState.m_view))
        ||  memcmp(prevDiffColor, settings.m_diffColor, sizeof(settings.m_diffColor))
        ||  uiChanged || rtRecreated)
        {
            memcpy(viewState.m_oldView, viewState.m_view, sizeof(viewState.m_view));
            memcpy(prevDiffColor, settings.m_diffColor, sizeof(settings.m_diffColor));
            settings.m_sampleCount = 0;
        }

        // Lights.
        Light pointLight =
        {
            { { 0.0f, 0.0f, 0.0f, 1.0f } }, //position
        };

        pointLight.m_position.m_x = 0.0f;
        pointLight.m_position.m_y = 0.0f;
        pointLight.m_position.m_z = 0.0f;

        // Update uniforms.
        s_gData.s_uniforms.m_roughness   = settings.m_roughness;
        s_gData.s_uniforms.m_reflectance = settings.m_reflectance;
        s_gData.s_uniforms.m_lightPtr    = &pointLight;

        // Setup uniforms.
        
        s_gData.s_uniforms.m_albedo = glm::vec4(
            powf(settings.m_diffColor[0], 2.2f),
            powf(settings.m_diffColor[1], 2.2f),
            powf(settings.m_diffColor[2], 2.2f),
            1.0f);
        
        // Clear color (maps to 1 after tonemapping)
        s_gData.s_uniforms.m_color = vec4(25.7f, 25.7f, 25.7f, 1.0f);

        float lightMtx[16];
        s_gData.s_uniforms.setPtrs(&pointLight, lightMtx);

        // Grab camera position
        glm::vec4 viewPos;
        cameraGetPosition(&viewPos[0]);
        
        // Compute transform matrices.
        float screenProj[16];
        float screenView[16];
        bx::mtxIdentity(screenView);
        bx::mtxOrtho(screenProj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

        float proj[16];
        memcpy(proj, viewState.m_proj, sizeof(proj));
        bgfx::setViewTransform(0, viewState.m_view, proj);
 
        // Setup views and render targets.
        bgfx::setViewRect(0, 0, 0, viewState.m_width, viewState.m_height);

        // Clear backbuffer at beginning.
        bgfx::setViewClear(0
                           , BGFX_CLEAR_COLOR
                           | BGFX_CLEAR_DEPTH
                           , clearValues.m_clearRgba
                           , clearValues.m_clearDepth
                           , clearValues.m_clearStencil
                           );
        bgfx::touch(0);

        uint8_t passViewID = RENDERVIEW_DRAWSCENE_0_ID;

        {
            Halton4D(s_gData.s_uniforms.m_samples, int(settings.m_sampleCount));
            s_gData.s_uniforms.m_sampleCount = (float)settings.m_sampleCount;
            s_gData.s_uniforms.submitPerFrameUniforms(viewPos);
            
            // Set the jittered projection matrix
            memcpy(proj, viewState.m_proj, sizeof(proj));
            jitterProjMatrix(proj, settings.m_sampleCount/NUM_SAMPLES,
                             settings.m_jitterAASigma,
                             (float)viewState.m_width, (float)viewState.m_height);

            bgfx::setViewTransform(passViewID, viewState.m_view, proj);
            
            bgfx::setViewFrameBuffer(passViewID, s_rtColorBuffer);
            bgfx::setViewRect(passViewID, 0, 0, viewState.m_width, viewState.m_height);
            
            uint16_t flagsRT = BGFX_CLEAR_DEPTH;
            if (settings.m_sampleCount == 0)
            {
                flagsRT |= BGFX_CLEAR_COLOR;
            }
            
            // Clear offscreen RT
            bgfx::setViewClear(passViewID
                               , flagsRT
                               , clearValues.m_clearRgba
                               , clearValues.m_clearDepth
                               , clearValues.m_clearStencil
                               );
            bgfx::touch(passViewID);
            
            // Render Depth
            for (uint64_t renderIdx = 0; renderIdx < rlist.count; ++renderIdx)
            {
                rlist.models[renderIdx].submit(s_gData, passViewID,
                                               s_programs.m_packDepth, LightColorMaps(0), s_renderStates[RenderState::ZPass]);
            }
            
            for (uint64_t renderIdx = 0; renderIdx < llist.count; ++renderIdx)
            {
                glm::mat4 lightTransform = SetLightUniforms(lsettings[renderIdx]);
                llist.models[renderIdx].transform = lightTransform;
                LightMaps colorMaps = LightColorMaps(lsettings[renderIdx].textureIdx);
        
                llist.models[renderIdx].submit(s_gData, passViewID,
                                               s_programs.m_packDepthLight, colorMaps, s_renderStates[RenderState::ZTwoSidedPass]);
            }
            
            // Render passes
            for (int renderPassIdx = 0; renderPassIdx < 2; renderPassIdx++)
            {
                // Render scene once for each light
                for (int lightPassIdx = 0; lightPassIdx < llist.count; ++lightPassIdx)
                {
                    const LightData& light = lsettings[lightPassIdx];
                    
                    // Setup light
                    SetLightUniforms(light);
                    LightMaps colorMaps = LightColorMaps(light.textureIdx);
        
                    bool diffuse = (renderPassIdx == 0);
                    bgfx::ProgramHandle progDraw = GetLightProgram(diffuse, settings.m_useGT);
                    
                    // Render scene models
                    for (uint64_t renderIdx = 0; renderIdx < rlist.count; ++renderIdx)
                    {
                        rlist.models[renderIdx].submit(s_gData, passViewID,
                                                       progDraw, colorMaps, s_renderStates[RenderState::ColorPass]);
                    }
                }
            }
            
            // render light objects
            for (uint64_t renderIdx = 0; renderIdx < llist.count; ++renderIdx)
            {
                // setup light
                glm::mat4 lightTransform = SetLightUniforms(lsettings[renderIdx]);
                LightMaps colorMaps = LightColorMaps(lsettings[renderIdx].textureIdx);
                
                llist.models[renderIdx].transform = lightTransform;
                llist.models[renderIdx].submit(s_gData, passViewID,
                                               s_programs.m_colorTextured, colorMaps,
                                               s_renderStates[RenderState::ColorAlphaPass]);
            }
            
            settings.m_sampleCount += NUM_SAMPLES;
            ++passViewID;
        }

        // Blit pass
        uint8_t blitID = passViewID;
        bgfx::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
        bgfx::setViewFrameBuffer(blitID, handle);
        bgfx::setViewRect(blitID, 0, 0, viewState.m_width, viewState.m_height);
        bgfx::setViewTransform(blitID, screenView, screenProj);
        
        bgfx::setTexture(2, s_gData.s_uColorMap, s_rtColorBuffer);
        bgfx::setState(BGFX_STATE_RGB_WRITE|BGFX_STATE_ALPHA_WRITE);
        screenSpaceQuad(s_flipV);
        bgfx::submit(blitID, s_programs.m_blit);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();
    }

    // shutdown
    PrimitiveDemo::shutDown();
    SponzaDemo::shutDown();

    bgfx::destroyTexture(s_gData.s_texLTCMat);
    bgfx::destroyTexture(s_gData.s_texLTCAmp);

    s_gData.s_texStainedGlassMaps.destroyTextures();
    s_gData.s_texWhiteMaps.destroyTextures();

    bgfx::destroyFrameBuffer(s_rtColorBuffer);

    s_programs.destroy();

    bgfx::destroyUniform(s_gData.s_uLTCMat);
    bgfx::destroyUniform(s_gData.s_uLTCAmp);
    bgfx::destroyUniform(s_gData.s_uColorMap);
    bgfx::destroyUniform(s_gData.s_uFilteredMap);

    s_gData.s_uniforms.destroy();

    cameraDestroy();
    imguiDestroy();

    // Shutdown bgfx.
    bgfx::shutdown();

    return 0;
}