#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/Light.h"

#define SUN_SIZE 200

class SunNode : public SceneNode {
public:
	SunNode(Mesh* mesh, Light* light) : SceneNode(mesh) {
		this->light = light;
	}
	void Update(float dt) override;
	void Draw(OGLRenderer& r) override;
private:
	Light* light;
	float angle;
};

