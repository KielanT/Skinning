#include "CTexture.h"


CTexture::CTexture(std::string textureName)
{
	mName = textureName;
	mName2 = "";
	mTexture = nullptr;
	mTexture2 = nullptr;
	mTextureSRV = nullptr;
	mTextureSRV2 = nullptr;

	mLoadTexture();
}

CTexture::CTexture(std::string textureName, std::string textureName2)
{
	mName = textureName;
	mName2 = textureName2;
	mTexture = nullptr;
	mTexture2 = nullptr;
	mTextureSRV = nullptr;
	mTextureSRV2 = nullptr;

	mLoadTwoTexture();
}

CTexture::~CTexture()
{
	delete mTexture;
	delete mTexture2;
	delete mTextureSRV;
	delete mTextureSRV2;
}

bool CTexture::mLoadTexture()
{
	if (!LoadTexture(mName, &mTexture, &mTextureSRV))
	{
		gLastError = "Error loading texture: " + mName;
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
