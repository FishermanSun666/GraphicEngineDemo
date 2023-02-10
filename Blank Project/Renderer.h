#pragma once
#include "RoleNode.h"
#include "TreeNode.h"
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

//const enum Cloud {
//	CLOUD_NUM = 100
//};

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
	//node tree
	void LoadNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeList();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void LoadRole();
	void LoadTrees();
	void CreateTrees(Mesh* mesh, vector<GLuint> textures);
	/*void LoadCloud();
	void DrawCloud(SceneNode* n);*/

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* sceneShader;

	HeightMap* heightMap;
	Mesh* baseMesh;
	Mesh* mesh;	
	Mesh* cloudMesh;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint muddyTex;
	GLuint CloudTex;
	Frustum frameFrustum;
	//water
	float waterRotate;
	float waterCycle;
	//node list
	SceneNode* root;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};