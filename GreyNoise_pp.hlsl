#include "Common.hlsli"
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); 


Texture2D NoiseMap : register(t1);
SamplerState TrilinearWrap : register(s1);

float4 main(PostProcessingInput input) : SV_Target
{
    const float NoiseStrength = 0.5f; 

    float3 sceneColour = SceneTexture.Sample(PointSample, input.uv).rgb;
    //float grey = 0/*FILTER - not 0, but a method to convert the r,g & b of texColour to a single grey-scale value (result in range 0->1)*/;
    sceneColour.rgb /= 1.0f;

    float2 noiseUV = input.uv * gNoiseScale + gNoiseOffset;
    sceneColour += NoiseStrength * (NoiseMap.Sample(TrilinearWrap, noiseUV).r - 0.5f); 
    
    return float4(sceneColour, 0.1f);
}