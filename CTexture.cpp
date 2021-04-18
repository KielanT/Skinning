#include "CTexture.h"


CTexture::CTexture(std::string textureName) // Sets CTexture to a texuture from file name
{
	// Set default data
	mName = textureName;
	mName2 = "";
	mTexture = nullptr;
	mTexture2 = nullptr;
	mTextureSRV = nullptr;
	mTextureSRV2 = nullptr;

	mLoadTexture(); // Loads one texure
}

CTexture::CTexture(std::string textureName, std::string textureName2) // Sets two textures
{
	// Set default data
	mName = textureName;
	mName2 = textureName2;
	mTexture = nullptr;
	mTexture2 = nullptr;
	mTextureSRV = nullptr;
	mTextureSRV2 = nullptr;

	mLoadTwoTexture(); // Loads two textures 
}

CTexture::~CTexture() // Deletes everything
{
	delete mTexture;
	delete mTexture2;
	delete mTextureSRV;
	delete mTextureSRV2;
}

bool CTexture::mLoadTexture() // Loads one texutre
{
	if (!LoadTexture(mName, &mTexture, &mTextureSRV))
	{
		gLastError = "Error loading texture: " + mName; // Says what texture is throwing an error
		return false;
	}

	return true;
}

bool CTexture::mLoadTwoTexture()
{
	if (!LoadTexture(mName, &mTexture, &mTextureSRV) || 
		!LoadTexture(mName2, &mTexture2, &mTextureSRV2))
	{
		gLastError = "Error loading textures";
		return false;
	}

	return true;
}
