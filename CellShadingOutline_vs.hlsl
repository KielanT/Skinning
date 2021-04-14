#include "Common.hlsli"

BasicPixelShaderInput main(BasicVertex modelVertex)
{
    BasicPixelShaderInput output; 
    
    float4 modelPosition = float4(modelVertex.position, 1);
    float4 worldPosition = mul(gWorldMatrix, modelPosition);

    float4 viewPosition = mul(gViewMatrix, worldPosition);

    float4 modelNormal = float4(modelVertex.normal, 0.0f);
    float4 worldNormal = normalize(mul(gWorldMatrix, modelNormal)); 

    worldPosition += gOutlineThickness * sqrt(viewPosition.z) * worldNormal;

    viewPosition = mul(gViewMatrix, worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    return output; 
}