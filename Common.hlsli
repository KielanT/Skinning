//--------------------------------------------------------------------------------------
// Common include file for all shaders
//--------------------------------------------------------------------------------------
// Using include files to define the type of data passed between the shaders


//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------

// The structure below describes the vertex data to be sent into the vertex shader for non-skinned models
struct BasicVertex
{
    float3 position : position;
    float3 normal   : normal;
    float2 uv       : uv;
};


// Particle data
struct ParticleData
{
    float3 position : position; 
    float3 velocity : velocity; 
    float life : life; 
};

//*******************

// The structure below describes the vertex data to be sent into the vertex shader for skinned models
// In addition to the usual vextex data it contains the indexes of 4 bones that influence this vertex and influence weight of each (which sum to 1)
struct SkinningVertex
{
    float3 position : position;
    float3 normal   : normal;
    float2 uv       : uv;
    uint4  bones    : bones;   // This is the first time we have used integers in a shader: these are indexes into the list of nodes for the skeleton
	//*** MISSING - below the "xxxxxx" should be the correct type for the bone weights
	uint4 weights   : weights;
};

//*******************

// Normal Mapping Vertex
struct TangentVertex
{
    float3 position : position;
    float3 normal : normal;
    float3 tangent : tangent;
    float2 uv : uv;
};


// This structure describes what data the lighting pixel shader receives from the vertex shader.
// The projected position is a required output from all vertex shaders - where the vertex is on the screen
// The world position and normal at the vertex are sent to the pixel shader for the lighting equations.
// The texture coordinates (uv) are passed from vertex shader to pixel shader unchanged to allow textures to be sampled
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition;   // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal   : worldNormal;     //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};


// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};

struct BasicPixelShaderInput
{
    float4 projectedPosition : SV_Position;
};

struct PostProcessingInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};

// Data for Normal mapping
struct NormalMappingPixelShaderInput
{
    float4 projectedPosition : SV_Position; 
                                            
    float3 worldPosition : worldPosition; 
    float3 worldNormal : worldNormal;
    float3 modelNormal   : modelNormal; 
    float3 modelTangent  : modelTangent; 
    
    float2 uv : uv;
};

struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 Posh : SV_POSITION;
    float3 PosL : POSITION;

};


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// These structures are "constant buffers" - a way of passing variables over from C++ to the GPU
// They are called constants but that only means they are constant for the duration of a single GPU draw call.
// These "constants" correspond to variables in C++ that we will change per-model, or per-frame etc.

// In this exercise the matrices used to position the camera are updated from C++ to GPU every frame along with lighting information
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp
cbuffer PerFrameConstants : register(b0) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gCameraMatrix;
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    float3   gLight1Position; // 3 floats: x, y z
    float    gViewportWidth; // Pad above variable to float4 (HLSL requirement - copied in the the C++ version of this structure)
    float3   gLight1Colour;
    float    gViewportHeight;
    float3   gLight1Facing;
    float    gLight1CosHalfAngle;
    float4x4 gLight1ViewMatrix;
    float4x4 gLight1ProjectionMatrix;
    int      gLight1Type;
    float3   padding3;
    
    float3   gLight2Position;
    float    padding4;
    float3   gLight2Colour;
    float    padding5;
    float3   gLight2Facing;
    float    gLight2CosHalfAngle;
    float4x4 gLight2ViewMatrix;
    float4x4 gLight2ProjectionMatrix;
    int      gLight2Type;
    float3   padding6;
    
    float3   gLight3Position;
    float    padding7;
    float3   gLight3Colour;
    float    padding8;
    float3   gLight3Facing;
    float    gLight3CosHalfAngle;
    float4x4 gLight3ViewMatrix;
    float4x4 gLight3ProjectionMatrix;
    int      gLight3Type;
    float3   padding9;

    float3   gLight4Position;
    float    padding10;
    float3   gLight4Colour;
    float    padding11;
    float3   gLight4Facing;
    float    gLight4CosHalfAngle;
    float4x4 gLight4ViewMatrix;
    float4x4 gLight4ProjectionMatrix;
    int      gLight4Type;
    float3   padding12;
    
    float3 gLight5Position;
    float padding13;
    float3 gLight5Colour;
    float padding14;
    float3 gLight5Facing;
    float gLight5CosHalfAngle;
    float4x4 gLight5ViewMatrix;
    float4x4 gLight5ProjectionMatrix;
    int gLight5Type;
    float3 padding15;
    
    float3   gAmbientColour;
    float    gSpecularPower;

    float3   gCameraPosition;
    float    gFrameTime;
    
    float    gWiggle;
    float    gParallaxDepth;
    float    pad;
    float    pad2;
    
    float3 gOutlineColour; 
    float  gOutlineThickness; 
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')



static const int MAX_BONES = 64;//*** MISSING - what is the maximum number of bones expected? Relates to a MISSING elsewhere

// If we have multiple models then we need to update the world matrix from C++ to GPU multiple times per frame because we
// only have one world matrix here. Because this data is updated more frequently it is kept in a different buffer for better performance.
// We also keep other data that changes per-model here
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b1) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;

    float3   gObjectColour;
    float    padding17;  // See notes on padding in structure above

    float4x4 gBoneMatrices[MAX_BONES];
}

cbuffer PostProcessingConstants : register(b1)
{
	// Tint post-process settings
    float3 gTintColour;
    float paddingA; // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

	// Grey noise post-process settings
    float2 gNoiseScale;
    float2 gNoiseOffset;

	// Burn post-process settings
    float gBurnHeight;
    float3 paddingC;

	// Distort post-process settings
    float gDistortLevel;
    float3 paddingD;

	// Spiral post-process settings
    float gSpiralLevel;
    float3 paddingE;

}