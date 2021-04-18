#pragma once

// Includes required
#include "Mesh.h"
#include "Model.h"
#include "CTexture.h"
#include "GraphicsHelpers.h"

// Class has some bugs so may not always be the best to use it
// -- Texture array doesn't work properly (For two textures, you need an array size of 3 because it skips the second element in the array)

enum class EBlendType : int // Enum blend class (public but can only be accessed if this class as been included)
{
	NoBlend = 1,
	Additive = 2,
	Multiplicative = 3,
	Alpha = 4
};

enum class ECullType : int // Enum Cull class (public but can only be accessed if this class as been included)
{
	Back = 1,
	Front = 2,
	None = 3
};

enum class ESamplerType : int // Enum Sampler class (public but can only be accessed if this class as been included)
{
	Point = 1,
	Trilinear = 2,
	Anisotropic4x = 3
};


class CModel
{
private: 
	std::string mName; // If there is an array of a specific type of model then you can indivdually set the names to keep track of them (if needed)
	Mesh* mMesh; // Used for setting the mesh
	Model* mModel; // Used for setting the mode
	std::string mTexture; // Used for setting one texture
	std::vector<std::string> mTextures; // Used for setting an array of textures

	ID3D11VertexShader* mVertexShader; // Used for setting the vertex shader
	ID3D11PixelShader* mPixelShader; // Used for setting the pixel shader

	ID3D11BlendState* mBlendState; // Used for setting blend state
	ID3D11DepthStencilState* mDepthStencilState; // Used for setting Depth stencil state
	ID3D11RasterizerState* mRasterizerState; // Used for setting cull state

	ID3D11Resource* mDiffuseMap; // Used for setting the resource map
	ID3D11ShaderResourceView* mDiffuseMapSRV;  // Used for setting the resource srv
	ID3D11SamplerState* mSamplerState; // Used for setting sampler state

	std::vector<ID3D11ShaderResourceView*> mDiffusesMapSRVs; // Used for setting an array of diffuse maps

public:
	// Constructors
	CModel();
	CModel(Mesh* mesh);
	CModel(Mesh* mesh, std::string texture);
	CModel(Mesh* mesh, std::vector<std::string> textures);
	CModel(std::string texture);
	CModel(std::vector<std::string> textures);
	CModel(CTexture* texture);
	~CModel(); // Deconstructors

	// Setters
	void SetName(std::string name) { mName = name; }

	void SetMesh(std::string mesh, bool requireTangent = false);
	
	void SetMesh(Mesh* mesh)
	{
		delete mMesh;
		delete mModel;
		mMesh = mesh;
		mModel = new Model(mMesh);
	}

	void SetShaders(ID3D11VertexShader* vs, ID3D11PixelShader* ps)
	{
		mVertexShader = vs;
		mPixelShader = ps;
	}
	void SetVSShader(ID3D11VertexShader* vs) { mVertexShader = vs; }
	void SetPSShader(ID3D11PixelShader* ps) { mPixelShader = ps; }


	void SetBlendType(EBlendType type);
	void SetCull(ECullType type);
	void SetSampler(ESamplerType type);

	void SetTexture(std::string texture) { mTexture = texture; LoadTexture(texture, &mDiffuseMap, &mDiffuseMapSRV); }
	void SetTexture(std::vector<std::string> texture) { mTextures = texture; LoadAllTextures(texture); }

	void SetPosition(CVector3 position, int node = 0) { mModel->SetPosition(position, node); }
	void SetRotation(CVector3 rotation, int node = 0) { mModel->SetRotation(rotation, node); }
	void SetScale   (CVector3 scale, int node = 0)    { mModel->SetScale(scale, node); }
	void SetScale   (float scale)                     { mModel->SetScale(scale); }

	// Getters
	CVector3 GetPosition(int node = 0) { return mModel->Position(node); }
	CVector3 GetRotation(int node = 0) { return mModel->Rotation(node); }
	CVector3 GetScale(int node = 0) { return mModel->Scale(node); }
	Model* GetModel() { return mModel; }
	std::string GetName() { return mName; }

	
	void Render(); // Render Function

private:

	// Private setter functions (Used for rendering)
	void mSetShaders(ID3D11VertexShader* vs, ID3D11PixelShader* ps)
	{
		gD3DContext->VSSetShader(vs, nullptr, 0);
		gD3DContext->PSSetShader(ps, nullptr, 0);
	}
	void mSetVSShader(ID3D11VertexShader* vs) { gD3DContext->VSSetShader(vs, nullptr, 0); }
	void mSetPSShader(ID3D11PixelShader* ps) { gD3DContext->PSSetShader(ps, nullptr, 0); }

	void SetStates(ID3D11BlendState* blend, ID3D11DepthStencilState* depthStencil, ID3D11RasterizerState* rasterizerState)
	{
		gD3DContext->OMSetBlendState(blend, nullptr, 0xffffff);
		gD3DContext->OMSetDepthStencilState(depthStencil, 0);
		gD3DContext->RSSetState(rasterizerState);
	}
	void SetBlendState       (ID3D11BlendState* blend)                { gD3DContext->OMSetBlendState(blend, nullptr, 0xffffff); }
	void SetDepthStencilState(ID3D11DepthStencilState* depthStencil)  { gD3DContext->OMSetDepthStencilState(depthStencil, 0); }
	void SetRasterizerState  (ID3D11RasterizerState* rasterizerState) { gD3DContext->RSSetState(rasterizerState); }

	void SetPSShaderResource(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* shaderResourceView)
	{
		gD3DContext->PSSetShaderResources(StartSlot, NumViews, &shaderResourceView);
	}
	void SetPSShaderResource(ID3D11ShaderResourceView* shaderResourceView) 
	{
		gD3DContext->PSSetShaderResources(0, 1, &shaderResourceView);
	}
	
	void SetPSShaderResource(std::vector<ID3D11ShaderResourceView*> shaderResourceView)
	{
		for (int i = 0; i < mDiffusesMapSRVs.size(); ++i)
		{
			gD3DContext->PSSetShaderResources(i, 1, &mDiffusesMapSRVs[i]);
		}
	}

	void SetSampler(ID3D11SamplerState* state) { gD3DContext->PSSetSamplers(0, 1, &state); }

	// Used for loading an array of functions
	void LoadAllTextures(std::vector<std::string> textures);

	
};

