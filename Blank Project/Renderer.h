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
#define POST_PASSES 10

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
	void UpdateKeyboard();

protected:
	void InitBasicScene();
	void DrawScene();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawNodes();
	void DrawNodeShadows();
	void DrawPostProcess();
	void PresentScene();
	//node tree
	void CreateNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeList();
	void ClearNodeLists();
	void CreateSimpleNodes();
	void CreateSunNode();
	void CreateAnimatedNodes();
	void CreateRoleNode();
	void CreateMaterialNodes();
	void CreateLandscapeNode(int number, string meshFile, string matFile);
	void BuildLandscapes(int number, Mesh* mesh, vector<GLuint> textures);

	Shader* sceneShader;
	Shader* skyboxShader;
	//water reflect
	Shader* reflectShader;
	//shadow
	Shader* shadowShader;
	//post process
	Shader* textureShader;
	Shader* processShader;

	HeightMap* heightMap;
	Mesh* quad;
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
	float waterRotate = 0.0f;
	float waterCycle = 0.0f;
	//node list
	SceneNode* root;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	//status param
	bool autoCamera;
	bool moveLight;
	//post process
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;
};