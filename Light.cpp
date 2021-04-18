#include "Light.h"
#include "GraphicsHelpers.h"
#include "Shader.h"
#include "State.h"
#include "Direct3DSetup.h"


Light::Light() // Constructer that sets up the lights
{
	mMesh = new Mesh("Light.x"); // Sets the default light mesh
	mModel = new Model(mMesh); // Set Model
	Colour = { 0.8f, 0.8f, 1.0f };// Set Colour
	Strength = 40; // Set Strength
	mModel->SetPosition({ 0, 10, 0 }); // Set ModelPos
	mModel->SetScale(pow(Strength, 0.7f));// SetModel Scale
	LoadTexture("Flare.jpg", &mLightDiffuseMap, &mLightDiffuseMapSRV); // Sets the texture
	mSpotlightConeAngle = 90; // Sets the default cone angle

	// Sets the default lighting data
	mLightColour = Colour * Strength; 
	mLightPosition = mModel->Position(); 
	mLightFacing = Normalise(mModel->WorldMatrix().GetZAxis());
	mLightCosHalfAngle = cos(ToRadians(mSpotlightConeAngle / 2));
	mLightViewMatrix = CalculateLightViewMatrix();
	mLightProjectionMatrix = CalculateLightProjectionMatrix();
	mLightType = 0; // Sets default light type
	mEffectType = 0; // Sets default effect type
}

Light::~Light() // Deletes everything
{
	delete mMesh;	mMesh = nullptr; 
	delete mModel;	mModel = nullptr;

	if (mLightDiffuseMap)		mLightDiffuseMap->Release();
	if (mLightDiffuseMapSRV)	mLightDiffuseMapSRV->Release();
}

void Light::RenderLightFromCamera() // Renders the lights from the camera
{
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0); // VS Shader
	gD3DContext->PSSetShader(gLightModelPixelShader, nullptr, 0); // PS Shader
	
	gD3DContext->PSSetShaderResources(0, 1, &mLightDiffuseMapSRV); // PS Shader Resources
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler); // PS Sampler

	gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff); // Set BlendState
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);// Set DepthStencil
	gD3DContext->RSSetState(gCullNoneState); // Set Cull State

	gPerModelConstants.objectColour = Colour; // SetColour
	mModel->Render(); // Render
}

void Light::RenderDepthBufferFromLight()  // Renders the light from the depth buffer
{
	// Get camera-like matrices from the spotlight, seet in the constant buffer and send over to GPU
	gPerFrameConstants.viewMatrix = CalculateLightViewMatrix();
	gPerFrameConstants.projectionMatrix = CalculateLightProjectionMatrix();
	gPerFrameConstants.viewProjectionMatrix = gPerFrameConstants.viewMatrix * gPerFrameConstants.projectionMatrix;
	UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

	// Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
	gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
	gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);


	//// Only render models that cast shadows ////

	// Use special depth-only rendering shaders
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gDepthOnlyPixelShader, nullptr, 0);

	// States - no blending, normal depth buffer and culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullFrontState);

	// Render models (Get an array)
}




void Light::UpdateScene(float frameTime, Model* modelToObit) // If the light has an effect then this gets called in update scene 
{
	const float gLightOrbit = 20.0f;
	const float gLightOrbitSpeed = 0.7f;
	// LightControls
	if (mEffectType == 1) // Obit effect
	{
		static float rotate = 0.0f;
		static bool go = true;
		SetPosition(modelToObit->Position() + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit });
		if (go)  rotate -= gLightOrbitSpeed * frameTime;
		if (KeyHit(Key_1))  go = !go;
	}
	else if (mEffectType == 2) // Pulsating effect
	{
		if (Strength >= 0.0f && Strength <= 30)
		{
			Strength -= 15.0f * frameTime;
			SetLightColour(Colour);
		}
		else
		{
			Strength = 30;
			SetLightColour(Colour);
		}
	}
	else if (mEffectType == 3) // Colour changing effect
	{
		if (Colour.z >= 0 && Colour.z <= 1)
		{
			float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // Gets a random number between 0 and 1
			Colour.z += random * frameTime;
			SetLightColour(Colour);
		}
		else
		{
			Colour.z = 0;
			SetLightColour(Colour);

		}

		if (Colour.y >= 0 && Colour.y <= 1)
		{
			float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			Colour.y += random * frameTime;
			SetLightColour(Colour);
		}
		else
		{
			Colour.y = 0;
			SetLightColour(Colour);

		}

		if (Colour.x >= 0 && Colour.x <= 1)
		{
			float random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			Colour.x += random * frameTime;
			SetLightColour(Colour);

		}
		else
		{
			Colour.x = 0;
			SetLightColour(Colour);
		}
	}
}


void Light::Render() // Renders the light
{
	mModel->Render();
}

// Light helper functions
CMatrix4x4 Light::CalculateLightViewMatrix() 
{ 
	return InverseAffine(mModel->WorldMatrix());
}

CMatrix4x4 Light::CalculateLightProjectionMatrix()
{
	return MakeProjectionMatrix(1.0f, ToRadians(mSpotlightConeAngle));
}
