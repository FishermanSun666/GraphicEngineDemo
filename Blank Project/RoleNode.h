#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/OGLRenderer.h"

class HeightMap;

const enum Role {
	ROLE_MOVE_SPEED = 60,
	ROLE_SCALE = 50,
	ROLE_MOVE_TIME = 20,
};

const float ROLE_POS_X = 0.40f;
const float ROLE_POS_Z = 0.45f;
const float ROLE_MOVE_MAX = 0.70f;

class RoleNode : public SceneNode{
public:
	RoleNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, vector<GLuint> matTextures);
	~RoleNode(void) {
		if (anim) {
			delete anim;
		}
		if (material) {
			delete material;
		}
	}
	void Update(float dt) override;
	void Draw(OGLRenderer& r) override;

	void SetHeightMap(HeightMap* map) {
		heightMap = map;
	}
	void SetPosition(Vector3 pos) {
		worldTransform.SetPositionVector(pos);
	}
protected:
	int direction = 1;
	int currentFrame;
	float frameTime;
	float moveTime = 0.0f;

	HeightMap* heightMap;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;
};

