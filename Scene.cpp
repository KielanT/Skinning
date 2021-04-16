//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include "Light.h"
#include "CModel.h"
#include "CTexture.h"

#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Post Processing types
enum class PostProcess
{
    None,
    Tint,
    GreyNoise,
    Burn,
    Distort,
    Spiral,
};

auto gCurrentPostProcess = PostProcess::None; // Sets the default process to none

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)

// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene

Camera* gCamera;

const int NUM_CHARACTERS = 1;
const int NUM_CUBES = 5;

CModel* gCharacters[NUM_CHARACTERS];
CModel* gCubes[NUM_CUBES];

CModel* gCrate;
CModel* gGround;
CModel* gFloor;
CModel* gTeapot;
CModel* gSphere;
CModel* gMyCar;

// Need to be done the old way due to limitations/bugs with the CModel class
Mesh* gCubeMesh;
Mesh* gTeapotMesh;
Mesh* gTrollMesh;
Mesh* gSphereMesh;

Model* gNormalMapCube;
Model* gParallaxTeapot;
Model* gTroll;

Model* gSkyBox;

// I made a cube in blender, UV unwrapped it and set up cube mapping in there instead of code
Mesh*  gMySkyBoxMesh;
Model* gMySkyBox;


// Store lights in an array in this exercise
const int NUM_LIGHTS = 5;
Light* gLight[NUM_LIGHTS];

// Additional light information
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

float gWiggle = 6.0f;

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

CVector3 OutlineColour = { 0, 0, 0 };
float    OutlineThickness = 0.03f;

float gSpotlightConeAngle = 90.0f;

// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;


// Shadow Texture
int gShadowMapSize = 2048;  // Quality of shadow map

ID3D11Texture2D*          gShadowMap1Texture      = nullptr;
ID3D11DepthStencilView*   gShadowMap1DepthStencil = nullptr;
ID3D11ShaderResourceView* gShadowMap1SRV          = nullptr;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--

// Post processing constants
PostProcessingConstants gPostProcessingConstants;
ID3D11Buffer* gPostProcessingConstantBuffer;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
CTexture* gTrollTexture;
CTexture* gManTexture;
CTexture* gPatternTexture;
CTexture* gPatternNormalTexture;
CTexture* gPatternHeightTexture;
CTexture* gCubeMapTexture;
CTexture* gSkyBoxTexture;

float gParallaxDepth = 0.3f;

// Post Processing textures

ID3D11Texture2D* gSceneTexture = nullptr;
ID3D11RenderTargetView* gSceneRenderTarget = nullptr;
ID3D11ShaderResourceView* gSceneTextureSRV = nullptr;

//CTexture* gNoiseTexture;
//CTexture* gBurnTexture;
//CTexture* gDistortTexture;

ID3D11Resource* gNoiseMap = nullptr;
ID3D11ShaderResourceView* gNoiseMapSRV = nullptr;
ID3D11Resource* gBurnMap = nullptr;
ID3D11ShaderResourceView* gBurnMapSRV = nullptr;
ID3D11Resource* gDistortMap = nullptr;
ID3D11ShaderResourceView* gDistortMapSRV = nullptr;

//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    // Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
    try 
    {
        gMySkyBoxMesh = new Mesh("MySkyBox.fbx");
        gSphereMesh = new Mesh("Sphere.x");
        gCubeMesh = new Mesh("cube.x", true);
        gTeapotMesh = new Mesh("teapot.x", true);
        gTrollMesh = new Mesh("troll.x");
    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }

    //// Load / prepare textures on the GPU ////

    if (!LoadTexture("Noise.png", &gNoiseMap, &gNoiseMapSRV) ||
        !LoadTexture("Burn.png", &gBurnMap, &gBurnMapSRV) ||
        !LoadTexture("Distort.png", &gDistortMap, &gDistortMapSRV))
    {
        gLastError = "Error loading textures";
        return false;
    }

    // Load textures and create DirectX objects for them
    gTrollTexture         = new CTexture("Green.png", "CellGradient.png");
    gManTexture           = new CTexture("ManDiffuseSpecular.dds");
    gPatternTexture       = new CTexture("PatternDiffuseSpecular.dds");
    gPatternNormalTexture = new CTexture("PatternNormal.dds");
    gPatternHeightTexture = new CTexture("PatternNormalHeight.dds");
    gCubeMapTexture       = new CTexture("CubeMap.dds");
    gSkyBoxTexture        = new CTexture("MyCubeMap.png");
    //gNoiseTexture         = new CTexture("Noise.png");
    //gBurnTexture          = new CTexture("Burn.png");
    //gDistortTexture       = new CTexture("Distort.png");

    // Load the shaders required for the geometry we will use (see Shader.cpp / .h)
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }

    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    // These allow us to pass data from CPU to shaders such as lighting information or matrices
    // See the comments above where these variable are declared and also the UpdateScene function
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    gPostProcessingConstantBuffer = CreateConstantBuffer(sizeof(gPostProcessingConstants));
    if (gPerFrameConstantBuffer       == nullptr || gPerModelConstantBuffer == nullptr ||
        gPostProcessingConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }

    // Create Scene Texture
    D3D11_TEXTURE2D_DESC sceneTextureDesc = {};
    sceneTextureDesc.Width = gViewportWidth;  
    sceneTextureDesc.Height = gViewportHeight;
    sceneTextureDesc.MipLevels = 1; 
    sceneTextureDesc.ArraySize = 1;
    sceneTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
    sceneTextureDesc.SampleDesc.Count = 1;
    sceneTextureDesc.SampleDesc.Quality = 0;
    sceneTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    sceneTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    sceneTextureDesc.CPUAccessFlags = 0;
    sceneTextureDesc.MiscFlags = 0;
    if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gSceneTexture)))
    {
        gLastError = "Error creating scene texture";
        return false;
    }

    if (FAILED(gD3DDevice->CreateRenderTargetView(gSceneTexture, NULL, &gSceneRenderTarget)))
    {
        gLastError = "Error creating scene render target view";
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
    srDesc.Format = sceneTextureDesc.Format;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = 1;
    if (FAILED(gD3DDevice->CreateShaderResourceView(gSceneTexture, &srDesc, &gSceneTextureSRV)))
    {
        gLastError = "Error creating scene shader resource view";
        return false;
    }

    //**** Create Shadow Map texture ****//

    // We also need a depth buffer to go with our portal
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = gShadowMapSize; // Size of the shadow map determines quality / resolution of shadows
    textureDesc.Height = gShadowMapSize;
    textureDesc.MipLevels = 1; // 1 level, means just the main texture, no additional mip-maps. Usually don't use mip-maps when rendering to textures (or we would have to render every level)
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // The shadow map contains a single 32-bit value [tech gotcha: have to say typeless because depth buffer and shaders see things slightly differently]
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE; // Indicate we will use texture as a depth buffer and also pass it to shaders
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    if (FAILED(gD3DDevice->CreateTexture2D(&textureDesc, NULL, &gShadowMap1Texture)))
    {
        gLastError = "Error creating shadow map texture";
        return false;
    }

    // Create the depth stencil view, i.e. indicate that the texture just created is to be used as a depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // See "tech gotcha" above. The depth buffer sees each pixel as a "depth" float
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = 0;
    if (FAILED(gD3DDevice->CreateDepthStencilView(gShadowMap1Texture, &dsvDesc, &gShadowMap1DepthStencil)))
    {
        gLastError = "Error creating shadow map depth stencil view";
        return false;
    }


    // We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // See "tech gotcha" above. The shaders see textures as colours, so shadow map pixels are not seen as depths
                                           // but rather as "red" floats (one float taken from RGB). Although the shader code will use the value as a depth
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    if (FAILED(gD3DDevice->CreateShaderResourceView(gShadowMap1Texture, &srvDesc, &gShadowMap1SRV)))
    {
        gLastError = "Error creating shadow map shader resource view";
        return false;
    }

    //*****************************//

  	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
    //// Set up scene ////
    for (int i = 0; i < NUM_CHARACTERS; ++i)
    {
        gCharacters[i] = new CModel(gManTexture);
        gCharacters[i]->SetMesh("Man.x");
        gCharacters[i]->SetScale(0.06f);
        gCharacters[i]->SetName("Character" + i);
    }
    gCharacters[0]->SetPosition({ 45, 16, 45 });
    gCharacters[0]->SetRotation({ ToRadians(0.0f), ToRadians(220.0f), ToRadians(90.0f) });
    gCharacters[0]->SetRotation({ ToRadians(0), ToRadians(-90), ToRadians(90) }, 6); // Right Lower Arm
    gCharacters[0]->SetRotation({ ToRadians(0), ToRadians(0), ToRadians(30) }, 5); // Right Upper Arm
    gCharacters[0]->SetRotation({ ToRadians(0), ToRadians(5), ToRadians(0) }, 40); // Right Upper Leg
    gCharacters[0]->SetRotation({ ToRadians(107374176.), ToRadians(-185), ToRadians(0) }, 36); // Right Lower Leg
    gCharacters[0]->SetRotation({ ToRadians(0), ToRadians(30), ToRadians(0) }, 3);  // Upper Torso
    gCharacters[0]->SetRotation({ ToRadians(0), ToRadians(-50), ToRadians(70) }, 7); // Right Hand
    gCharacters[0]->SetRotation({ ToRadians(-90), ToRadians(0), ToRadians(0) }, 19); // Left Upper Arm

    gGround = new CModel("GrassDiffuseSpecular.dds");
    gGround->SetMesh("Hills.x");
    gCrate = new CModel("CargoA.dds");
    gCrate->SetMesh("CargoContainer.x");
    gCrate->SetPosition({ 45, 0, 45 });
    gCrate->SetScale(6.0f);
    gCrate->SetRotation({ 0.0f, ToRadians(-50.0f), 0.0f });

    gFloor = new CModel("Wood2.jpg");
    gFloor->SetMesh("Floor.x");
    gFloor->SetPosition({ -320, 0, 0 });

    gTeapot = new CModel("tech02.jpg");
    gTeapot->SetMesh("Teapot.x");
    gTeapot->SetPosition({ -320, 0, 0 });

    gSphere = new CModel("tech02.jpg");
    gSphere->SetMesh("Sphere.x");
    gSphere->SetPosition({ -320, 10, 30 });
    gSphere->SetScale(.5f);

    for (int i = 0; i < NUM_CUBES; ++i)
    {
        gCubes[i] = new CModel();
    }

    std::vector<std::string> cubeTextures;
    cubeTextures.push_back("Wood2.jpg");
    cubeTextures.push_back("Flare.jpg"); // For some reason it skips over the second one so it needs to have three textures
    cubeTextures.push_back("tech02.jpg");
    gCubes[0]->SetTexture(cubeTextures);
    gCubes[0]->SetPosition({ -320, 10, 60 });
    gCubes[0]->SetName("Cube");

    gCubes[1]->SetTexture("Flare.jpg");
    gCubes[1]->SetPosition({ -320, 10, 80 });
    gCubes[1]->SetName("Additive Cube");

    gCubes[2]->SetTexture("Glass.jpg");
    gCubes[2]->SetPosition({ -320, 10, 100 });
    gCubes[2]->SetName("Multiplicative Cube");

    gCubes[3]->SetTexture("Smoke.png");
    gCubes[3]->SetPosition({ -320, 10, 120 });
    gCubes[3]->SetName("Alpha Cube");

    gCubes[4]->SetTexture("Moogle.png");
    gCubes[4]->SetPosition({ -320, 10, 140 });
    gCubes[4]->SetName("Moogle Cube");

    gNormalMapCube = new Model(gCubeMesh); 
    gNormalMapCube->SetPosition({ -320, 10, 160 });

    gParallaxTeapot = new Model(gTeapotMesh);
    gParallaxTeapot->SetPosition({ -320, 0, -20 });

    gTroll = new Model(gTrollMesh);
    gTroll->SetPosition({ -320, 0, -40 });
    gTroll->SetRotation({ 0, -80, 0 });
    gTroll->SetScale(4.0f);

    gMyCar = new CModel("CarTexture.png");
    gMyCar->SetMesh("MyCar.fbx");
    gMyCar->SetPosition({ -320, 0, 200 });
    gMyCar->SetRotation({ 0, 200, 0 });
    gMyCar->SetScale(.2f);

    gSkyBox = new Model(gSphereMesh);
    gSkyBox->SetPosition({ 0, 0, 0 });

    gMySkyBox = new Model(gMySkyBoxMesh);
    gMySkyBox->SetScale(0.1f);
    //gMySkyBox->SetScale(10.0f); // For use as a skybox
    gMySkyBox->SetPosition({ -320, 10, -60 });
    //gMySkyBox->SetPosition({ 0, 0, 0 }); // For use as a skybox
    
    // Light set-up - using an array this time
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLight[i] = new Light();
    }
    
    gLight[0]->SetStrength(10);
    gLight[0]->SetPosition({ 30, 10, 0 });
    gLight[0]->SetType(1);
    gLight[0]->SetEffect(1);

    gLight[1]->SetLightColour({ 1.0f, 0.8f, 0.2f });
    gLight[1]->SetStrength(60);
    gLight[1]->SetPosition({ -10, 25, -30 });

    gLight[2]->SetLightColour({ 1.0f, 0.0f, 0.0f });
    gLight[2]->SetStrength(50);
    gLight[2]->SetPosition({ -310, 10, 0 });
    gLight[2]->SetEffect(2);

    gLight[3]->SetLightColour({ 0.2f, 0.5f, 8.0f });
    gLight[3]->SetStrength(40);
    gLight[3]->SetPosition({ -310, 25, 30 });
    gLight[3]->SetEffect(3);
    
    gLight[4]->SetLightColour({ 0.75f, 0.75f, 0.75f });
    gLight[4]->SetStrength(40);
    gLight[4]->SetPosition({ -20, 40, -50 });
    gLight[4]->SetType(2);

    srand(static_cast <unsigned> (time(0))); // Used for seeding the random number generator
    //// Set up camera ////

    gCamera = new Camera();
    gCamera->SetPosition({ 25, 12,-10 });
    gCamera->SetRotation({ ToRadians(13.0f), ToRadians(15.0f), 0.0f });

    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    if (gShadowMap1DepthStencil)        gShadowMap1DepthStencil->Release();
    if (gShadowMap1SRV)                 gShadowMap1SRV->Release();
    if (gShadowMap1Texture)             gShadowMap1Texture->Release();

    if (gPerModelConstantBuffer)        gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)        gPerFrameConstantBuffer->Release();
    if (gPostProcessingConstantBuffer)  gPostProcessingConstantBuffer->Release();

    if (gSceneTextureSRV)               gSceneTextureSRV->Release();
    if (gSceneRenderTarget)             gSceneRenderTarget->Release();
    if (gSceneTexture)                  gSceneTexture->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        delete gLight[i];  gLight[i] = nullptr;
    }
    delete gCamera;           gCamera           = nullptr;
    delete gNormalMapCube;    gNormalMapCube    = nullptr;
    delete gParallaxTeapot;   gParallaxTeapot   = nullptr;
    delete gTroll;            gTroll            = nullptr;
    delete gSkyBox;           gSkyBox           = nullptr;

    delete gCubeMesh;         gCubeMesh         = nullptr;
    delete gTeapotMesh;       gTeapotMesh       = nullptr;
    delete gTrollMesh;        gTrollMesh        = nullptr;

    delete gMySkyBox;       gMySkyBox = nullptr;
    delete gMySkyBoxMesh;   gMySkyBoxMesh = nullptr;
}

//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------


// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.cameraMatrix         = camera->WorldMatrix();
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader
    gD3DContext->GSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);
    
    gD3DContext->GSSetShader(nullptr, nullptr, 0); // Turns off the geometry shader
    //// Render skinned models ////
    for (int i = 0; i < NUM_CHARACTERS; ++i)
    {
        gCharacters[i]->SetVSShader(gSkinningVertexShader);
        gCharacters[i]->Render();
    }

    //// Render non-skinned models ////
    gGround->Render();
    
    gCrate->Render();
    
    gFloor->Render();

    gTeapot->Render();

    gMyCar->SetCull(ECullType::None);
    gMyCar->Render();

    gCubes[0]->SetPSShader(gTextureFadePixelShader);

    // Render Additive cube
    gCubes[1]->SetVSShader(gBasicTransformVertexShader);
    gCubes[1]->SetPSShader(gLightModelPixelShader);
    gCubes[1]->SetBlendType(EBlendType::Additive);
    gCubes[1]->SetCull(ECullType::None);

    // Render Multiplicative cube
    gCubes[2]->SetVSShader(gBasicTransformVertexShader);
    gCubes[2]->SetPSShader(gSimplePixelShader);
    gCubes[2]->SetBlendType(EBlendType::Multiplicative);
    gCubes[2]->SetCull(ECullType::None);

    // Render Alpha Cubes
    gCubes[3]->SetVSShader(gBasicTransformVertexShader);
    gCubes[3]->SetPSShader(gSimplePixelShader);
    gCubes[3]->SetBlendType(EBlendType::Alpha);
    gCubes[3]->SetCull(ECullType::None);

    gCubes[4]->SetVSShader(gBasicTransformVertexShader);
    gCubes[4]->SetPSShader(gSimplePixelShader);
    gCubes[4]->SetBlendType(EBlendType::Alpha);
    gCubes[4]->SetCull(ECullType::None);

    for (int i = 0; i < NUM_CUBES; ++i)
    {
        gCubes[i]->Render();
    }

    gD3DContext->VSSetShader(gNormalMapVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gNormalMapPixelShader, nullptr, 0);

    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);
    
    gD3DContext->PSSetShaderResources(0, 1, gPatternTexture->GetTexture()); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetShaderResources(2, 1, gPatternNormalTexture->GetTexture());
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
    gNormalMapCube->Render();

    gD3DContext->VSSetShader(gNormalMapVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gParallaxMapPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(2, 1, gPatternHeightTexture->GetTexture());
    gParallaxTeapot->Render();

    // Render Troll Outline
    gD3DContext->VSSetShader(gCellShadingOutlineVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gCellShadingOutlinePixelShader, nullptr, 0);
    gD3DContext->RSSetState(gCullFrontState);
    gTroll->Render();

    // Render Main Troll
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gCellShadingPixelShader, nullptr, 0);
    gD3DContext->RSSetState(gCullBackState);
    gD3DContext->PSSetShaderResources(0, 1, gTrollTexture->GetTexture());
    gD3DContext->PSSetShaderResources(2, 1, gTrollTexture->GetTexture2());
    gD3DContext->PSSetSamplers(2, 1, &gPointSampler);
    gTroll->Render();

    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader, nullptr, 0);
    gD3DContext->RSSetState(gCullNoneState);
    gD3DContext->PSSetShaderResources(0, 1, gSkyBoxTexture->GetTexture());
    gMySkyBox->Render();

    gPerModelConstants.objectColour = { 1, 1, 0 };
    gSphere->SetPSShader(gTintPixelShader);
    gSphere->SetVSShader(gWiggleVertexShader);
    gSphere->Render();

    //// Render lights ////
    // Render all the lights in the array
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLight[i]->GetLightColour(); // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLight[i]->RenderLightFromCamera();
    }
}

// Run Post Procssing

void PostProcessing(float frameTime)
{

    // Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget /*MISSING, 2nd pass specify back buffer as render target (note: needs an &)*/, gDepthStencil);


    // Give the pixel shader (post-processing shader) access to the scene texture 
    gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRV/* MISSING select the scene texture shader resource view (note: needs an &)*/);
    gD3DContext->PSSetSamplers(0, 1, &gPointSampler); // Use point sampling (no bilinear, trilinear, mip-mapping etc. for most post-processes)


    // Using special vertex shader than creates its own data for a full screen quad
    gD3DContext->VSSetShader(gFullScreenQuadVertexShader, nullptr, 0);
    gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)


    // States - no blending, ignore depth buffer and culling
    gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);


    // No need to set vertex/index buffer (see fullscreen quad vertex shader), just indicate that the quad will be created as a triangle strip
    gD3DContext->IASetInputLayout(NULL); // No vertex data
    gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // Prepare custom settings for current post-process
    if (gCurrentPostProcess == PostProcess::Tint)
    {
        gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);
        //gPostProcessingConstants.tintColour = { 1, 0, 0 };
        gPostProcessingConstants.tintColour = { 1, 1, 0 };//?? FILTER - Make a nice colour*/;
    }


    else if (gCurrentPostProcess == PostProcess::GreyNoise)
    {
        gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

        // Noise scaling adjusts how fine the noise is.
        const float grainSize = 140; // Fineness of the noise grain
        gPostProcessingConstants.noiseScale = { gViewportWidth / grainSize, gViewportHeight / grainSize };

        // The noise offset is randomised to give a constantly changing noise effect (like tv static)
        //gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };
        gPostProcessingConstants.noiseOffset = { Random(2.0f, 10.0f), Random(2.0f, 10.0f)/*FILTER - 2 random UVs please*/ };

        // Give pixel shader access to the noise texture
        gD3DContext->PSSetShaderResources(1, 1, &gNoiseMapSRV);
        gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
    }


    else if (gCurrentPostProcess == PostProcess::Burn)
    {
        gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

        // Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
        const float burnSpeed = 0.2f;
        gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

        // Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
        gD3DContext->PSSetShaderResources(1, 1, &gBurnMapSRV);
        gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
    }


    else if (gCurrentPostProcess == PostProcess::Distort)
    {
        gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

        // Set the level of distortion
        gPostProcessingConstants.distortLevel = 0.03f;

        // Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
        gD3DContext->PSSetShaderResources(1, 1, &gDistortMapSRV);
        gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
    }


    else if (gCurrentPostProcess == PostProcess::Spiral)
    {
        gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);

        static float wiggle = 0.0f;
        const float wiggleSpeed = 1.0f;

        // Set and increase the amount of spiral - use a tweaked cos wave to animate
        gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f);
        wiggle += wiggleSpeed * frameTime;
    }

    UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
    gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);

    // Draw a quad
    gD3DContext->Draw(4/*MISSING - Post-process pass renderes a quad*/, 0);


    // These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
    ID3D11ShaderResourceView* nullSRV = nullptr;
    gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}

// Rendering the scene
void RenderScene(float frameTime)
{
    //// Common settings ////

    // Set up the light information in the constant buffer
    // Don't send to the GPU yet, the function RenderSceneFromCamera will do that

    //Light 1 as spotlight
    gPerFrameConstants.light1Colour           = gLight[0]->GetLightColour();
    gPerFrameConstants.light1Position         = gLight[0]->GetLightPosition();
    gPerFrameConstants.light1Facing           = gLight[0]->GetLightFacing();
    gPerFrameConstants.light1CosHalfAngle     = gLight[0]->GetLightCosHalfAngle();
    gPerFrameConstants.light1ViewMatrix       = gLight[0]->GetLightViewMatrix();
    gPerFrameConstants.light1ProjectionMatrix = gLight[0]->GetLightProjectionMatrix();
    gPerFrameConstants.light1Type             = gLight[0]->GetLightType();

    gPerFrameConstants.light2Colour           = gLight[1]->GetLightColour();
    gPerFrameConstants.light2Position         = gLight[1]->GetLightPosition();
    gPerFrameConstants.light2Facing           = gLight[1]->GetLightFacing();
    gPerFrameConstants.light2CosHalfAngle     = gLight[1]->GetLightCosHalfAngle();
    gPerFrameConstants.light2ViewMatrix       = gLight[1]->GetLightViewMatrix();
    gPerFrameConstants.light2ProjectionMatrix = gLight[1]->GetLightProjectionMatrix();
    gPerFrameConstants.light2Type             = gLight[1]->GetLightType();

    gPerFrameConstants.light3Colour           = gLight[2]->GetLightColour();
    gPerFrameConstants.light3Position         = gLight[2]->GetLightPosition();
    gPerFrameConstants.light3Facing           = gLight[2]->GetLightFacing();
    gPerFrameConstants.light3CosHalfAngle     = gLight[2]->GetLightCosHalfAngle();
    gPerFrameConstants.light3ViewMatrix       = gLight[2]->GetLightViewMatrix();
    gPerFrameConstants.light3ProjectionMatrix = gLight[2]->GetLightProjectionMatrix();
    gPerFrameConstants.light3Type             = gLight[2]->GetLightType();

    gPerFrameConstants.light4Colour           = gLight[3]->GetLightColour();
    gPerFrameConstants.light4Position         = gLight[3]->GetLightPosition();
    gPerFrameConstants.light4Facing           = gLight[3]->GetLightFacing();
    gPerFrameConstants.light4CosHalfAngle     = gLight[3]->GetLightCosHalfAngle();
    gPerFrameConstants.light4ViewMatrix       = gLight[3]->GetLightViewMatrix();
    gPerFrameConstants.light4ProjectionMatrix = gLight[3]->GetLightProjectionMatrix();
    gPerFrameConstants.light4Type             = gLight[3]->GetLightType();

    gPerFrameConstants.light5Colour           = gLight[4]->GetLightColour();
    gPerFrameConstants.light5Position         = gLight[4]->GetLightPosition();
    gPerFrameConstants.light5Facing           = gLight[4]->GetLightFacing();
    gPerFrameConstants.light5CosHalfAngle     = gLight[4]->GetLightCosHalfAngle();
    gPerFrameConstants.light5ViewMatrix       = gLight[4]->GetLightViewMatrix();
    gPerFrameConstants.light5ProjectionMatrix = gLight[4]->GetLightProjectionMatrix();
    gPerFrameConstants.light5Type             = gLight[4]->GetLightType();

    gPerFrameConstants.ambientColour    = gAmbientColour;
    gPerFrameConstants.specularPower    = gSpecularPower;
    gPerFrameConstants.cameraPosition   = gCamera->Position();
    gPerFrameConstants.parallaxDepth    = gParallaxDepth;
    gPerFrameConstants.outlineColour    = OutlineColour;       
    gPerFrameConstants.outlineThickness = OutlineThickness;    

    gPerFrameConstants.viewportWidth = static_cast<float>(gViewportWidth);
    gPerFrameConstants.viewportHeight = static_cast<float>(gViewportHeight);

    gPerFrameConstants.frameTime = frameTime;

    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap1DepthStencil);
    gD3DContext->ClearDepthStencilView(gShadowMap1DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (gCurrentPostProcess != PostProcess::None)
    {
        gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget/*MISSING select scene texture as render target (note: needs &)*/, gDepthStencil);
        gD3DContext->ClearRenderTargetView(gSceneRenderTarget, &gBackgroundColor.r);
    }
    else
    {
        gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);
        gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    }
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light 1 (only depth values written)

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLight[i]->RenderDepthBufferFromLight();
    }
    // Main scene rendering ////

    // Setup the viewport to the size of the main window
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Set shadow map
    gD3DContext->PSSetShaderResources(1, 1, &gShadowMap1SRV);
    gD3DContext->PSSetSamplers(1, 1, &gPointSampler);

    // Set SkyBox Followed Introduction to 3D Game Programming With DirectX 11 but it doesn't work quite right
    gD3DContext->VSSetShader(gCubeMapVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gCubeMapPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, gCubeMapTexture->GetTexture());
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gLessEqualDepthBufferState, 0);
    gD3DContext->RSSetState(gCullNoneState);
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
    //gSkyBox->Render();

    // Render the scene from the main camera
    RenderSceneFromCamera(gCamera);
    

    // Unbind shadow maps
    ID3D11ShaderResourceView* nullView = nullptr;
    gD3DContext->PSSetShaderResources(1, 1, &nullView);

    //// Scene completion ////

    if (gCurrentPostProcess != PostProcess::None)  PostProcessing(frameTime);

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
    if (KeyHit(Key_1))  gCurrentPostProcess = PostProcess::Tint;
    if (KeyHit(Key_2))  gCurrentPostProcess = PostProcess::GreyNoise;
    if (KeyHit(Key_3))  gCurrentPostProcess = PostProcess::Burn;
    if (KeyHit(Key_4))  gCurrentPostProcess = PostProcess::Distort;
    if (KeyHit(Key_5))  gCurrentPostProcess = PostProcess::Spiral;
    if (KeyHit(Key_0))  gCurrentPostProcess = PostProcess::None;

    gCharacters[0]->GetModel()->Control(20, frameTime, Key_0, Key_0, Key_0, Key_0, Key_U, Key_O, Key_I, Key_0); // Wave
    
	// Control character part. First parameter is node number - index from flattened depth-first array of model parts. 0 is root
	gCharacters[0]->GetModel()->Control(33, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_I, Key_0); // Head
	gCharacters[0]->GetModel()->Control(37, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_T, Key_0); // Left Lower Leg
	gCharacters[0]->GetModel()->Control(41, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_T, Key_0); // Right Lower Leg
	gCharacters[0]->GetModel()->Control(6,  frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_Z, Key_0); // Right Lower Arm
	gCharacters[0]->GetModel()->Control(20, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_Z, Key_0); // Left Lower Arm

    gPerFrameConstants.wiggle += sin(gWiggle * 6);

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLight[i]->UpdateScene(frameTime, gCharacters[0]->GetModel());
    }

	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );

    // Toggle FPS limiting
    if (KeyHit(Key_P))  lockFPS = !lockFPS;

    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float totalFrameTime = 0;
    static int frameCount = 0;
    totalFrameTime += frameTime;
    ++frameCount;
    if (totalFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
        float avgFrameTime = totalFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "Graphics Assignment - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        totalFrameTime = 0;
        frameCount = 0;
    }
}

