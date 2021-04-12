#pragma once
#include "Mesh.h"
#include "Model.h"

class CModel
{
private:
	Mesh* mMesh;
	Model* mModel;

public:
	CModel(std::string mesh);
	CModel(Mesh* mesh);

	void SetMesh(std::string mesh) { mMesh = new Mesh(mesh); mModel = new Model(mMesh); };
	//void SetScale(CVector3 scale, int node = 0) { mModel->SetScale(scale, node); }
	void SetScale(float scale) { mModel->SetScale(scale); }

	void Render();
};

