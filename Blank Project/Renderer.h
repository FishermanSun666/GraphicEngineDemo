#pragma once
#include "RoleNode.h"
#include "MaterialNode.h"
#include "SunNode.h"
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

#define SHADOW_SIZE 2048
#define WATER_HEIGHT 128

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
	void CreateNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeList();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNodeShadows();
	void CreateSimpleNodes();
	void CreateSunNode();
	void CreateAnimatedNodes();
	void CreateRole();
	void CreateMaterialNodes();
	void CreateTree();
	void CreateTrees(Mesh* mesh, vector<GLuint> textures);
	/*void LoadCloud();
	void DrawCloud(SceneNode* n);*/

	Shader* mapShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* sceneShader;
	Shader* shadowShader;

	HeightMap* heightMap;
	Mesh* quadMesh;
	Mesh* mesh;	

	Light* light;
	Camera* camera;
	//map
	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint sunTex;
	GLuint muddyTex;
	//shadow
	GLuint shadowTex;
	GLuint shadowFBO;
	//frustum
	Frustum frameFrustum;
	//water
	float waterRotate;
	float waterCycle;
	//node list
	SceneNode* root;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	//status param
	bool autoCamera;
	bool lightMove;
};