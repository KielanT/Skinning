//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------
#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <string>

#include "CVector2.h"
#include "CVector3.h"
#include "CMatrix4x4.h"


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files. "extern" means
// this variable is defined in another file somewhere. We should use classes and avoid
// use of globals, but done this way to keep code simpler so the DirectX content is
// clearer. However, try to architect your own code in a better way.

// Windows variables
extern HWND gHWnd;

// Viewport size
extern int gViewportWidth;
extern int gViewportHeight;


// Important DirectX variables
extern ID3D11Device*           gD3DDevice;
extern ID3D11DeviceContext*    gD3DContext;
extern IDXGISwapChain*         gSwapChain;
extern ID3D11RenderTargetView* gBackBufferRenderTarget;  // Back buffer is where we render to
extern ID3D11DepthStencilView* gDepthStencil;            // The depth buffer contains a depth for each back buffer pixel
extern ID3D11ShaderResourceView* gDepthShaderView;

// Input constsnts
extern const float ROTATION_SPEED;
extern const float MOVEMENT_SPEED;


// A global error message to help track down fatal errors - set it to a useful message
// when a serious error occurs
extern std::string gLastError;



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
struct PerFrameConstants
{
    // These are the matrices used to position the camera
    CMatrix4x4 cameraMatrix;
    CMatrix4x4 viewMatrix;
    CMatrix4x4 projectionMatrix;
    CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    CVector3   light1Position; // 3 floats: x, y z
    float      viewportWidth;       // Pad above variable to float4 (HLSL requirement - which we must duplicate in this the C++ version of the structure)
    CVector3   light1Colour;
    float      viewportHeight;
    CVector3   light1Facing;
    float      light1CosHalfAngle;
    CMatrix4x4 light1ViewMatrix;
    CMatrix4x4 light1ProjectionMatrix;
    int        light1Type; // Used for setting the light type
    CVector3   padding3;

    CVector3   light2Position;
    float      padding4;
    CVector3   light2Colour;
    float      padding5;
    CVector3   light2Facing;
    float      light2CosHalfAngle;
    CMatrix4x4 light2ViewMatrix;
    CMatrix4x4 light2ProjectionMatrix;
    int        light2Type;
    CVector3   padding6;

    CVector3   light3Position;
    float      padding7;
    CVector3   light3Colour;
    float      padding8;
    CVector3   light3Facing;
    float      light3CosHalfAngle;
    CMatrix4x4 light3ViewMatrix;
    CMatrix4x4 light3ProjectionMatrix;
    int        light3Type;
    CVector3   padding9;

    CVector3   light4Position;
    float      padding10;
    CVector3   light4Colour;
    float      padding11;
    CVector3   light4Facing;
    float      light4CosHalfAngle;
    CMatrix4x4 light4ViewMatrix;
    CMatrix4x4 light4ProjectionMatrix;
    int        light4Type;
    CVector3   padding12;

    CVector3   light5Position;
    float      padding13;
    CVector3   light5Colour;
    float      padding14;
    CVector3   light5Facing;
    float      light5CosHalfAngle;
    CMatrix4x4 light5ViewMatrix;
    CMatrix4x4 light5ProjectionMatrix;
    int        light5Type;
    CVector3   padding15;

    CVector3   ambientColour;
    float      specularPower;

    CVector3   cameraPosition;
    float      frameTime;

    float      wiggle; // Used for controlling the wiggle variable C++ (CPU) side
    float      parallaxDepth; // Used for setting the parallx depth  C++ (CPU) side
    float      pad; // Padding variables (hlsl requires everything to be grouped in fours otherwise there is weird graphical errors
    float      pad2;

    CVector3   outlineColour;    // Cell shading outline colour
    float      outlineThickness; // Cell shading outline thickness
};

extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure



static const int MAX_BONES = 64;

// This is the matrix that positions the next thing to be rendered in the scene. Unlike the structure above this data can be
// updated and sent to the GPU several times every frame (once per model). However, apart from that it works in the same way.
struct PerModelConstants
{
    CMatrix4x4 worldMatrix;
    CVector3   objectColour; // Allows each light model to be tinted to match the light colour they cast
    float      padding17;
    CMatrix4x4 boneMatrices[MAX_BONES]; /*** MISSING - fill in this array size - easy. Relates to another MISSING*/
};
extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure

struct PostProcessingConstants // From future module (post processing lab)
{
    // Tint post-process settings
    CVector3 tintColour;
    float    paddingA;  

    // Grey noise post-process settings
    CVector2 noiseScale;
    CVector2 noiseOffset;

    // Burn post-process settings
    float    burnHeight;
    CVector3 paddingC;

    // Distort post-process settings
    float    distortLevel;
    CVector3 paddingD;

    // Spiral post-process settings
    float    spiralLevel;
    CVector3 paddingE;
};
extern PostProcessingConstants gPostProcessingConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer* gPostProcessingConstantBuffer;

#endif //_COMMON_H_INCLUDED_
