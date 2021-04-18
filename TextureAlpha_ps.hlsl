#include "Common.hlsli"

// Alpha shader (removes pixels to create a see through effect around a texture)

Texture2D DiffuseMap : register(t0);
SamplerState TexSampler : register(s0);

float4 main(SimplePixelShaderInput input) : SV_Target
{

    float4 diffuseMapColour = DiffuseMap.Sample(TexSampler, input.uv);

    if (diffuseMapColour.a < 0.5)
    {
        discard;
    }
    
    return diffuseMapColour;
}