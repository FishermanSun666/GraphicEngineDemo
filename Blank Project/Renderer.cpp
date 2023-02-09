#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	//initialize
	baseMesh = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.65f, 3.0f, 0.65f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f),
		Vector4(2.2, 2.275, 2.15, 1), heightmapSize.x);
	//load texture
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	muddyTex = SOIL_load_OGL_texture(TEXTUREDIR"muddy.png",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"blizzard_rt.jpg", TEXTUREDIR"blizzard_lf.jpg",
		TEXTUREDIR"blizzard_up.jpg", TEXTUREDIR"blizzard_dn.jpg",
		TEXTUREDIR "blizzard_bk.jpg", TEXTUREDIR"blizzard_ft.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	if (!earthTex || !earthBump || !cubeMap || !waterTex) {
		return;
	}
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	
	sceneShader = new Shader("SceneVertex.glsl", "coursework_texture_fragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	
	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess()|| !lightShader->LoadSuccess() || !sceneShader->LoadSuccess()) {
		return;
	}
	//node tree
	LoadNodes();

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	UpdateShaderMatrices();

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); //enabled and set to standard linear interpolation
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //sample linearly between each of the 6 separate textures
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}

Renderer::~Renderer(void) {
	if (camera) {
		delete camera;
	}
	if (heightMap) {
		delete heightMap;
	}
	if (baseMesh) {
		delete baseMesh;
	}
	if (reflectShader) {
		delete reflectShader;
	}
	if (skyboxShader) {
		delete skyboxShader;
	}
	if (lightShader) {
		delete lightShader;
	}
	if (light) {
		delete light;
	}
	if (sceneShader) {
		delete sceneShader;
	}	
}

void Renderer::UpdateScene(float dt) {
	if (camera->GetPosition().x > heightMap->GetHeightmapSize().x * 0.8f || camera->GetPosition().z > heightMap->GetHeightmapSize().z * 0.8f) {
		float yaw = camera->GetYaw();
		yaw = (int)yaw % 180 ;
		camera->SetYaw(yaw);
	}
	camera->UpdateCamera(dt);
	//update frustum
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	//rotate and shift the texture applied to the water slightly.
	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;
	//update node tree
	root->Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawHeightmap();
	DrawWater();

	BuildNodeLists(root);
	SortNodeList();
	UpdateShaderMatrices();

	DrawNodes();
	ClearNodeLists();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();
	baseMesh->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(),
		"diffuseTex1"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, muddyTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(),
		"cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) *
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	SetShaderLight(*light);
	baseMesh->Draw();
}
