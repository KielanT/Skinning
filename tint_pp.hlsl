#include "Common.hlsli"

// Post processing shader that tints (similar to the tint pixel shader)
Texture2D SceneTexture : register(t0); // Uses the scene texture
SamplerState PointSample : register(s0); 

float4 main(PostProcessingInput input) : SV_Target
{
    float3 colour = SceneTexture.Sample(PointSample, input.uv).rgb * gTintColour;

    return float4(colour, 0.1f);
}