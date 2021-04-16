#pragma once
#include "GraphicsHelpers.h"

class CTexture
{

private:
	std::string mName;
	std::string mName2;
	ID3D11Resource* mTexture;
	ID3D11Resource* mTexture2;
	ID3D11ShaderResourceView* mTextureSRV;
	ID3D11ShaderResourceView* mTextureSRV2;

	bool mLoadTexture();
	bool mLoadTwoTexture();

public:
	CTexture(std::string textureName);
	CTexture(std::string textureName, std::string textureName2);
	~CTexture();

	ID3D11ShaderResourceView* GetTextureSRV() { return mTextureSRV; }
	ID3D11ShaderResourceView* const* GetTexture() { return &mTextureSRV; }
	ID3D11ShaderResourceView* const* GetTexture2() { return &mTextureSRV2; }
};

