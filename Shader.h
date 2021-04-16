//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------
#ifndef _SHADER_H_INCLUDED_
#define _SHADER_H_INCLUDED_

#include "Common.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global variables available to other files. "extern" means this variable is defined in another
// file somewhere. We should use classes and avoid use of globals, but done this way to keep code simpler
// so the DirectX content is clearer. However, try to architect your own code in a better way.

// Vertex and pixel shader DirectX objects
extern ID3D11VertexShader*   gPixelLightingVertexShader;
extern ID3D11PixelShader*    gPixelLightingPixelShader;
extern ID3D11VertexShader*   gBasicTransformVertexShader;
extern ID3D11VertexShader*   gSkinningVertexShader; // Skinning is performed in the vertex shader (matrix work), we can use any pixel shader for lighting etc.
extern ID3D11PixelShader*    gLightModelPixelShader;
extern ID3D11VertexShader*   gWiggleVertexShader;
extern ID3D11PixelShader*    gTextureFadePixelShader;
extern ID3D11PixelShader*    gSimplePixelShader;
extern ID3D11PixelShader*    gDepthOnlyPixelShader;
extern ID3D11VertexShader*   gNormalMapVertexShader;
extern ID3D11PixelShader*    gNormalMapPixelShader;
extern ID3D11PixelShader*    gParallaxMapPixelShader;
extern ID3D11VertexShader*   gCellShadingOutlineVertexShader;
extern ID3D11PixelShader*    gCellShadingOutlinePixelShader;
extern ID3D11PixelShader*    gCellShadingPixelShader;
extern ID3D11VertexShader*   gCubeMapVertexShader;
extern ID3D11PixelShader*    gCubeMapPixelShader;
extern ID3D11PixelShader*    gTintPixelShader;


//--------------------------------------------------------------------------------------
// Shader creation / destruction
//--------------------------------------------------------------------------------------

// Load shaders required for this app, returns true on success
bool LoadShaders();

// Release shaders used by the app
void ReleaseShaders();


//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11Buffer* CreateConstantBuffer(int size);


//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Load a shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11VertexShader*   LoadVertexShader(std::string shaderName);
ID3D11GeometryShader* LoadGeometryShader(std::string shaderName);
ID3D11PixelShader*	  LoadPixelShader (std::string shaderName);

// Special method to load a geometry shader that can use the stream-out stage, Use like the other functions in this file except
// also pass the stream out declaration, number of entries in the declaration and the size of each output element. 
// The returned pointer needs to be released before quitting. Returns nullptr on failure. 
ID3D11GeometryShader* LoadStreamOutGeometryShader(std::string shaderName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries, unsigned int soStride);


// Helper function. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements);


#endif //_SHADER_H_INCLUDED_
