#pragma once
#include "RenderNode.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/OGLRenderer.h"

class HeightMap;

const enum Role {
	ROLE_MOVE_SPEED = 40,
	ROLE_SCALE = 30,
};

const float ROLE_POS_X = 0.40f;
const float ROLE_POS_Z = 0.45f;
const float ROLE_MOVE_MAX = 0.70f;

class RoleNode : public RenderNode{
public:
	RoleNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, vector<GLuint> matTextures, HeightMap* map, Shader* shader);
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
protected:
	int direction = 1;
	int currentFrame;
	float frameTime;

	HeightMap* heightMap;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;
};

