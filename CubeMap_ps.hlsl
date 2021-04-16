#include "Common.hlsli" 

TextureCube CubeMap : register(t0);
SamplerState texSampler : register(s0);

float4 main(VertexOut pin) : SV_Target
{
    return CubeMap.Sample(texSampler, pin.PosL);
}