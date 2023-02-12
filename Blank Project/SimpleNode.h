#pragma once
#include "../nclgl/SceneNode.h"
class SunNode : public SceneNode {
public:
	SunNode(Mesh* mesh) : SceneNode(mesh) {};
	void Draw(OGLRenderer& r) override;
};

