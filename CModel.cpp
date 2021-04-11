#include "CModel.h"
#include "Shader.h"
#include "State.h"


CModel::CModel()
{
	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gBasicTransformVertexShader;
	mPixelShader = gTextureFadePixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture("DefaultTexture.jpg", &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
	
}

CModel::CModel(std::string texture)
{
	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gBasicTransformVertexShader;
	mPixelShader = gSimplePixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture(texture, &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });

}

CModel::CModel(std::vector<std::string> textures)
{
	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gTextureFadePixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadAllTextures(textures);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
	
}

CModel::~CModel()
{
	delete mMesh;
	delete mModel;
	delete mDiffuseMap;
	delete mDiffuseMapSRV;
}

void CModel::SetBlendType(EBlendType type)
{
	if (type == EBlendType::NoBlend)
	{
		mBlendState = gNoBlendingState;
		mDepthStencilState = gUseDepthBufferState;
	}
	else if (type == EBlendType::Additive)
	{
		mBlendState = gAdditiveBlendingState;
		mDepthStencilState = gDepthReadOnlyState;
	}
	else if (type == EBlendType::Multiplicative)
	{
		mBlendState = gMultiplicativeBlendingState;
		mDepthStencilState = gDepthReadOnlyState;
	}
	else if (type == EBlendType::Alpha)
	{
		mBlendState = gAlphaBlendingState;
		mDepthStencilState = gDepthReadOnlyState;
	}
}

void CModel::SetCull(ECullType type)
{
	if (type == ECullType::Back)
	{
		mRasterizerState = gCullBackState;
	}
	else if (type == ECullType::Front)
	{
		mRasterizerState = gCullFrontState;
	}
	else if (type == ECullType::None)
	{
		mRasterizerState = gCullNoneState;
	}
}

void CModel::Render()
{
	SetVSShader(mVertexShader);
	
	SetPSShader(gSimplePixelShader);                 // PS Shader
	
	
	//
	//SetBlendState(gNoBlendingState);                // Blend State
	//SetDepthStencilState(gUseDepthBufferState);  // Depth Stencil
	//SetRasterizerState(gCullBackState);      // Rat State
	//
	//if (mDiffusesMapSRVs.empty())
	//{
	//	SetPSShaderResource(mDiffuseMapSRV);       // Shader Resource							 
	//}
	//else
	//{
	//	SetPSShaderResource(mDiffusesMapSRVs);
	//}
	//SetSampler(gAnisotropic4xSampler);                 // Samplers

	mModel->Render(); // Render
}

void CModel::LoadAllTextures(std::vector<std::string> textures)
{
	for (int i = 0; i < textures.size(); ++i)
	{
		LoadTexture(textures[i], &mDiffuseMap, &mDiffuseMapSRV);
		mDiffusesMapSRVs.push_back(mDiffuseMapSRV);
	}
}

