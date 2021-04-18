#include "Common.hlsli"

// Post processing shader grey noise
Texture2D SceneTexture : register(t0); // Takes the scene texture
SamplerState PointSample : register(s0); 

Texture2D NoiseMap : register(t1); // Also takes the noise texture
SamplerState TrilinearWrap : register(s1);

float4 main(PostProcessingInput input) : SV_Target
{
    const float NoiseStrength = 10.5f;  // The noise strength which shows how much you can see (opacity)

    float3 sceneColour = SceneTexture.Sample(PointSample, input.uv).rgb;
    //float grey = 0/*FILTER - not 0, but a method to convert the r,g & b of texColour to a single grey-scale value (result in range 0->1)*/;
    sceneColour.rgb /= 1.0f;

    float2 noiseUV = input.uv * gNoiseScale + gNoiseOffset;
    sceneColour += NoiseStrength * (NoiseMap.Sample(TrilinearWrap, noiseUV).r - 0.5f); 
    
    return float4(sceneColour, 0.1f);
}