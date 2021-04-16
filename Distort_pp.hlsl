#include "Common.hlsli"

Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); 

Texture2D DistortMap : register(t1);
SamplerState TrilinearWrap : register(s1);

float4 main(PostProcessingInput input) : SV_Target
{
    const float lightStrength = 0.015f;
	

    float3 distortTexture = DistortMap.Sample(TrilinearWrap, input.uv).rgb;

    float2 distortVector = distortTexture.rg/*FILTER - not 0, similar to crinkle vector used in burn shader above*/;

    distortVector -= float2(0.5f, 0.5f);

    float light = dot(normalize(distortVector), float2(0.707f, 0.707f)) * lightStrength;
	
    float3 outputColour = light /*FILTER - not 0, read comment*/ + SceneTexture.Sample(PointSample, input.uv + gDistortLevel * distortVector).rgb;


    return float4(outputColour, 0.1f);

}