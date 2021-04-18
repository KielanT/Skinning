#pragma once
#include "GraphicsHelpers.h"

class CTexture
{

private:
	std::string mName; // Stores the name of the first texture
	std::string mName2; // Stores the name of the second texture
	ID3D11Resource* mTexture;	// Stores the the first texture resource
	ID3D11Resource* mTexture2; 	// Stores the second texture resource
	ID3D11ShaderResourceView* mTextureSRV;  // Stores the the first texture resource view 
	ID3D11ShaderResourceView* mTextureSRV2;	// Stores the second texture resource view

	// Private functions for loading the textures
	bool mLoadTexture(); 
	bool mLoadTwoTexture();

public:
	// Constructors
	CTexture(std::string textureName); 
	CTexture(std::string textureName, std::string textureName2);
	~CTexture(); // deconstructors

	// Getters
	ID3D11ShaderResourceView* GetTextureSRV() { return mTextureSRV; }
	ID3D11ShaderResourceView* const* GetTexture() { return &mTextureSRV; }
	ID3D11ShaderResourceView* const* GetTexture2() { return &mTextureSRV2; }
};

