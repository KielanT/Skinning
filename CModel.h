#pragma once
#include "Mesh.h"
#include "Model.h"
#include "GraphicsHelpers.h"

enum class EBlendType : int
{
	NoBlend = 1,
	Additive = 2,
	Multiplicative = 3,
	Alpha = 4
};

enum class ECullType : int
{
	Back = 1,
	Front = 2,
	None = 3
};

enum class ESamplerType : int
{
	Point = 1,
	Trilinear = 2,
	Anisotropic4x = 3
};


class CModel
{
private: 
	std::string mName;
	Mesh* mMesh;
	Model* mModel;
	std::string mTexture;
	std::vector<std::string> mTextures;

	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;

	ID3D11BlendState* mBlendState;
	ID3D11DepthStencilState* mDepthStencilState;
	ID3D11RasterizerState* mRasterizerState;

	ID3D11Resource* mDiffuseMap;
	ID3D11ShaderResourceView* mDiffuseMapSRV;
	ID3D11SamplerState* mSamplerState;

	std::vector<ID3D11ShaderResourceView*> mDiffusesMapSRVs;

public:
	CModel();
	CModel(Mesh* mesh);
	CModel(Mesh* mesh, std::string texture);
	CModel(Mesh* mesh, std::vector<std::string> textures);
	CModel(std::string texture);
	CModel(std::vector<std::string> textures);
	~CModel();

	// Setters
	void SetName(std::string name) { mName = name; }

	void SetMesh(std::string mesh) 
	{ 
		delete mMesh;
		delete mModel;
		mMesh = new Mesh(mesh);  
		mModel = new Model(mMesh); 
	}
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

	void Render();

private:

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

	void LoadAllTextures(std::vector<std::string> textures);

	
};

