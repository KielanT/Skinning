#include "Common.hlsli"

// Wiggle vertex shader

LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output;
	
    float4 modelPosition = float4(modelVertex.position, 1);
    float4 worldPosition = mul(gWorldMatrix, modelPosition);
    
    float4 modelNormal = float4(modelVertex.normal, 0);
    output.worldNormal = mul(gWorldMatrix, modelNormal).xyz;
    
    output.worldPosition = worldPosition.xyz;
    
    output.uv = modelVertex.uv;
    
    // Moves the vertices depending on the wiggle variable                                
    worldPosition.x += sin(modelPosition.y + gWiggle / 6) * 1.1f; // Divides wiggle by 6 for the speed. * by 1.1 for the object scale
    worldPosition.y += sin(modelPosition.x + gWiggle / 6) * 1.1f;
    
    float4 viewPosition = mul(gViewMatrix, worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);
    
    return output;
}