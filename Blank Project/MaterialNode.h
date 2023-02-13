#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshMaterial.h"

const enum Tree {
	TREE_NUM = 20,
	TREE_MIN_SPACE = 100,
	TREE_MIN = 50,
	TREE_RANGE = 30,
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

