#include "Common.hlsli"

VertexOut main(VertexIn vin)
{
    VertexOut output;
    
    output.Posh = mul(float4(vin.PosL, 1.0f), gViewProjectionMatrix).xyww;
    
    output.PosL = vin.PosL;
    
    return output;
}