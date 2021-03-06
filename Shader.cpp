//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------

#include "Shader.h"
#include <fstream>
#include <vector>
#include <d3dcompiler.h>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Globals used to keep code simpler, but try to architect your own code in a better way
//**** Update Shader.h if you add things here ****//

// Vertex and pixel shader DirectX objects
ID3D11VertexShader*   gPixelLightingVertexShader      = nullptr;
ID3D11PixelShader*    gPixelLightingPixelShader       = nullptr;
ID3D11VertexShader*   gBasicTransformVertexShader     = nullptr;
ID3D11VertexShader*   gSkinningVertexShader           = nullptr; // Skinning is performed in the vertex shader (matrix work), we can use any pixel shader for lighting etc.
ID3D11PixelShader*    gLightModelPixelShader          = nullptr;
ID3D11VertexShader*   gWiggleVertexShader             = nullptr;
ID3D11PixelShader*    gTextureFadePixelShader         = nullptr;
ID3D11PixelShader*    gSimplePixelShader              = nullptr;
ID3D11PixelShader*    gDepthOnlyPixelShader           = nullptr;
ID3D11VertexShader*   gNormalMapVertexShader          = nullptr;
ID3D11PixelShader*    gNormalMapPixelShader           = nullptr;
ID3D11PixelShader*    gParallaxMapPixelShader         = nullptr;
ID3D11VertexShader*   gCellShadingOutlineVertexShader = nullptr;
ID3D11PixelShader*    gCellShadingOutlinePixelShader  = nullptr;
ID3D11PixelShader*    gCellShadingPixelShader         = nullptr;
ID3D11VertexShader*   gCubeMapVertexShader            = nullptr;
ID3D11PixelShader*    gCubeMapPixelShader             = nullptr;
ID3D11PixelShader*    gTintPixelShader                = nullptr;

// Post Procesing shaders
ID3D11VertexShader* gFullScreenQuadVertexShader = nullptr;
ID3D11PixelShader*  gTintPostProcess            = nullptr;
ID3D11PixelShader*  gGreyNoisePostProcess       = nullptr;
ID3D11PixelShader*  gBurnPostProcess            = nullptr;
ID3D11PixelShader*  gDistortPostProcess         = nullptr;
ID3D11PixelShader*  gSpiralPostProcess          = nullptr;


//--------------------------------------------------------------------------------------
// Shader creation / destruction
//--------------------------------------------------------------------------------------

// Load shaders required for this app, returns true on success
bool LoadShaders()
{
    // Shaders must be added to the Visual Studio project to be compiled, they use the extension ".hlsl".
    // To load them for use, include them here without the extension. Use the correct function for each.
    // Ensure you release the shaders in the ShutdownDirect3D function below
    gPixelLightingVertexShader      = LoadVertexShader("PixelLighting_vs"); // Note how the shader files are named to show what type they are
    gPixelLightingPixelShader       = LoadPixelShader ("PixelLighting_ps");
    gBasicTransformVertexShader     = LoadVertexShader("BasicTransform_vs");
    gSkinningVertexShader           = LoadVertexShader("Skinning_vs");
    gLightModelPixelShader          = LoadPixelShader ("LightModel_ps");
    gWiggleVertexShader             = LoadVertexShader("WiggleShader_vs");
    gTextureFadePixelShader         = LoadPixelShader ("TextureFade_ps");
    gSimplePixelShader              = LoadPixelShader ("TextureAlpha_ps");
    gDepthOnlyPixelShader           = LoadPixelShader ("DepthOnly_ps");
    gNormalMapVertexShader          = LoadVertexShader("NormalMapping_vs");
    gNormalMapPixelShader           = LoadPixelShader ("NormalMap_ps");
    gParallaxMapPixelShader         = LoadPixelShader ("ParallaxMapping_ps");
    gCellShadingOutlineVertexShader = LoadVertexShader("CellShadingOutline_vs");
    gCellShadingOutlinePixelShader  = LoadPixelShader ("CellShadingOutline_ps");
    gCellShadingPixelShader         = LoadPixelShader ("CellShading_ps");
    gCubeMapVertexShader            = LoadVertexShader("CubeMap_vs");
    gCubeMapPixelShader             = LoadPixelShader ("CubeMap_ps");
    gTintPixelShader                = LoadPixelShader ("PixelLightingWithTint_ps");
    gFullScreenQuadVertexShader     = LoadVertexShader("FullScreenQuad_pp");
    gTintPostProcess                = LoadPixelShader ("tint_pp");
    gGreyNoisePostProcess           = LoadPixelShader ("GreyNoise_pp");
    gBurnPostProcess                = LoadPixelShader ("Burn_pp");
    gDistortPostProcess             = LoadPixelShader ("Distort_pp");
    gSpiralPostProcess              = LoadPixelShader ("Spiral_pp");

    if (gPixelLightingVertexShader      == nullptr || gPixelLightingPixelShader      == nullptr ||
        gBasicTransformVertexShader     == nullptr || gSkinningVertexShader          == nullptr || 
        gLightModelPixelShader          == nullptr || gWiggleVertexShader            == nullptr ||
        gTextureFadePixelShader         == nullptr || gSimplePixelShader             == nullptr ||
        gDepthOnlyPixelShader           == nullptr || gNormalMapVertexShader         == nullptr ||
        gNormalMapPixelShader           == nullptr || gParallaxMapPixelShader        == nullptr ||
        gCellShadingOutlineVertexShader == nullptr || gCellShadingOutlinePixelShader == nullptr ||
        gCellShadingPixelShader         == nullptr || gCubeMapVertexShader           == nullptr ||
        gCubeMapPixelShader             == nullptr || gTintPixelShader               == nullptr ||
        gFullScreenQuadVertexShader     == nullptr || gTintPostProcess               == nullptr ||
        gGreyNoisePostProcess           == nullptr || gBurnPostProcess               == nullptr ||
        gDistortPostProcess             == nullptr || gSpiralPostProcess             == nullptr)
    {
        gLastError = "Error loading shaders";
        return false;
    }

    return true;
}


void ReleaseShaders()
{
    if (gLightModelPixelShader)             gLightModelPixelShader->Release();
    if (gSkinningVertexShader)              gSkinningVertexShader->Release();
    if (gBasicTransformVertexShader)        gBasicTransformVertexShader->Release();
    if (gPixelLightingPixelShader)          gPixelLightingPixelShader->Release();
    if (gPixelLightingVertexShader)         gPixelLightingVertexShader->Release();
    if (gWiggleVertexShader)                gWiggleVertexShader->Release();
    if (gTextureFadePixelShader)            gTextureFadePixelShader->Release();
    if (gSimplePixelShader)                 gSimplePixelShader->Release();
    if (gDepthOnlyPixelShader)              gDepthOnlyPixelShader->Release();
    if (gNormalMapVertexShader)             gNormalMapVertexShader->Release();
    if (gNormalMapPixelShader)              gNormalMapPixelShader->Release();
    if (gParallaxMapPixelShader)            gParallaxMapPixelShader->Release();
    if (gCellShadingOutlineVertexShader)    gCellShadingOutlineVertexShader->Release();
    if (gCellShadingOutlinePixelShader)     gCellShadingOutlinePixelShader->Release();
    if (gCellShadingPixelShader)            gCellShadingPixelShader->Release();

    if (gCubeMapVertexShader)               gCubeMapVertexShader->Release();
    if (gCubeMapPixelShader)                gCubeMapPixelShader->Release();
    if (gTintPixelShader)                   gTintPixelShader->Release();

    if (gFullScreenQuadVertexShader)        gFullScreenQuadVertexShader->Release();
}



// Load a vertex shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure. 
ID3D11VertexShader* LoadVertexShader(std::string shaderName)
{
    // Open compiled shader object file
    std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        return nullptr;
    }

    // Read file into vector of chars
    std::streamoff fileSize = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);
    std::vector<char> byteCode(fileSize);
    shaderFile.read(&byteCode[0], fileSize);
    if (shaderFile.fail())
    {
        return nullptr;
    }

    // Create shader object from loaded file (we will use the object later when rendering)
    ID3D11VertexShader* shader;
    HRESULT hr = gD3DDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &shader);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return shader;
}

// Load a geometry shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure. 
// Basically the same code as above but for pixel shaders
ID3D11GeometryShader* LoadGeometryShader(std::string shaderName)
{
    // Open compiled shader object file
    std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        return nullptr;
    }

    // Read file into vector of chars
    std::streamoff fileSize = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);
    std::vector<char>byteCode(fileSize);
    shaderFile.read(&byteCode[0], fileSize);
    if (shaderFile.fail())
    {
        return nullptr;
    }

    // Create shader object from loaded file (we will use the object later when rendering)
    ID3D11GeometryShader* shader;
    HRESULT hr = gD3DDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &shader);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return shader;
}

ID3D11GeometryShader* LoadStreamOutGeometryShader(std::string shaderName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries, unsigned int soStride)
{
    // Open compiled shader object file
    std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        return nullptr;
    }

    // Read file into vector of chars
    std::streamoff fileSize = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);
    std::vector<char>byteCode(fileSize);
    shaderFile.read(&byteCode[0], fileSize);
    if (shaderFile.fail())
    {
        return nullptr;
    }

    // Create shader object from loaded file (we will use the object later when rendering)
    ID3D11GeometryShader* shader;
    HRESULT hr = gD3DDevice->CreateGeometryShaderWithStreamOutput(byteCode.data(), byteCode.size(),
        soDecl, soNumEntries, &soStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &shader);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return shader;
}

// Load a pixel shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure. 
// Basically the same code as above but for pixel shaders
ID3D11PixelShader* LoadPixelShader(std::string shaderName)
{
    // Open compiled shader object file
    std::ifstream shaderFile(shaderName + ".cso", std::ios::in | std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        return nullptr;
    }

    // Read file into vector of chars
    std::streamoff fileSize = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);
    std::vector<char>byteCode(fileSize);
    shaderFile.read(&byteCode[0], fileSize);
    if (shaderFile.fail())
    {
        return nullptr;
    }

    // Create shader object from loaded file (we will use the object later when rendering)
    ID3D11PixelShader* shader;
    HRESULT hr = gD3DDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &shader);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return shader;
}

// Very advanced topic: When creating a vertex layout for geometry (see Scene.cpp), you need the signature
// (bytecode) of a shader that uses that vertex layout. This is an annoying requirement and tends to create
// unnecessary coupling between shaders and vertex buffers.
// This is a trick to simplify things - pass a vertex layout to this function and it will write and compile
// a temporary shader to match. You don't need to know about the actual shaders in use in the app.
// Release the signature (called a ID3DBlob!) after use. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements)
{
    std::string shaderSource = "float4 main(";
    for (int elt = 0; elt < numElements; ++elt)
    {
        auto& format = vertexLayout[elt].Format;
        // This list should be more complete for production use
        if      (format == DXGI_FORMAT_R32G32B32A32_FLOAT) shaderSource += "float4";
        else if (format == DXGI_FORMAT_R32G32B32_FLOAT)    shaderSource += "float3";
        else if (format == DXGI_FORMAT_R32G32_FLOAT)       shaderSource += "float2";
        else if (format == DXGI_FORMAT_R32_FLOAT)          shaderSource += "float";
        else if (format == DXGI_FORMAT_R8G8B8A8_UINT)      shaderSource += "uint4";
        else return nullptr; // Unsupported type in layout

        uint8_t index = static_cast<uint8_t>(vertexLayout[elt].SemanticIndex);
        std::string semanticName = vertexLayout[elt].SemanticName;
        semanticName += ('0' + index);

        shaderSource += " ";
        shaderSource += semanticName;
        shaderSource += " : ";
        shaderSource += semanticName;
        if (elt != numElements - 1)  shaderSource += " , ";
    }
    shaderSource += ") : SV_Position {return 0;}";

    ID3DBlob* compiledShader;
    HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), NULL, NULL, NULL, "main",
        "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, &compiledShader, NULL);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return compiledShader;
}


//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Constant Buffers are a way of passing data from C++ to the GPU. They are called constants but that only means
// they are constant for the duration of a single GPU draw call. The "constants" correspond to variables in C++
// that we will change per-model, or per-frame etc.
//
// We typically set up a C++ structure to exactly match the values we need in a shader and then create a constant
// buffer the same size as the structure. That makes updating values from C++ to shader easy - see the main code.

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure. 
ID3D11Buffer* CreateConstantBuffer(int size)
{
    D3D11_BUFFER_DESC cbDesc;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.ByteWidth = 16 * ((size + 15) / 16);     // Constant buffer size must be a multiple of 16 - this maths rounds up to the nearest multiple
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;             // Indicates that the buffer is frequently updated
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU is only going to write to the constants (not read them)
    cbDesc.MiscFlags = 0;
    ID3D11Buffer* constantBuffer;
    HRESULT hr = gD3DDevice->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }

    return constantBuffer;
}


