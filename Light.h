#pragma once
#include "Model.h"
#include "Mesh.h"

class Light
{
private:
	Mesh* mMesh; // Stores the mesh
	Model* mModel; // Stores the model
	
	ID3D11Resource* mLightDiffuseMap; // Stores the resource
	ID3D11ShaderResourceView* mLightDiffuseMapSRV; // Stores the resource srv
	float mSpotlightConeAngle; // Stores the code

	// Scene Properties
	CVector3 mLightColour; 
	CVector3 mLightPosition;
	CVector3 mLightFacing;
	float mLightCosHalfAngle;
	CMatrix4x4 mLightViewMatrix;
	CMatrix4x4 mLightProjectionMatrix;
	int mLightType;
	int mEffectType;
	
	CVector3 Colour; 
	float Strength;

public:
	Light(); // Constructor
	~Light(); // deconstructor
	
	// Public functions for rendering
	void RenderLightFromCamera(); 
	void RenderDepthBufferFromLight();
	void Render();

	// Public function for update scene
	void UpdateScene(float frameTime, Model* modelToObit);
	
	// Setters
	void SetLightColour(CVector3 lightColour) { Colour = lightColour; mLightColour = lightColour * Strength; }
	void SetStrength(float strength) { Strength = strength; mLightColour = Colour * Strength;  }
	void SetPosition(CVector3 position) { mModel->SetPosition(position); mLightPosition = position; }
	void SetScale(float scale) { mModel->SetScale(scale); }
	void SetType(int type) { mLightType = type; }
	void SetEffect(int type) { mEffectType = type; }

	// Getters
	CVector3 GetLightColour() { return mLightColour; }
	float GetStrength() { return Strength; }
	CVector3 GetLightPosition() { return mLightPosition; }
	CVector3 GetLightFacing() { return mLightFacing; }
	float GetLightCosHalfAngle() { return mLightCosHalfAngle; }
	CMatrix4x4 GetLightViewMatrix() { return mLightViewMatrix; }
	CMatrix4x4 GetLightProjectionMatrix() { return mLightProjectionMatrix; }
	int GetLightType() { return mLightType; }
	int GetEffect() { return mEffectType; }

	// Helper functions
	CMatrix4x4 CalculateLightViewMatrix();
	CMatrix4x4 CalculateLightProjectionMatrix();
};

