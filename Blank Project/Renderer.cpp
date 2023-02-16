#include "Renderer.h"


Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	//initialize
	InitBasicScene();
	//load texture
	InitBasicTextures();
	//load shader
	InitShaders();
	//node tree
	InitNodeList();
	//shadow
	InitShadow();
	InitBufferFBO();
	//post process, generate scene depth texture
	InitPostProcess();
	InitSplitScreen();
	//deferred rendering
	InitParticleEmitter();
	InitDeferredRendering();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer(void) {
	glDeleteTextures(1, &cubeMap);
	glDeleteTextures(1, &waterTex);
	glDeleteTextures(1, &earthTex);
	glDeleteTextures(1, &earthBump);
	glDeleteTextures(1, &sunTex);
	glDeleteTextures(1, &muddyTex);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteTextures(SPLIT_SCREEN_NUM, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteFramebuffers(1, &deferRenderFBO);

	if (quad) {
		delete quad;
	}
	if (camera) {
		delete camera;
	}
	if (light) {
		delete light;
	}
	if (flameParticleEmitter) {
		delete flameParticleEmitter;
	}
}

void Renderer::InitBasicScene() {
	quad = Mesh::GenerateQuad();
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-30.0f, 0.0f, heightmapSize * Vector3(0.7f, 3.0f, 0.7f));
	//light = new Light(heightmapSize * Vector3(0.5f, 2.0f, -0.2f), Vector4(3.825f, 3.165f, 2.325f, 1.0f), heightmapSize.x);
	light = new Light(heightmapSize * Vector3(0.5f, 2.0f, -0.2f), Vector4(2.547f, 2.11f, 1.55f, 1.0f), heightmapSize.x);
}

void Renderer::InitBasicTextures() {
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	muddyTex = SOIL_load_OGL_texture(TEXTUREDIR"muddy.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sunTex = SOIL_load_OGL_texture(TEXTUREDIR"sun_texture.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(
		SKYBOXDIR"rusted_rt.jpg", SKYBOXDIR"rusted_lf.jpg",
		SKYBOXDIR"rusted_up.jpg", SKYBOXDIR"rusted_dn.jpg",
		SKYBOXDIR "rusted_bk.jpg", SKYBOXDIR"rusted_ft.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!earthTex || !earthBump || !cubeMap || !waterTex || !sunTex || !muddyTex) {
		return;
	}
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(muddyTex, true);
	SetTextureRepeating(sunTex, true);
}

void Renderer::InitShaders() {
	sceneShader = new Shader("mapVertex.glsl", "mapFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	shadowShader = new Shader("shadowVertex.glsl", "shadowFragment.glsl");
	textureShader = new Shader("texturedVertex.glsl", "texturedFragment.glsl");
	processShader = new Shader("texturedVertex.glsl", "trocessFragment.glsl");
	pointlightShader = new Shader("pointlightvertex.glsl", "pointlightfrag.glsl");
	combineShader = new Shader("combinevert.glsl", "combinefrag.glsl");
	particleShader = new Shader("particleVertex.glsl", "particleFragment.glsl", "particleGeometry.glsl");
	if (!sceneShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !reflectShader->LoadSuccess() || !shadowShader->LoadSuccess() || !textureShader->LoadSuccess() || !processShader->LoadSuccess() || !pointlightShader->LoadSuccess() || !combineShader->LoadSuccess() || !particleShader->LoadSuccess()){
		return;
	}
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth, bool shadow) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;
	if (shadow) {
		glTexImage2D(GL_TEXTURE_2D, 0, format, SHADOW_SIZE, SHADOW_SIZE, 0, type, GL_UNSIGNED_BYTE, NULL);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::UpdateScene(float dt) {
	UpdateKeyboard();
	if (!splitScreen) {
		UpdateRoleCamera(dt);
	}
	if (moveLight) {
		light->Rotation(dt, heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f));
	}
	//update frustum
	projMatrix = defaultView;
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	//rotate and shift the texture applied to the water slightly.
	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;
	//update node tree
	root->Update(dt);
	//particle flame
	flameParticleEmitter->Update(dt);
}

void Renderer::UpdateKeyboard() {
	//view mode
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		if (role) {
			role->ChangeViewMode();
		}
	}
	//move sun
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
		moveLight = moveLight ? false : true;
	}
	//post process
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
		postProcess = postProcess ? false : true;
		splitScreen = postProcess ? false : splitScreen;
	}
	//splitScreen
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
		splitScreen = splitScreen ? false : true;
		postProcess = splitScreen ? false : postProcess;
	}
	//deferred rendering
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5)) {
		dRendering = dRendering ? false : true;
	}
}

void Renderer::UpdateRoleCamera(float dt) {
	if (!role) {
		camera->UpdateCamera(dt);
		return;
	}
	if (DEFAULT_VIEW_MODE == role->GetViewMode()) {
		camera->UpdateCamera(dt);
		return;
	}
	float dir = camera->CaptureYaw();
	camera->CapturePitch();
	role->TurnDirection(dir);
	role->Move(dt, dir);
	Vector3 offSet = Vector3(0.0f, ROLE_MODEL_SCALE * 2.3f, 0.0f);
	if (THIRD_PERSON_VIEW_MODE == role->GetViewMode()) {
		offSet += Matrix4::Rotation(dir, Vector3(0, 1, 0)) * Vector3(0, 0, 1) * ROLE_THIRD_VIEW_OFFSET;
	}
	camera->SetPosition(role->GetPosition() + offSet);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	GenerateNodeLists(root);
	SortNodeList();
	DrawShadowBuffer();
	projMatrix = defaultView;
	if (postProcess) {
		DrawSceneWithPostProcess();
	}
	else if (splitScreen) {
		DrawSceneOnSplitScreen();
		DrawSplitScreen();
	}
	else if (dRendering) {
		DrawSceneWithPointLight();
	}
	else {
		viewMatrix = camera->BuildViewMatrix();
		UpdateShaderMatrices();
		DrawScene();
		DrawParticle();
	}
	ClearNodeLists();
}

void Renderer::DrawScene() {
	DrawSkybox();
	DrawHeightmap();
	DrawNodes();
	DrawWater();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(sceneShader);
	SetShaderLight(*light);
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "map"), true);

	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "muddyTex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, muddyTex);

	UpdateShaderMatrices();
	heightMap->Draw();
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "map"), false);
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(),
		"diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(),
		"cubeTex"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	projMatrix = defaultView;
	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}


void Renderer::DrawNodes() {
	BindShader(sceneShader);
	UpdateShaderMatrices();
	//set light;
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	SetShaderLight(*light);
	for (const auto& i : nodeList) {
		i->Draw(*this);
	}
	for (const auto& i : transparentNodeList) {
		i->Draw(*this);
	}
}

void Renderer::InitShadow() {
	GenerateScreenTexture(shadowTex, true, true);
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawShadowBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition() * Vector3(1.0f, 2.0f, 1.0f), Vector3(heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f)));
	projMatrix = defaultView;
	shadowMatrix = projMatrix * viewMatrix;
	for (const auto& i : nodeList) {
		i->DrawShadow(*this);
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void Renderer::InitNodeList(void) {
	root = new SceneNode();
	LoadAnimatedNodes();
	LoadMaterialNodes();
	LoadSimpleNodes();
}

void Renderer::LoadSimpleNodes() {
	if (!root || !heightMap) {
		return;
	}
	GenerateSunNode();
}

void Renderer::GenerateSunNode() {
	Mesh* sunMesh = Mesh::LoadFromMeshFile("Sphere.msh");
	auto node = new SunNode(sunMesh, light);

	node->SetModelScale(Vector3(SUN_SIZE, SUN_SIZE, SUN_SIZE));
	node->SetColour(Vector4(2.55f, 2.36f, 1.39f, 0.4f));
	node->SetBoundingRadius(heightMap->GetHeightmapSize().Length() * 10.0f);
	node->SetTexture(sunTex);
	if (root) {
		root->AddChild(node);
	}
}

void Renderer::LoadAnimatedNodes() {
	if (!root || !heightMap) {
		return;
	}
	LoadRoleNode();
}

void Renderer::LoadRoleNode() {
	//load role node
	Mesh* roleMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	auto anim = new MeshAnimation("Role_T.anm");
	auto material = new MeshMaterial("Role_T.mat");
	vector<GLuint> matTextures;
	//load sub mesh
	for (int i = 0; i < roleMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}
	GenerateRoleNode(roleMesh, anim, material, matTextures);
}

void Renderer::GenerateRoleNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* mat, vector<GLuint> textures) {
	role = new RoleNode(mesh, anim, mat, textures);
	role->SetHeightMap(heightMap);
	//Keeping the role on the ground
	float px = ROLE_POS_X * heightMap->GetHeightmapSize().x;
	float pz = ROLE_POS_Z * heightMap->GetHeightmapSize().z;
	role->SetPosition(Vector3(px, heightMap->GetHeight(px, pz), pz));
	role->SetModelScale(Vector3(ROLE_MODEL_SCALE, ROLE_MODEL_SCALE, ROLE_MODEL_SCALE));
	role->SetBoundingRadius(heightMap->GetHeightmapSize().Length());
	if (root) {
		root->AddChild(role);
	}
}

void Renderer::LoadMaterialNodes() {
	if (!root || !heightMap) {
		return;
	}
	//tree
	LoadLandscapeNode(TREE_NUM, "tree.msh", "tree.mat");
	//low grass
	LoadLandscapeNode(LOW_GRASS_NUM, "grass0.msh", "grass0.mat");
	//high grass
	LoadLandscapeNode(HIGH_GRASS_NUM, "grass1.msh", "grass1.mat");
}

void Renderer::LoadLandscapeNode(int number, string meshFile, string matFile) {
	Mesh* mesh = Mesh::LoadFromMeshFile(meshFile);
	MeshMaterial* material = new MeshMaterial(matFile);
	if (!mesh || !material) {
		return;
	}
	//load multi texture
	vector<GLuint> textures;
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		textures.emplace_back(texID);
	}
	//create tree
	GenerateLandscapeElements(number, mesh, textures);
}

//Create trees of a certain density depending on the size of the map
//@param mesh - tree mesh
//@param textures - tree's multiple texture
void Renderer::GenerateLandscapeElements(int number, Mesh* mesh, vector<GLuint> textures) {
	Vector3 mapSize = heightMap->GetHeightmapSize();
	float randBase = 0;
	//random generate
	for (int i = 0; i < number; i++) {
		randBase = rand() % (LANDSCAPE_SIZE_INTERVAL);
		float size = randBase + LANDSCAPE_SIZE_MIN;
		float x = rand() % (int)mapSize.x;
		float z = rand() % (int)mapSize.z;
		float y = heightMap->GetHeight(x, z);
		//no elements under the river
		if (y <= WATER_HEIGHT) {
			i = 0 < i ? i - 1 : i;
			continue;
		}
		MaterialNode* s = new MaterialNode(mesh, textures);
		auto transform = Matrix4::Translation(Vector3(x, y, z)) * Matrix4::Scale(Vector3(size, size, size)) * Matrix4::Rotation(randBase, Vector3(0, 1, 0));
		s->SetTransform(transform);
		s->SetBoundingRadius(mapSize.Length());
		root->AddChild(s);
		//generate point light
		GeneratePointLight(size * 2.0f, Vector3(x, y + 2.0f * size, z));
	}
}

void Renderer::GenerateNodeLists(SceneNode* from) {
	////Render only the meshes within the field of view
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (auto i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		GenerateNodeLists((*i));
	}
}

void Renderer::SortNodeList() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(),
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(),
		SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::InitBufferFBO() {
	//generate depth texture
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	//generate colour texture
	for (int i = 0; i < SPLIT_SCREEN_NUM; ++i) {
		GenerateScreenTexture(bufferColourTex[i]);
	}
	GenerateScreenTexture(bufferNormalTex);
	//generate buffer FBO
	glGenFramebuffers(1, &bufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glDrawBuffer(GL_COLOR_ATTACHMENT1);
	//check FBO attachment success using this command
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//post process
void Renderer::InitPostProcess() {
	glGenFramebuffers(1, &processFBO);
}


void Renderer::DrawSceneWithPostProcess() {
	//draw scene on the buffer
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	viewMatrix = camera->BuildViewMatrix();
	UpdateShaderMatrices();
	DrawScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (dRendering) {
		DrawPointLights(camera->GetPosition());
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		CombineBuffers(bufferColourTex[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	ExecutePostProcess();
	PresentScene();
}

void Renderer::ExecutePostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
	//determine long distance
	int distJudge = height/2;
	for (int i = 0; i < POST_PASSES; ++i) {
		distJudge -= height / (2 * POST_PASSES);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "distJudge"), distJudge);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();
		//swap the colour buffers , and do the second blur pass
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(textureShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(textureShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}

//split screen
void Renderer::InitSplitScreen() {
	if (!heightMap) {
		return;
	}
	//generate view vertex
	Vector3 mapSize = heightMap->GetHeightmapSize();
	screenViewPosition[0] = mapSize * Vector3(0.5f, 2.0f, 0.0f);
	screenViewPosition[1] = mapSize * Vector3(1.0f, 2.0f, 0.5f);
	screenViewPosition[2] = mapSize * Vector3(1.0f, 2.0f, 1.0f);
	screenViewPosition[3] = mapSize * Vector3(0.0f, 2.0f, 0.5f);
}

void Renderer::DrawSceneOnSplitScreen() {
	if (!splitScreen) {
		return;
	}
	//draw
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	for (int i = 0; i < SPLIT_SCREEN_NUM; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[i], 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		viewMatrix = Matrix4::BuildViewMatrix(screenViewPosition[i], heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f));
		UpdateShaderMatrices();
		DrawScene();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (dRendering) {
		for (int i = 0; i < SPLIT_SCREEN_NUM; i++) {
			viewMatrix = Matrix4::BuildViewMatrix(screenViewPosition[i], heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f));
			DrawPointLights(screenViewPosition[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[i], 0);
			CombineBuffers(bufferColourTex[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

void Renderer::DrawSplitScreen() {
	if (!splitScreen) {
		return;
	}
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(textureShader);
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	modelMatrix.SetScalingVector(Vector3(0.5f, 0.5f, 0.5f));
	for (int i = 0; i < SPLIT_SCREEN_NUM; i++) {
		viewMatrix.ToIdentity();
		viewMatrix.SetPositionVector(screenPositions[i]);
		UpdateShaderMatrices();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glUniform1i(glGetUniformLocation(textureShader->GetProgram(), "diffuseTex"), 0);
		quad->Draw();
	}
}

void Renderer::InitDeferredRendering() {
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);
	glGenFramebuffers(1, &deferRenderFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, deferRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::GeneratePointLight(float redius, Vector3 position) {
	//rand colour
	Vector4 colour = Vector4(POINT_LIGHT_COLOUR_BASE + (float)(rand() / (float)RAND_MAX), POINT_LIGHT_COLOUR_BASE + (float)(rand() / (float)RAND_MAX), POINT_LIGHT_COLOUR_BASE + (float)(rand() / (float)RAND_MAX), 1);
	pointLights.push_back(new Light(position, colour, redius));
}

void Renderer::DrawPointLights(Vector3 viewPos) {
	glBindFramebuffer(GL_FRAMEBUFFER, deferRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	BindShader(pointlightShader);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(pointlightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glUniform1i(glGetUniformLocation(pointlightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);
	glUniform3fv(glGetUniformLocation(pointlightShader->GetProgram(), "cameraPos"), 1, (float*)&viewPos);
	glUniform2f(glGetUniformLocation(pointlightShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);
	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();
	for (auto l : pointLights) {
		SetShaderLight(*l);
		sphere->Draw();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CombineBuffers(GLuint tex) {
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	quad->Draw();
}

void Renderer::DrawSceneWithPointLight() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	viewMatrix = camera->BuildViewMatrix();
	UpdateShaderMatrices();
	DrawScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	DrawPointLights(camera->GetPosition());
	CombineBuffers(bufferColourTex[0]);
}

void Renderer::InitParticleEmitter() {
	GenerateScreenTexture(particleTex);
	flameParticleEmitter = new ParticleEmitter();
	flameParticleEmitter->SetParticleSize(1.0f);
	SetShaderParticleSize(flameParticleEmitter->GetParticleSize());
}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(particleShader->GetProgram(), "particleSize"), f);
}

void Renderer::DrawParticle() {
	glBindFramebuffer(GL_FRAMEBUFFER, deferRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, particleTex, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	BindShader(particleShader);

	glUniform1i(glGetUniformLocation(particleShader->GetProgram(), "diffuseTex"), 0);
	flameParticleEmitter->SetParticleSize(1.0f);
	SetShaderParticleSize(flameParticleEmitter->GetParticleSize());

	projMatrix = defaultView;
	Vector3 mapSize = heightMap->GetHeightmapSize();
	modelMatrix = Matrix4::Translation(mapSize * Vector3(0.5f, 0.8f, 0.5f)) * Matrix4::Scale(Vector3(5, 5, 5));

	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	flameParticleEmitter->Draw();
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}