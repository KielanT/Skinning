#include "Common.hlsli" 

float4 main(BasicPixelShaderInput input) : SV_Target
{
    return float4(gOutlineColour, 1.0f);
}