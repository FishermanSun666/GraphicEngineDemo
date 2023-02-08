#pragma once
#include "FrameRole.h"

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include"../nclgl/MeshAnimation.h"
#include"../nclgl/MeshMaterial.h"
#include"../nclgl/CubeRobot.h"

const enum Tree {
	TREE_NUM = 20,
	TREE_MIN_SPACE = 100,
	TREE_MIN = 50,
	TREE_RANGE = 30,
};

const enum Role {
	ROLE_MOVE_SPEED = 40,
};

const enum Cloud {
	CLOUD_NUM = 100
};

const float ROLE_POS_X = 0.40f;
const float ROLE_POS_Z = 0.45f;
const float ROLE_MOVE_MAX = 0.70f;

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();

	void LoadNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeList();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	void LoadTrees();
	void DrawTree(SceneNode* n);

	void LoadCloud();
	void DrawCloud(SceneNode* n);

	void LoadRobot();

	void DrawRole();
	void LoadRole();
	void UpdateRoleFrame(float dt);

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* sceneShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* roleMesh;
	Mesh* treeMesh;
	Mesh* cloudMesh;
	Mesh* robotMesh;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint muddyTex;
	GLuint CloudTex;
	vector<GLuint> treeTexs;

	MeshAnimation* anim;
	MeshMaterial* material;
	MeshMaterial* treeMaterial;
	vector<GLuint> matTextures;
	Vector3 rolePosition;
	bool roleDir = false;
	int currentFrame;
	float frameTime;

	float waterRotate;
	float waterCycle;


	Frustum frameFrustum;

	SceneNode* root;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};