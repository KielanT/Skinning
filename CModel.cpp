#include "CModel.h"

CModel::CModel(std::string mesh)
{
	mMesh = new Mesh(mesh);
	mModel = new Model(mMesh);
}

CModel::CModel(Mesh* mesh)
{

}

void CModel::Render()
{
	mModel->Render();
}
