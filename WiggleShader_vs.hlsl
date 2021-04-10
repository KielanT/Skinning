#include "Common.hlsli"


LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output;
	
    float4 modelPosition = float4(modelVertex.position, 1);
    float4 worldPosition = mul(gWorldMatrix, modelPosition);
    
    float4 modelNormal = float4(modelVertex.normal, 0);
    output.worldNormal = mul(gWorldMatrix, modelNormal).xyz;
    
    output.worldPosition = worldPosition.xyz;
    
    output.uv = modelVertex.uv;
    

    worldPosition.x += sin(modelPosition.y + gWiggle / 6) * 1.1f;
    worldPosition.y += sin(modelPosition.x + gWiggle / 6) * 1.1f;
    
    float4 viewPosition = mul(gViewMatrix, worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);
    
    return output;
}