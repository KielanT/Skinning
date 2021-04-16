#include "Common.hlsli"

Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); 

Texture2D BurnMap : register(t1);
SamplerState TrilinearWrap : register(s1);

float4 main(PostProcessingInput input) : SV_Target
{
    const float3 white = 1.0f;

    const float3 burnColour = float3(0.8f, 0.4f, 0.0f);
    const float3 glowColour = float3(1.0f, 0.8f, 0.0f);
    const float glowAmount = 0.25f;
    const float crinkle = 0.15f;  

    float4 burnTexture = BurnMap.Sample(TrilinearWrap, input.uv);
    
    float burnLevelMax = gBurnHeight + glowAmount;

    float3 outputColour;

    if (burnTexture.r <= gBurnHeight)
    {
        outputColour = float3(0.0f, 0.0f, 0.0f);
    }
    
    else if (burnTexture.r >= burnLevelMax)
    {
        float3 texColour = SceneTexture.Sample(PointSample, input.uv).rgb;
        outputColour = texColour; /*FILTER, not 0, read the comment and refer to earlier post-processing shaders for code required*/
    }
    else 
    {


        float glowLevel = 1.0f - (burnTexture.r - gBurnHeight) / glowAmount;

		
        float2 crinkleVector = burnTexture.gb /*FILTER, not 0, read the comment*/;
		
        crinkleVector -= float2(0.5f, 0.5f);;

        float3 texColour = SceneTexture.Sample(PointSample, input.uv - glowLevel * crinkle * crinkleVector).rgb;

        glowLevel *= 2.0f;
        if (glowLevel < 1.0f)
        {
            outputColour = lerp(texColour, burnColour * texColour, glowLevel);
        }
        else
        {
            outputColour = lerp(burnColour * texColour, glowColour, glowLevel - 1.0f);
        }
    }

    return float4(outputColour, 0.1f);
}