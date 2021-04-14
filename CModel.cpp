#include "CModel.h"
#include "Shader.h"
#include "State.h"


CModel::CModel()
{
	mName = "none";

	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture("DefaultTexture.jpg", &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
}

CModel::CModel(Mesh* mesh)
{
	mName = "none";

	mMesh = mesh;
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture("DefaultTexture.jpg", &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
}

CModel::CModel(Mesh* mesh, std::string texture)
{
	mName = "none";

	mMesh = mesh;
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture(texture, &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
}

CModel::CModel(Mesh* mesh, std::vector<std::string> textures)
{
	mName = "none";

	mMesh = mesh;
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadAllTextures(textures);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
}

CModel::CModel(std::string texture)
{
	mName = "none";

	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadTexture(texture, &mDiffuseMap, &mDiffuseMapSRV);
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });

}

CModel::CModel(std::vector<std::string> textures)
{
	mName = "none";

	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	LoadAllTextures(textures);
	mSamplerState = gAnisotropic4xSampler;

}

CModel::~CModel()
{
	delete mMesh;
	delete mModel;
	delete mDiffuseMap;
	delete mDiffuseMapSRV;
	delete mVertexShader;
	delete mPixelShader;
	delete mBlendState;
	delete mDepthStencilState;
	delete mRasterizerState;
	delete mSamplerState;

	for (int i = 0; i < mDiffusesMapSRVs.size(); ++i)
	{
		delete mDiffusesMapSRVs[i];
	}
}

void CModel::SetMesh(std::string mesh, bool requireTangent)
{
	delete mMesh;
	delete mModel;
	mMesh = new Mesh(mesh, requireTangent);
	mModel = new Model(mMesh);
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

void CModel::SetSampler(ESamplerType type)
{
	if (type == ESamplerType::Point)
	{
		mSamplerState = gPointSampler;
	}
	else if (type == ESamplerType::Trilinear)
	{
		mSamplerState = gTrilinearSampler;
	}
	else if (type == ESamplerType::Anisotropic4x)
	{
		mSamplerState = gAnisotropic4xSampler;
	}
}

void CModel::Render()
{
	mSetVSShader(mVertexShader);
	
	mSetPSShader(mPixelShader);                 // PS Shader
	
	SetBlendState(mBlendState);                // Blend State
	SetDepthStencilState(mDepthStencilState);  // Depth Stencil
	SetRasterizerState(mRasterizerState);      // Rat State
	
	if (mDiffusesMapSRVs.empty())
	{
		SetPSShaderResource(mDiffuseMapSRV);		 
	}
	else
	{
		SetPSShaderResource(mDiffusesMapSRVs);
	}
	SetSampler(mSamplerState);                 // Samplers

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
