#pragma once
#include "../nclgl/SceneNode.h"
#include"../nclgl/OGLRenderer.h"

enum NodeType {
	DEFAULT_NODE_TYPE,
	ROLE_NODE,
	TREE_NODE,
	CLOUD_NODE,
};

class RenderNode : public SceneNode
{
protected:
	int nodeType;
	Shader* shader;
public:
	RenderNode(int type, Shader* shader) : SceneNode() {
		nodeType = type;
		this->shader = shader;
	}
	~RenderNode() {}

	int GetType() { return nodeType; }
	Shader* GetShader() { return shader; }
};

