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

#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gCharacterMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;
Mesh* gFloorMesh;
Mesh* gTeapotMesh;
Mesh* gSphereMesh;
Mesh* gCubeMesh;

Model* gCharacter;
Model* gCrate;
Model* gGround;
Model* gFloor;
Model* gTeapot;
Model* gSphere;
Model* gCube;
Model* gAdditiveCube;
Model* gMultiplicativeCube;
Model* gAlphaCube;
Model* gMoogleCube;

Camera* gCamera;


// Store lights in an array in this exercise
const int NUM_LIGHTS = 5;
struct Light
{
    Model*   model;
    CVector3 colour;
    float    strength;
};
Light gLights[NUM_LIGHTS]; 


// Additional light information
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

float gWiggle = 6.0f;

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;
const float gLight3Strength = 30.0f;

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



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
ID3D11Resource*           gCharacterDiffuseSpecularMap    = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11ShaderResourceView* gCharacterDiffuseSpecularMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Resource*           gCrateDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gGroundDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gLightDiffuseMap    = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;

ID3D11Resource*           gFloorDiffuseMap = nullptr;
ID3D11ShaderResourceView* gFloorDiffuseMapSRV = nullptr;

ID3D11Resource*           gTeapotDiffuseMap = nullptr;
ID3D11ShaderResourceView* gTeapotDiffuseMapSRV = nullptr;

ID3D11Resource*           gFadeOneDiffuseMap = nullptr;
ID3D11ShaderResourceView* gFadeOneDiffuseMapSRV = nullptr;

ID3D11Resource*           gFadeTwoDiffuseMap = nullptr;
ID3D11ShaderResourceView* gFadeTwoDiffuseMapSRV = nullptr;

ID3D11Resource*           gGlassDiffuseMap = nullptr;
ID3D11ShaderResourceView* gGlassDiffuseMapSRV = nullptr;

ID3D11Resource*           gSmokeDiffuseMap = nullptr;
ID3D11ShaderResourceView* gSmokeDiffuseMapSRV = nullptr;

ID3D11Resource*           gMoogleDiffuseMap = nullptr;
ID3D11ShaderResourceView* gMoogleDiffuseMapSRV = nullptr;

// Light Helper functions
CMatrix4x4 CalculateLightViewMatrix(int lightIndex)
{
    return InverseAffine(gLights[lightIndex].model->WorldMatrix());
}

// Get "camera-like" projection matrix for a spotlight
CMatrix4x4 CalculateLightProjectionMatrix(int lightIndex)
{
    return MakeProjectionMatrix(1.0f, ToRadians(gSpotlightConeAngle)); // Helper function in Utility\GraphicsHelpers.cpp
}


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
        gCharacterMesh = new Mesh("Man.x");
        gCrateMesh     = new Mesh("CargoContainer.x");
        gGroundMesh    = new Mesh("Hills.x");
        gLightMesh     = new Mesh("Light.x");
        gFloorMesh     = new Mesh("Floor.x");
        gTeapotMesh    = new Mesh("Teapot.x");
        gSphereMesh    = new Mesh("Sphere.x");
        gCubeMesh      = new Mesh("Cube.x");
    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }


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
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }


    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    // The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
    // texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
    // The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
    if (!LoadTexture("ManDiffuseSpecular.dds",   &gCharacterDiffuseSpecularMap, &gCharacterDiffuseSpecularMapSRV) ||
        !LoadTexture("CargoA.dds",               &gCrateDiffuseSpecularMap,     &gCrateDiffuseSpecularMapSRV    ) ||
        !LoadTexture("GrassDiffuseSpecular.dds", &gGroundDiffuseSpecularMap,    &gGroundDiffuseSpecularMapSRV   ) ||
        !LoadTexture("Flare.jpg",                &gLightDiffuseMap,             &gLightDiffuseMapSRV            ) ||
        !LoadTexture("Wood2.jpg",                &gFloorDiffuseMap,             &gFloorDiffuseMapSRV            ) ||
        !LoadTexture("tech02.jpg",               &gFadeOneDiffuseMap,           &gFadeOneDiffuseMapSRV          ) ||
        !LoadTexture("Wood2.jpg",                &gFadeTwoDiffuseMap,           &gFadeTwoDiffuseMapSRV          ) ||
        !LoadTexture("Glass.jpg",                &gGlassDiffuseMap,             &gGlassDiffuseMapSRV            ) ||
        !LoadTexture("Smoke.png",                &gSmokeDiffuseMap,             &gSmokeDiffuseMapSRV            ) ||
        !LoadTexture("Moogle.png",               &gMoogleDiffuseMap,            &gMoogleDiffuseMapSRV           ) ||
        !LoadTexture("tech02.jpg",               &gTeapotDiffuseMap,            &gTeapotDiffuseMapSRV))
    {
        gLastError = "Error loading textures";
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

    gCharacter          = new Model(gCharacterMesh);
    gCrate              = new Model(gCrateMesh);
    gGround             = new Model(gGroundMesh);
    gFloor              = new Model(gFloorMesh);
    gTeapot             = new Model(gTeapotMesh);
    gSphere             = new Model(gSphereMesh);
    gCube               = new Model(gCubeMesh);
    gAdditiveCube       = new Model(gCubeMesh);
    gMultiplicativeCube = new Model(gCubeMesh);
    gAlphaCube          = new Model(gCubeMesh);
    gMoogleCube         = new Model(gCubeMesh);

	// Initial positions
	//gCharacter->SetPosition({ 25, 0.5, 10 });
    //gCharacter->SetScale(0.06f);
    //gCharacter->SetRotation({ 0.0f, ToRadians(140.0f), 0.0f });

    // Position and rotate whole character
    gCharacter->SetPosition({ 45, 16, 45 });
    gCharacter->SetRotation({ ToRadians(0.0f), ToRadians(220.0f), ToRadians(90.0f) });
    gCharacter->SetScale(0.06f);

    gCharacter->SetRotation({ ToRadians(0), ToRadians(-90), ToRadians(90) }, 6); // Right Lower Arm
    gCharacter->SetRotation({ ToRadians(0), ToRadians(0), ToRadians(30) }, 5); // Right Upper Arm
    gCharacter->SetRotation({ ToRadians(0), ToRadians(5), ToRadians(0) }, 40); // Right Upper Leg
    gCharacter->SetRotation({ ToRadians(107374176.), ToRadians(-185), ToRadians(0) }, 36); // Right Lower Leg
    gCharacter->SetRotation({ ToRadians(0), ToRadians(30), ToRadians(0) }, 3);  // Upper Torso
    gCharacter->SetRotation({ ToRadians(0), ToRadians(-50), ToRadians(70) }, 7); // Right Hand
    gCharacter->SetRotation({ ToRadians(-90), ToRadians(0), ToRadians(0) }, 19); // Left Upper Arm

	gCrate->SetPosition({ 45, 0, 45 });
	gCrate->SetScale( 6.0f );
	gCrate->SetRotation({ 0.0f, ToRadians(-50.0f), 0.0f });

    gFloor->SetPosition({ -300, 0, 0 });

    // Light set-up - using an array this time
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLights[i].model = new Model(gLightMesh);
    }
    
    gLights[0].colour = { 0.8f, 0.8f, 1.0f };
    gLights[0].strength = 10;
    gLights[0].model->SetPosition({ 30, 10, 0 });
    gLights[0].model->SetScale(pow(gLights[0].strength, 0.7f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.
    
    gLights[1].colour = { 1.0f, 0.8f, 0.2f };
    gLights[1].strength = 40;
    gLights[1].model->SetPosition({ -10, 25, -30 });
    gLights[1].model->SetScale(pow(gLights[1].strength, 0.7f));

    gLights[2].colour = { 1.0f, 0.0f, 0.0f };
    gLights[2].strength = gLight3Strength;
    gLights[2].model->SetPosition({ -310, 10, 0 });
    gLights[2].model->SetScale(pow(gLights[2].strength, 0.7f));

    gLights[3].colour = { 0.2f, 0.5f, 8.0f };
    gLights[3].strength = 30;
    gLights[3].model->SetPosition({ -310, 25, 30 });
    gLights[3].model->SetScale(pow(gLights[3].strength, 0.7f));

    gLights[4].colour = { 0.75f, 0.75f, 0.75f };
    gLights[4].strength = 40;
    gLights[4].model->SetPosition({ -20, 40, -50 });
    gLights[4].model->SetScale(pow(gLights[4].strength, 0.7f));

    gTeapot->SetPosition({ -340, 0, 0 });

    gSphere->SetPosition({ -300, 10, 30 });
    gSphere->SetScale(.5f);

    gCube->SetPosition({ -320, 10, 60 });

    gAdditiveCube->SetPosition({ -320, 10, 80 });

    gMultiplicativeCube->SetPosition({ -320, 10, 100 });

    gAlphaCube->SetPosition({ -320, 10, 120 });

    gMoogleCube->SetPosition({ -320, 10, 140 });



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

    if (gShadowMap1DepthStencil)  gShadowMap1DepthStencil->Release();
    if (gShadowMap1SRV)           gShadowMap1SRV->Release();
    if (gShadowMap1Texture)       gShadowMap1Texture->Release();

    if (gLightDiffuseMapSRV)             gLightDiffuseMapSRV->Release();
    if (gLightDiffuseMap)                gLightDiffuseMap->Release();
    if (gGroundDiffuseSpecularMapSRV)    gGroundDiffuseSpecularMapSRV->Release();
    if (gGroundDiffuseSpecularMap)       gGroundDiffuseSpecularMap->Release();
    if (gCrateDiffuseSpecularMapSRV)     gCrateDiffuseSpecularMapSRV->Release();
    if (gCrateDiffuseSpecularMap)        gCrateDiffuseSpecularMap->Release();
    if (gCharacterDiffuseSpecularMapSRV) gCharacterDiffuseSpecularMapSRV->Release();
    if (gCharacterDiffuseSpecularMap)    gCharacterDiffuseSpecularMap->Release();
    if (gFloorDiffuseMapSRV)             gFloorDiffuseMapSRV->Release();
    if (gFloorDiffuseMap)                gFloorDiffuseMap->Release();
    if (gTeapotDiffuseMapSRV)            gTeapotDiffuseMapSRV->Release();
    if (gTeapotDiffuseMap)               gTeapotDiffuseMap->Release();
    if (gFadeOneDiffuseMap)              gFadeOneDiffuseMap->Release();
    if (gFadeOneDiffuseMapSRV)           gFadeOneDiffuseMapSRV->Release();
    if (gFadeTwoDiffuseMap)              gFadeTwoDiffuseMap->Release();
    if (gFadeTwoDiffuseMapSRV)           gFadeTwoDiffuseMapSRV->Release();
    if (gGlassDiffuseMap)                gGlassDiffuseMap->Release();
    if (gGlassDiffuseMapSRV)             gGlassDiffuseMapSRV->Release();
    if (gSmokeDiffuseMap)                gSmokeDiffuseMap->Release();
    if (gSmokeDiffuseMapSRV)             gSmokeDiffuseMapSRV->Release();
    if (gMoogleDiffuseMap)               gMoogleDiffuseMap->Release();
    if (gMoogleDiffuseMapSRV)            gMoogleDiffuseMapSRV->Release();


    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        delete gLights[i].model;  gLights[i].model = nullptr;
    }
    delete gCamera;                   gCamera             = nullptr;
    delete gGround;                   gGround             = nullptr;
    delete gCrate;                    gCrate              = nullptr;
    delete gCharacter;                gCharacter          = nullptr;
    delete gFloor;                    gFloor              = nullptr;
    delete gTeapot;                   gTeapot             = nullptr;
    delete gSphere;                   gSphere             = nullptr;
    delete gCube;                     gCube               = nullptr;
    delete gAdditiveCube;             gAdditiveCube       = nullptr;
    delete gMultiplicativeCube;       gMultiplicativeCube = nullptr;
    delete gAlphaCube;                gAlphaCube          = nullptr;
    delete gMoogleCube;               gMoogleCube         = nullptr;

    delete gLightMesh;      gLightMesh     = nullptr;
    delete gGroundMesh;     gGroundMesh    = nullptr;
    delete gCrateMesh;      gCrateMesh     = nullptr;
    delete gCharacterMesh;  gCharacterMesh = nullptr;
    delete gFloorMesh;      gFloorMesh     = nullptr;
    delete gTeapotMesh;     gTeapotMesh    = nullptr;
    delete gSphereMesh;     gSphereMesh    = nullptr;
    delete gCubeMesh;       gCubeMesh      = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

void RenderDepthBufferFromLight(int lightIndex)
{
    // Get camera-like matrices from the spotlight, seet in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix = CalculateLightViewMatrix(lightIndex);
    gPerFrameConstants.projectionMatrix = CalculateLightProjectionMatrix(lightIndex);
    gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Only render models that cast shadows ////

    // Use special depth-only rendering shaders
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gDepthOnlyPixelShader, nullptr, 0);

    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullFrontState);

    // Render models - no state changes required between each object in this situation (no textures used in this step)
    gGround->Render();
    gCharacter->Render();
    gCrate->Render();
    gFloor->Render();
    gTeapot->Render();
    gSphere->Render();
    gCube->Render();
    gAdditiveCube->Render();
    gMultiplicativeCube->Render();
    gAlphaCube->Render();
    gMoogleCube->Render();
}

// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


    //// Render skinned models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gSkinningVertexShader,     nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gCharacterDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    gCharacter->Render();


    //// Render non-skinned models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0); // Only need to change the vertex shader from skinning
    

    // Render lit models, only change textures for each onee
    gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
    gGround->Render();
    

    gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV);
    gCrate->Render();

    gD3DContext->PSSetShaderResources(0, 1, &gFloorDiffuseMapSRV);
    gFloor->Render();

    gD3DContext->PSSetShaderResources(0, 1, &gTeapotDiffuseMapSRV);
    gTeapot->Render();

    gD3DContext->PSSetShader(gTextureFadePixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gFadeOneDiffuseMapSRV);
    gD3DContext->PSSetShaderResources(2, 1, &gFadeTwoDiffuseMapSRV);
    gCube->Render();

    gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    gSphere->Render();

    // Render Additive cube
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV);
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);
    gAdditiveCube->Render();

    // Render Multiplicative cube
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gSimplePixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gGlassDiffuseMapSRV);
    gD3DContext->OMSetBlendState(gMultiplicativeBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);
    gMultiplicativeCube->Render();

    // Render Alpha Cube
    gD3DContext->PSSetShaderResources(0, 1, &gSmokeDiffuseMapSRV);
    gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);
    gAlphaCube->Render();

    gD3DContext->PSSetShaderResources(0, 1, &gMoogleDiffuseMapSRV);
    gMoogleCube->Render();

    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling (standard set-up for blending
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the array
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLights[i].colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLights[i].model->Render();
    }
}




// Rendering the scene
void RenderScene()
{
    //// Common settings ////

    // Set up the light information in the constant buffer
    // Don't send to the GPU yet, the function RenderSceneFromCamera will do that

    //Light 1 as spotlight
    gPerFrameConstants.light1Colour   = gLights[0].colour * gLights[0].strength;
    gPerFrameConstants.light1Position = gLights[0].model->Position();
    gPerFrameConstants.light1Facing = Normalise(gLights[0].model->WorldMatrix().GetZAxis());
    gPerFrameConstants.light1CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2));
    gPerFrameConstants.light1ViewMatrix = CalculateLightViewMatrix(0);
    gPerFrameConstants.light1ProjectionMatrix = CalculateLightProjectionMatrix(0);
    gPerFrameConstants.light1Type = 1;


    gPerFrameConstants.light2Colour   = gLights[1].colour * gLights[1].strength;
    gPerFrameConstants.light2Position = gLights[1].model->Position();
    gPerFrameConstants.light2Facing = Normalise(gLights[1].model->WorldMatrix().GetZAxis());
    gPerFrameConstants.light2CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2));
    gPerFrameConstants.light2ViewMatrix = CalculateLightViewMatrix(1);
    gPerFrameConstants.light2ProjectionMatrix = CalculateLightProjectionMatrix(1);
    gPerFrameConstants.light2Type = 0;


    gPerFrameConstants.light3Colour   = gLights[2].colour * gLights[2].strength;
    gPerFrameConstants.light3Position = gLights[2].model->Position();
    gPerFrameConstants.light3Facing = Normalise(gLights[2].model->WorldMatrix().GetZAxis());
    gPerFrameConstants.light3CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2));
    gPerFrameConstants.light3ViewMatrix = CalculateLightViewMatrix(2);
    gPerFrameConstants.light3ProjectionMatrix = CalculateLightProjectionMatrix(2);
    gPerFrameConstants.light3Type = 0;


    gPerFrameConstants.light4Colour   = gLights[3].colour * gLights[3].strength;
    gPerFrameConstants.light4Position = gLights[3].model->Position();
    gPerFrameConstants.light4Facing = Normalise(gLights[3].model->WorldMatrix().GetZAxis());
    gPerFrameConstants.light4CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2));
    gPerFrameConstants.light4ViewMatrix = CalculateLightViewMatrix(3);
    gPerFrameConstants.light4ProjectionMatrix = CalculateLightProjectionMatrix(3);
    gPerFrameConstants.light4Type = 0;

    gPerFrameConstants.light4Colour = gLights[4].colour * gLights[4].strength;
    gPerFrameConstants.light4Position = gLights[4].model->Position();
    gPerFrameConstants.light4Facing = Normalise(gLights[4].model->WorldMatrix().GetZAxis());
    gPerFrameConstants.light4CosHalfAngle = cos(ToRadians(gSpotlightConeAngle / 2));
    gPerFrameConstants.light4ViewMatrix = CalculateLightViewMatrix(4);
    gPerFrameConstants.light4ProjectionMatrix = CalculateLightProjectionMatrix(4);
    gPerFrameConstants.light4Type = 2;

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();

    // Render from light's point of view 
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(gShadowMapSize);
    vp.Height = static_cast<FLOAT>(gShadowMapSize);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);


    // Select the shadow map texture as the current depth buffer. We will not be rendering any pixel colours
    // Also clear the the shadow map depth buffer to the far distance
    gD3DContext->OMSetRenderTargets(0, nullptr, gShadowMap1DepthStencil);
    gD3DContext->ClearDepthStencilView(gShadowMap1DepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the scene from the point of view of light 1 (only depth values written)
    RenderDepthBufferFromLight(0);
    RenderDepthBufferFromLight(1);
    RenderDepthBufferFromLight(2);
    RenderDepthBufferFromLight(3);
    RenderDepthBufferFromLight(4);

    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    // When finished the back buffer is sent to the "front buffer" - which is the monitor.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    
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

    // Render the scene from the main camera
    RenderSceneFromCamera(gCamera);

    // Unbind shadow maps
    ID3D11ShaderResourceView* nullView = nullptr;
    gD3DContext->PSSetShaderResources(1, 1, &nullView);

    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------
void LightControl(float frameTime);

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
    gCharacter->Control(20, frameTime, Key_0, Key_0, Key_0, Key_0, Key_U, Key_O, Key_I, Key_0); // Wave

	// Control character part. First parameter is node number - index from flattened depth-first array of model parts. 0 is root
	gCharacter->Control(33, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_I, Key_0); // Head
	gCharacter->Control(37, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_T, Key_0); // Left Lower Leg
	gCharacter->Control(41, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_T, Key_0); // Right Lower Leg
	gCharacter->Control(6,  frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_Z, Key_0); // Right Lower Arm
	gCharacter->Control(20, frameTime, Key_0, Key_0, Key_0, Key_0, Key_0, Key_0, Key_Z, Key_0); // Left Lower Arm

    LightControl(frameTime);

    gPerFrameConstants.wiggle += sin(gWiggle * 6);


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


void LightControl(float frameTime)
{
    /**** Light 1 ****/
    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
    static float rotate = 0.0f;
    static bool go = true;
    gLights[0].model->SetPosition(gCharacter->Position() + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit });
    if (go)  rotate -= gLightOrbitSpeed * frameTime;
    if (KeyHit(Key_1))  go = !go;

    /**** Light 3 ****/
    if (gLights[2].strength >= 0.0f && gLights[2].strength <= gLight3Strength)
    {
        gLights[2].strength -= 15.0f * frameTime;
    }
    else
    {
        gLights[2].strength = gLight3Strength;
    }

    /**** Light 4 ****/
   if (gLights[3].colour.z >= 0 && gLights[3].colour.z <= 1)
   {
       float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // Gets a random number between 0 and 1
       gLights[3].colour.z += random * frameTime;
   }
   else
   {
       gLights[3].colour.z = 0;
   }
   
   if (gLights[3].colour.y >= 0 && gLights[3].colour.y <= 1)
   {
       float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
       gLights[3].colour.y += random * frameTime;
   }
   else
   {
       gLights[3].colour.y = 0;
   }
   
   if (gLights[3].colour.x >= 0 && gLights[3].colour.x <= 1)
   {
       float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
       gLights[3].colour.x += random * frameTime;
   }
   else
   {
       gLights[3].colour.x = 0;
   }
}