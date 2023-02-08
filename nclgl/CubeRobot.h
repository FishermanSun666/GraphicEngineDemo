#pragma once
#include "SceneNode.h"

class CubeRobot : public SceneNode {
public:
	CubeRobot(Mesh* cube);
	~CubeRobot(void) {};

	void Update(float dt) override;
protected:
	SceneNode* head;
	SceneNode* leftArm;
	SceneNode* rightArm;
};
