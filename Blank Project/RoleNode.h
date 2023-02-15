#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/OGLRenderer.h"

class HeightMap;

//role move
#define ROLE_MOVE_SPEED 60.0f
#define ROLE_MOVE_TIME 20.0f
#define ROLE_MOVE_MAX 0.70f
//role position
#define ROLE_POS_X 0.65f
#define ROLE_POS_Z 0.35f
//role size
#define ROLE_MODEL_SCALE 50.0f
//view offset
#define ROLE_THIRD_VIEW_OFFSET 40.0f

enum ViewMode {
	DEFAULT_VIEW_MODE,
	FIRST_PERSON_VIEW_MODE,
	THIRD_PERSON_VIEW_MODE,
	VIEW_MODE_MAX
};

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
		for (auto i : matTextures) {
			glDeleteTextures(1, &i);
		}
	}
	int GetViewMode() { return viewMode; }
	Vector3	GetPosition() { return worldTransform.GetPositionVector(); }

	void Update(float dt) override;
	void ExecuteFrame(float dt);
	void Draw(OGLRenderer& r) override;
	void DrawShadow(OGLRenderer& r) override;
	void SetHeightMap(HeightMap* map) {
		heightMap = map;
	}
	void ChangeViewMode() {
		viewMode++;
		if (VIEW_MODE_MAX <= viewMode) {
			viewMode = DEFAULT_VIEW_MODE;
		}
	}
	void TurnDirection(int angle) {
		Vector3 pos = worldTransform.GetPositionVector();
		worldTransform = worldTransform.Rotation(angle + 180.0f, Vector3(0.0f, 1.0f, 0.0f));
		worldTransform.SetPositionVector(pos);
	}
	void Move(float dt, float dir);
protected:

	int direction = 1;
	int currentFrame;
	int viewMode = DEFAULT_VIEW_MODE;

	float frameTime;
	float moveTime = 0.0f;

	HeightMap* heightMap;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;
};

