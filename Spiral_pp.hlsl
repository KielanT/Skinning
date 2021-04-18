#include "Common.hlsli"

// Post process shader that spirals ( Doesn't work)
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); 

float4 main(PostProcessingInput input) : SV_Target
{
    const float2 centreUV = float2(0.5f, 0.5f);
    float2 centreOffsetUV = input.uv /*FILTER - not 0, read comment*/ - centreUV;
    float centreDistance = length(centreOffsetUV);
	
    float s, c;
    sincos(centreDistance * gSpiralLevel * gSpiralLevel, s, c);
	
    matrix<float, 2, 2> rot2D =
    {
        c, s,
	   -s, c /*FILTER - not 0, it's a 2D rotation matrix...?*/
    };
    float2 rotatedOffsetUV = mul(centreOffsetUV, rot2D);
    
    float3 outputColour = SceneTexture.Sample(PointSample, 0 /*FILTER - not 0, read the comment*/).rgb;

    return float4(outputColour, 0.1f);
}