// Includes Required
#include "CModel.h"
#include "Shader.h"
#include "State.h"


CModel::CModel() //Default Constructer that sets
{
	mName = "none"; // Sets default name

	mMesh = new Mesh("Cube.x");  // Creates default Model
	mModel = new Model(mMesh);	 // Creates default Model

	mVertexShader = gPixelLightingVertexShader; // Sets default Vertex Shader
	mPixelShader = gPixelLightingPixelShader; // Sets default pixel Shader

	mBlendState = gNoBlendingState;				// Sets default blend State
	mDepthStencilState = gUseDepthBufferState;  // Sets default depth stencil State
	mRasterizerState = gCullBackState;			// Sets default culling

	LoadTexture("DefaultTexture.jpg", &mDiffuseMap, &mDiffuseMapSRV); // Sets default texture
	mSamplerState = gAnisotropic4xSampler; // Sets default sampler state

	SetPosition({ 0, 10, 0 }); // Sets default Position
}

CModel::CModel(Mesh* mesh) // Contructors that sets a mesh
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

CModel::CModel(Mesh* mesh, std::string texture) // Constructor that sets the mesh and texture
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

CModel::CModel(Mesh* mesh, std::vector<std::string> textures) // Sets mesh and an array of textures
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

CModel::CModel(std::string texture) // Sets texture using the file name
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

CModel::CModel(CTexture* texture) // Sets a CTexture 
{
	mName = "none";

	mMesh = new Mesh("Cube.x");
	mModel = new Model(mMesh);

	mVertexShader = gPixelLightingVertexShader;
	mPixelShader = gPixelLightingPixelShader;

	mBlendState = gNoBlendingState;
	mDepthStencilState = gUseDepthBufferState;
	mRasterizerState = gCullBackState;

	mDiffuseMapSRV = texture->GetTextureSRV();
	mSamplerState = gAnisotropic4xSampler;

	SetPosition({ 0, 10, 0 });
}


CModel::CModel(std::vector<std::string> textures) // Set array of textures that saws the file name
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

CModel::~CModel() // Deletes everything
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

void CModel::SetMesh(std::string mesh, bool requireTangent) // Sets The mesh, and if the tangent is required or not
{
	delete mMesh; // Deletes previous mesh
	delete mModel; // Deletes previous model
	mMesh = new Mesh(mesh, requireTangent); // Sets the mesh
	mModel = new Model(mMesh); // Creates the model
}

void CModel::SetBlendType(EBlendType type) // Sets the blend type using an enum class
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

void CModel::SetCull(ECullType type) // Sets the cull state using enum class
{ 
	if (type == ECullType::Back) // Shows the back side of the triangles (
	{
		mRasterizerState = gCullBackState;
	}
	else if (type == ECullType::Front) // Shows the front side of the triangles (inside out)
	{
		mRasterizerState = gCullFrontState;
	}
	else if (type == ECullType::None) // Shows all sides of the triangles
	{
		mRasterizerState = gCullNoneState;
	}
}

void CModel::SetSampler(ESamplerType type) // Sets the sampler type using enum class
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

void CModel::Render() // Renders everything with the correct settings
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

void CModel::LoadAllTextures(std::vector<std::string> textures) // Function to load all the textures in an array
{
	for (int i = 0; i < textures.size(); ++i)
	{
		LoadTexture(textures[i], &mDiffuseMap, &mDiffuseMapSRV);
		mDiffusesMapSRVs.push_back(mDiffuseMapSRV);
	}
}
