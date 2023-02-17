#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshMaterial.h"

const enum LandScapeParam {
	TREE_NUM = 30,
	LOW_GRASS_NUM = 1000,
	HIGH_GRASS_NUM = 400,
	LANDSCAPE_SIZE_MIN = 100,
	LANDSCAPE_SIZE_INTERVAL = 60,
};

class MaterialNode : public SceneNode
{
public:
	MaterialNode(Mesh* mesh, vector<GLuint> textures) : SceneNode(mesh){
		this->textures = textures;
	}
	~MaterialNode() {}
	void Draw(OGLRenderer& r) override;
private:
	vector<GLuint> textures;
};

