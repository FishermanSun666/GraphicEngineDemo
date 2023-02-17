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

#define SHADOW_SIZE 2048
#define WATER_HEIGHT 128
#define POST_PASSES 10
#define SPLIT_SCREEN_NUM 4
#define POINT_LIGHT_COLOUR_BASE 0.3f

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
	void UpdateRoleCamera(float dt);

protected:
	void InitBasicScene();
	void InitBasicTextures();
	void InitShaders();
	void InitBufferFBO();
	void GenerateScreenTexture(GLuint& into, bool depth = false, bool shadow = false);
	void DrawScene();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	//node tree
	void InitNodeList();
	void DrawNodes();
	void GenerateNodeLists(SceneNode* from);
	void SortNodeList();
	void ClearNodeLists();
	void LoadSimpleNodes();
	void LoadAnimatedNodes();
	void LoadRoleNode();
	void LoadMaterialNodes();
	void LoadLandscapeNode(int number, string meshFile, string matFile, bool groundLight = false);
	void GenerateSunNode();
	void GenerateRoleNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat, vector<GLuint> textures);
	void GenerateLandscapeElements(int number, Mesh* mesh, vector<GLuint> texturesbool, bool groundLight = false);
	//shadow
	void InitShadow();
	void DrawShadowBuffer();
	//split screen;
	void InitSplitScreen();
	void DrawSceneOnSplitScreen();
	void DrawSplitScreen();
	//post processl;
	void InitPostProcess();
	void DrawSceneWithPostProcess();
	void ExecutePostProcess();
	void PresentScene();
	//deferred rendering
	void InitDeferredRendering();
	void GeneratePointLight(float redius, Vector3 position);
	void DrawPointLights(Vector3 viewPos);
	void CombineBuffers(GLuint tex);
	void DrawSceneWithPointLight();

	Shader* sceneShader;
	Shader* skyboxShader;
	Shader* reflectShader;
	Shader* shadowShader;
	Shader* textureShader;
	Shader* processShader;
	Shader* pointlightShader;
	Shader* combineShader;
	//painting mesh
	Mesh* quad;
	Mesh* sphere;
	HeightMap* heightMap;
	Light* light;
	Camera* camera;
	//map
	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint sunTex;
	GLuint muddyTex;
	//water
	float waterRotate = 0.0f;
	float waterCycle = 0.0f;
	//shadow
	GLuint shadowTex;
	GLuint shadowFBO;
	//frustum
	Frustum frameFrustum;
	//node list
	SceneNode* root;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	//status param
	bool autoCamera = false;
	bool moveLight;
	//post process
	bool postProcess = false;
	GLuint bufferFBO;
	//buffer process
	GLuint processFBO;
	GLuint bufferDepthTex;
	GLuint bufferNormalTex;
	GLuint bufferColourTex[SPLIT_SCREEN_NUM];
	//split screen
	bool splitScreen = false;
	Vector3 screenViewPosition[SPLIT_SCREEN_NUM];
	Vector3 screenPositions[SPLIT_SCREEN_NUM] = {
		Vector3::Vector3(-0.5f, 0.5f, 0.5f),
		Vector3::Vector3(0.5f, 0.5f, 0.5f),
		Vector3::Vector3(-0.5f, -0.5f, 0.5f),
		Vector3::Vector3(0.5f, -0.5f, 0.5f)
	}; //parameters of camera and screen position
	//perspective binding
	RoleNode* role;
	//deferred rendering
	bool dRendering = false;
	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;
	vector<Light*> pointLights;
};