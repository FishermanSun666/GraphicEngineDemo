#pragma once
#include "RenderNode.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshMaterial.h"

const enum Tree {
	TREE_NUM = 20,
	TREE_MIN_SPACE = 100,
	TREE_MIN = 50,
	TREE_RANGE = 30,
};

class TreeNode : public RenderNode
{
public:
	TreeNode(vector<GLuint> textures, Shader* shader) : RenderNode(TREE_NODE, shader) {
		this->textures = textures;
	}
	~TreeNode() {}
	void Draw(OGLRenderer& r) override;
private:
	vector<GLuint> textures;
};

