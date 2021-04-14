//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shadeer

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory (the words map and texture are used interchangeably)
//****| INFO | Normal map, now contains per pixel heights in the alpha channel ****//
Texture2D DiffuseSpecularMap : register(t0); // Diffuse map (main colour) in rgb and specular map (shininess level) in alpha - C++ must load this into slot 0
Texture2D NormalHeightMap : register(t2); // Normal map in rgb and height maps in alpha - C++ must load this into slot 1
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic

Texture2D ShadowMapLight1 : register(t1);
SamplerState PointClamp : register(s1);

float4 main(NormalMappingPixelShaderInput input) : SV_Target
{
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
    float3 modelNormal = normalize(input.modelNormal);
    float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
    float3 modelBiTangent = cross(modelNormal, modelTangent);
    float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);
	

	//****| INFO |**********************************************************************************//
	// The following few lines are the parallax mapping. Converts the camera direction into model
	// space and adjusts the UVs based on that and the bump depth of the texel we are looking at
	// Although short, this code involves some intricate matrix work / space transformations
	//**********************************************************************************************//

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	// Transform camera vector from world into model space. Need *inverse* world matrix for this.
	// Only need 3x3 matrix to transform vectors, to invert a 3x3 matrix we transpose it (flip it about its diagonal)
    float3x3 invWorldMatrix = transpose((float3x3) gWorldMatrix);
    float3 cameraModelDir = normalize(mul(invWorldMatrix, cameraDirection)); // Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
    float3x3 tangentMatrix = transpose(invTangentMatrix);
    float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix).xy;
	
	// Get the height info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
    float textureHeight = gParallaxDepth * (NormalHeightMap.Sample(TexSampler, input.uv).a - 0.5f);
	
	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
    float2 offsetTexCoord = input.uv + textureHeight * textureOffsetDir;


	//*******************************************

	//****| INFO |**********************************************************************************//
	// The above chunk of code is used only to calculate "offsetTexCoord", which is the offset in 
	// which part of the texture we see at this pixel due to it being bumpy. The remaining code is 
	// exactly the same as normal mapping, but uses offsetTexCoord instead of the usual input.uv
	//**********************************************************************************************//

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
    float3 textureNormal = 2.0f * NormalHeightMap.Sample(TexSampler, offsetTexCoord).rgb - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
    float3 worldNormal = normalize(mul((float3x3) gWorldMatrix, mul(textureNormal, invTangentMatrix)));


	///////////////////////
	// Calculate lighting
    
	//// Light 1 ////

	// Direction and distance from pixel to light
    float3 light1Direction = normalize(gLight1Position - input.worldPosition);
    float3 light1Dist = length(gLight1Position - input.worldPosition);
    
    float3 diffuseLight1 = 0;
    float3 specularLight1 = 0;
    float3 halfway = 0;
    
    float spotlight1 = dot(-gLight1Facing, light1Direction);
    
    // Check if the pixel is within a cone
    if (spotlight1 > gLight1CosHalfAngle && gLight1Type == 1) // SpotLight
    {
        float4 light1ViewPos = mul(gLight1ViewMatrix, float4(input.worldPosition, 1.0f));
        float4 light1Projection = mul(gLight1ProjectionMatrix, light1ViewPos);
   
        //ShadowMap
        float2 shadowMapUV = 0.5f * light1Projection.xy / light1Projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y;
   
        float depthFromLight = light1Projection.z / light1Projection.w;
   
       // Set lighting
        if (depthFromLight = ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
        {
            light1Dist = length(gLight1Position - input.worldPosition);
            diffuseLight1 = gLight1Colour * max(dot(worldNormal, light1Direction), 0) / light1Dist;
            halfway = normalize(light1Direction + cameraDirection);
            specularLight1 = diffuseLight1 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
        }
    }
    else if (gLight1Type == 2) // Directional Light
    {
        // Directional lighting using the equation from Introduction to 3D Game Programming With DirectX 11 by Frank D. Luna (7.12.3)
        float3 lightVec = -light1Direction;
        float diffuseFactor = dot(lightVec, worldNormal);
        
        if (diffuseFactor > 0.0f)
        {
            float3 v = reflect(-lightVec, worldNormal);
            halfway = normalize(light1Direction + cameraDirection);
            float specFactor = pow(max(dot(v, halfway), 0.0f), gSpecularPower);
            diffuseLight1 = diffuseFactor;
            specularLight1 = specFactor;
        }

    }
    else if (gLight1Type == 0) // Defualt Light
    {
        light1Dist = length(gLight1Position - input.worldPosition);
        diffuseLight1 = gLight1Colour * max(dot(worldNormal, light1Direction), 0) / light1Dist;
        halfway = normalize(light1Direction + cameraDirection);
        specularLight1 = diffuseLight1 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
    }
    
	//// Light 2 ////

    float3 light2Direction = normalize(gLight2Position - input.worldPosition);
    float3 light2Dist = length(gLight2Position - input.worldPosition);
    
    float3 diffuseLight2 = 0;
    float3 specularLight2 = 0;
    float3 halfway2 = 0;
    
    float spotlight2 = dot(-gLight2Facing, light2Direction);
    
    // Check if the pixel is within a cone
    if (spotlight2 > gLight2CosHalfAngle && gLight2Type == 1) // SpotLight
    {
        float4 light2ViewPos = mul(gLight2ViewMatrix, float4(input.worldPosition, 1.0f));
        float4 light2Projection = mul(gLight2ProjectionMatrix, light2ViewPos);
   
        //ShadowMap
        float2 shadowMapUV = 0.5f * light2Projection.xy / light2Projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y;
   
        float depthFromLight = light2Projection.z / light2Projection.w;
   
       // Set lighting
        if (depthFromLight = ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
        {
            light2Dist = length(gLight2Position - input.worldPosition);
            diffuseLight2 = gLight2Colour * max(dot(worldNormal, light2Direction), 0) / light2Dist;
            halfway2 = normalize(light2Direction + cameraDirection);
            specularLight2 = diffuseLight2 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
        }
    }
    else if (gLight2Type == 2) // Directional Light
    {
        // Directional lighting using the equation from Introduction to 3D Game Programming With DirectX 11 by Frank D. Luna (7.12.3)
        float3 lightVec = -light1Direction;
        float diffuseFactor = dot(lightVec, worldNormal);
        
        if (diffuseFactor > 0.0f)
        {
            float3 v = reflect(-lightVec, worldNormal);
            halfway2 = normalize(light2Direction + cameraDirection);
            float specFactor = pow(max(dot(v, halfway2), 0.0f), gSpecularPower);
            diffuseLight2 = diffuseFactor;
            specularLight2 = specFactor;
        }

    }
    else if (gLight2Type == 0) // Defualt Light
    {
        light2Dist = length(gLight2Position - input.worldPosition);
        diffuseLight2 = gLight2Colour * max(dot(worldNormal, light2Direction), 0) / light2Dist;
        halfway2 = normalize(light2Direction + cameraDirection);
        specularLight2 = diffuseLight2 * pow(max(dot(worldNormal, halfway2), 0), gSpecularPower);
    }


    // Light 3
    
    float3 light3Direction = normalize(gLight3Position - input.worldPosition);
    float3 light3Dist = length(gLight3Position - input.worldPosition);
    
    float3 diffuseLight3 = 0;
    float3 specularLight3 = 0;
    float3 halfway3 = 0;
    
    float spotlight3 = dot(-gLight3Facing, light3Direction);
    
    // Check if the pixel is within a cone
    if (spotlight3 > gLight3CosHalfAngle && gLight3Type == 1) // SpotLight
    {
        float4 light3ViewPos = mul(gLight3ViewMatrix, float4(input.worldPosition, 1.0f));
        float4 light3Projection = mul(gLight3ProjectionMatrix, light3ViewPos);
   
        //ShadowMap
        float2 shadowMapUV = 0.5f * light3Projection.xy / light3Projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y;
   
        float depthFromLight = light3Projection.z / light3Projection.w;
   
       // Set lighting
        if (depthFromLight = ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
        {
            light3Dist = length(gLight3Position - input.worldPosition);
            diffuseLight3 = gLight3Colour * max(dot(worldNormal, light3Direction), 0) / light3Dist;
            halfway3 = normalize(light3Direction + cameraDirection);
            specularLight3 = diffuseLight3 * pow(max(dot(worldNormal, halfway3), 0), gSpecularPower);
        }
    }
    else if (gLight3Type == 2) // Directional Light
    {
        // Directional lighting using the equation from Introduction to 3D Game Programming With DirectX 11 by Frank D. Luna (7.12.3)
        float3 lightVec = -light3Direction;
        float diffuseFactor = dot(lightVec, worldNormal);
        
        if (diffuseFactor > 0.0f)
        {
            float3 v = reflect(-lightVec, worldNormal);
            halfway3 = normalize(light2Direction + cameraDirection);
            float specFactor = pow(max(dot(v, halfway3), 0.0f), gSpecularPower);
            diffuseLight3 = diffuseFactor;
            specularLight3 = specFactor;
        }

    }
    else if (gLight3Type == 0) // Defualt Light
    {
        light3Dist = length(gLight3Position - input.worldPosition);
        diffuseLight3 = gLight3Colour * max(dot(worldNormal, light3Direction), 0) / light3Dist;
        halfway3 = normalize(light3Direction + cameraDirection);
        specularLight3 = diffuseLight3 * pow(max(dot(worldNormal, halfway3), 0), gSpecularPower);
    }
    
    // Light 4
    
    float3 light4Direction = normalize(gLight4Position - input.worldPosition);
    float3 light4Dist = length(gLight4Position - input.worldPosition);
    
    float3 diffuseLight4 = 0;
    float3 specularLight4 = 0;
    float3 halfway4 = 0;
    
    float spotlight4 = dot(-gLight4Facing, light4Direction);
    
    // Check if the pixel is within a cone
    if (spotlight4 > gLight4CosHalfAngle && gLight4Type == 1) // SpotLight
    {
        float4 light4ViewPos = mul(gLight4ViewMatrix, float4(input.worldPosition, 1.0f));
        float4 light4Projection = mul(gLight4ProjectionMatrix, light4ViewPos);
   
        //ShadowMap
        float2 shadowMapUV = 0.5f * light4Projection.xy / light4Projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y;
   
        float depthFromLight = light4Projection.z / light4Projection.w;
   
       // Set lighting
        if (depthFromLight = ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
        {
            light4Dist = length(gLight4Position - input.worldPosition);
            diffuseLight4 = gLight4Colour * max(dot(worldNormal, light4Direction), 0) / light4Dist;
            halfway4 = normalize(light4Direction + cameraDirection);
            specularLight4 = diffuseLight4 * pow(max(dot(worldNormal, halfway4), 0), gSpecularPower);
        }
    }
    else if (gLight4Type == 2) // Directional Light
    {
        // Directional lighting using the equation from Introduction to 3D Game Programming With DirectX 11 by Frank D. Luna (7.12.3)
        float3 lightVec = -light4Direction;
        float diffuseFactor = dot(lightVec, worldNormal);
        
        if (diffuseFactor > 0.0f)
        {
            float3 v = reflect(-lightVec, worldNormal);
            halfway4 = normalize(light4Direction + cameraDirection);
            float specFactor = pow(max(dot(v, halfway4), 0.0f), gSpecularPower);
            diffuseLight4 = diffuseFactor;
            specularLight4 = specFactor;
        }

    }
    else if (gLight4Type == 0) // Defualt Light
    {
        light4Dist = length(gLight4Position - input.worldPosition);
        diffuseLight4 = gLight4Colour * max(dot(worldNormal, light4Direction), 0) / light4Dist;
        halfway4 = normalize(light4Direction + cameraDirection);
        specularLight4 = diffuseLight4 * pow(max(dot(worldNormal, halfway4), 0), gSpecularPower);
    }
    
    // Light 5
    
    float3 light5Direction = normalize(gLight5Position - input.worldPosition);
    float3 light5Dist = length(gLight5Position - input.worldPosition);
    
    float3 diffuseLight5 = 0;
    float3 specularLight5 = 0;
    float3 halfway5 = 0;
    
    float spotlight5 = dot(-gLight5Facing, light5Direction);
    
    // Check if the pixel is within a cone
    if (spotlight5 > gLight5CosHalfAngle && gLight5Type == 1) // SpotLight
    {
        float4 light5ViewPos = mul(gLight5ViewMatrix, float4(input.worldPosition, 1.0f));
        float4 light5Projection = mul(gLight5ProjectionMatrix, light5ViewPos);
   
        //ShadowMap
        float2 shadowMapUV = 0.5f * light5Projection.xy / light5Projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y;
   
        float depthFromLight = light5Projection.z / light5Projection.w;
   
       // Set lighting
        if (depthFromLight = ShadowMapLight1.Sample(PointClamp, shadowMapUV).r)
        {
            light5Dist = length(gLight5Position - input.worldPosition);
            diffuseLight5 = gLight5Colour * max(dot(worldNormal, light5Direction), 0) / light5Dist;
            halfway5 = normalize(light5Direction + cameraDirection);
            specularLight5 = diffuseLight5 * pow(max(dot(worldNormal, halfway5), 0), gSpecularPower);
        }
    }
    else if (gLight5Type == 2) // Directional Light
    {
        // Directional lighting using the equation from Introduction to 3D Game Programming With DirectX 11 by Frank D. Luna (7.12.3)
        float3 lightVec = -light5Direction;
        float diffuseFactor = dot(lightVec, worldNormal);
        
        if (diffuseFactor > 0.0f)
        {
            float3 v = reflect(-lightVec, worldNormal);
            halfway5 = normalize(light5Direction + cameraDirection);
            float specFactor = pow(max(dot(v, halfway5), 0.0f), gSpecularPower);
            diffuseLight5 = diffuseFactor;
            specularLight5 = specFactor;
        }

    }
    else if (gLight4Type == 0) // Defualt Light
    {
        light5Dist = length(gLight4Position - input.worldPosition);
        diffuseLight5 = gLight4Colour * max(dot(worldNormal, light5Direction), 0) / light5Dist;
        halfway5 = normalize(light5Direction + cameraDirection);
        specularLight5 = diffuseLight5 * pow(max(dot(worldNormal, halfway5), 0), gSpecularPower);
    }
    
	// Sum the effect of the lights - add the ambient at this stage rather than for each light (or we will get too much ambient)
    float3 diffuseLight = gAmbientColour + diffuseLight1 + diffuseLight2 + diffuseLight3 + diffuseLight4 + diffuseLight5;
    float3 specularLight = specularLight1 + specularLight2 + specularLight3 + specularLight4 + specularLight5;
    
	////////////////////
	// Combine lighting and textures

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a; // Specular material colour in texture A (shininess of the surface)

    // Combine lighting with texture colours
    float3 finalColour = diffuseLight * diffuseMaterialColour + specularLight * specularMaterialColour;
    //float3 finalColour = (diffuseLight + diffuse2Light) * diffuseMaterialColour + (specular2Light + specularLight) * specularMaterialColour; // Adds the diffues lights together (does same effect as light above)

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}