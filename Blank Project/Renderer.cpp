#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	//initialize
	quadMesh = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.65f, 3.0f, 0.65f));
	light = new Light(heightmapSize * Vector3(0.5f, 2.0f, 0.0f), Vector4(2.55f, 2.46f, 1.45f, 1), heightmapSize.x);
	//load texture
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	muddyTex = SOIL_load_OGL_texture(TEXTUREDIR"muddy.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sunTex = SOIL_load_OGL_texture(TEXTUREDIR"sun_texture.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//cubeMap = SOIL_load_OGL_cubemap(
	//	TEXTUREDIR"blizzard_rt.jpg", TEXTUREDIR"blizzard_lf.jpg",
	//	TEXTUREDIR"blizzard_up.jpg", TEXTUREDIR"blizzard_dn.jpg",
	//	TEXTUREDIR "blizzard_bk.jpg", TEXTUREDIR"blizzard_ft.jpg",
	//	SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	cubeMap = SOIL_load_OGL_cubemap(
		SKYBOXDIR"rusted_rt.jpg", SKYBOXDIR"rusted_lf.jpg",
		SKYBOXDIR"rusted_up.jpg", SKYBOXDIR"rusted_dn.jpg",
		SKYBOXDIR "rusted_bk.jpg", SKYBOXDIR"rusted_ft.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!earthTex || !earthBump || !cubeMap || !waterTex || !sunTex || !muddyTex ) {
		return;
	}
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(muddyTex, true);
	SetTextureRepeating(sunTex, true);
	
	sceneShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	mapShader = new Shader("MapVertex.glsl", "MapFragment.glsl");
	shadowShader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	
	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess()|| !sceneShader->LoadSuccess() || !mapShader->LoadSuccess() || !shadowShader->LoadSuccess()){
		return;
	}
	//node tree
	CreateNodes();

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	UpdateShaderMatrices();
	//shadow
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
	glDeleteTextures(1, &cubeMap);
	glDeleteTextures(1, &waterTex);
	glDeleteTextures(1, &earthTex);
	glDeleteTextures(1, &earthBump);
	glDeleteTextures(1, &sunTex);
	glDeleteTextures(1, &muddyTex);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	if (camera) {
		delete camera;
	}
	if (light) {
		delete light;
	}
	if (quadMesh) {
		delete quadMesh;
	}
	//if (reflectShader) {
	//	delete reflectShader;
	//}
	//if (skyboxShader) {
	//	delete skyboxShader;
	//}
	//if (mapShader) {
	//	delete mapShader;
	//}
	//if (sceneShader) {
	//	delete sceneShader;
	//}	
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	light->Rotation(dt, heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f));
	//update frustum
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
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
	//node tree
	BuildNodeLists(root);
	SortNodeList();
	DrawNodeShadows();
	DrawHeightmap();
	DrawNodes();
	ClearNodeLists();
	DrawWater();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();
	quadMesh->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(mapShader);
	SetShaderLight(*light);
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "map"), true);

	glUniform3fv(glGetUniformLocation(mapShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "muddyTex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, muddyTex);

	UpdateShaderMatrices();
	heightMap->Draw();
	glUniform1i(glGetUniformLocation(mapShader->GetProgram(), "map"), false);
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

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	//SetShaderLight(*light);
	quadMesh->Draw();
}


void Renderer::DrawNodes() {
	//BindShader(mapShader);
	UpdateShaderMatrices();
	//set light;
	glUniform3fv(glGetUniformLocation(mapShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	SetShaderLight(*light);
	for (const auto& i : nodeList) {
		i->Draw(*this);
	}
	for (const auto& i : transparentNodeList) {
		i->Draw(*this);
	}
}



void Renderer::DrawNodeShadows() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(heightMap->GetHeightmapSize() * Vector3(0.5f, 0.0f, 0.5f)));
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	shadowMatrix = projMatrix * viewMatrix;
	for (const auto& i : nodeList) {
		i->DrawShadow(*this);
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CreateNodes(void) {
	root = new SceneNode();
	CreateAnimatedNodes();
	CreateMaterialNodes();
	CreateSimpleNodes();
	//LoadCloud();
}

void Renderer::CreateSimpleNodes() {
	if (!root) {
		return;
	}
	CreateSunNode();
}

void Renderer::CreateSunNode() {
	Mesh* sunMesh = Mesh::LoadFromMeshFile("Sphere.msh");
	auto node = new SunNode(sunMesh, light);
	//float px = ROLE_POS_X * heightMap->GetHeightmapSize().x;
	//float pz = ROLE_POS_Z * heightMap->GetHeightmapSize().z + 300;
	//node->SetTransform(Matrix4::Translation(Vector3(px, heightMap->GetHeight(px, pz) + 200, pz)));
	node->SetModelScale(Vector3(SUN_SIZE, SUN_SIZE, SUN_SIZE));
	node->SetColour(Vector4(2.55f, 2.36f, 1.39f, 0.4f));
	//node->SetBoundingRadius(200000.0f);
	node->SetTexture(sunTex);
	if (root) {
		root->AddChild(node);
	}
}

void Renderer::CreateAnimatedNodes() {
	if (!root) {
		return;
	}
	CreateRole();
}

void Renderer::CreateRole() {
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
	RoleNode* role = new RoleNode(roleMesh, anim, material, matTextures);
	role->SetHeightMap(heightMap);
	//Keeping the role on the ground
	float px = ROLE_POS_X * heightMap->GetHeightmapSize().x;
	float pz = ROLE_POS_Z * heightMap->GetHeightmapSize().z;
	role->SetPosition(Vector3(px, heightMap->GetHeight(px, pz), pz));
	role->SetModelScale(Vector3(ROLE_SCALE, ROLE_SCALE, ROLE_SCALE));
	role->SetBoundingRadius(2000.0f);
	if (root) {
		root->AddChild(role);
	}
}

void Renderer::CreateMaterialNodes() {
	if (!root) {
		return;
	}
	CreateTree();
}

void Renderer::CreateTree() {
	Mesh* treeMesh = Mesh::LoadFromMeshFile("Big_Tree.msh");
	MeshMaterial* material = new MeshMaterial("Big_Tree.mat");
	if (!treeMesh || !material) {
		return;
	}
	//load multi texture
	vector<GLuint> textures;
	for (int i = 0; i < treeMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		textures.emplace_back(texID);
	}
	//create tree
	CreateTrees(treeMesh, textures);
}

//Create trees of a certain density depending on the size of the map
//@param mesh - tree mesh
//@param textures - tree's multiple texture
void Renderer::CreateTrees(Mesh* mesh, vector<GLuint> textures) {
	if (!root || !heightMap ) {
		return;
	}
	Vector3 mapSize = heightMap->GetHeightmapSize();
	float sizeBase = 0;
	//random generate
	for (int i = 0; i < TREE_NUM; i++) {
		sizeBase = rand() % (TREE_RANGE);
		float nSize = sizeBase + TREE_MIN;
		float nx = (rand() + TREE_MIN_SPACE) % (int)mapSize.x;
		float nz = (rand() + TREE_MIN_SPACE) % (int)mapSize.z;
		float ny = heightMap->GetHeight(nx, nz);
		//no trees in the river
		if (ny <= WATER_HEIGHT) {
			if (0 < i) {
				i--;
			}
			continue;
		}

		MaterialNode* s = new MaterialNode(mesh, textures);
		s->SetTransform(Matrix4::Translation(Vector3(nx, ny, nz)));
		s->SetModelScale(Vector3(nSize, nSize, nSize));
		s->SetBoundingRadius(2000.0f);
		root->AddChild(s);
	}
}

void Renderer::BuildNodeLists(SceneNode* from) {
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
		BuildNodeLists((*i));
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

//void Renderer::LoadCloud() {
//	cloudMesh = Mesh::LoadFromMeshFile("Sphere.msh");
//	auto mapSize = heightMap->GetHeightmapSize();
//	float hCloud = mapSize.z * 0.9f;
//	float sCloud = 100.0f;
//	CloudTex = SOIL_load_OGL_texture(TEXTUREDIR"ground.png",
//		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
//	//SetTextureRepeating(CloudTex, true);
//
//	for (int i = 0; i < CLOUD_NUM; i++) {
//		SceneNode* s = new SceneNode();
//		float nx = rand() % (int)mapSize.x;
//		float nz = rand() % (int)mapSize.z;
//		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.3f));
//		s->SetTransform(Matrix4::Translation(Vector3(nx, hCloud, nz)));
//		s->SetModelScale(Vector3(sCloud + (float)(rand() % (int)100.0f), sCloud, sCloud + (float)(rand() % (int)100.0f)));
//		s->SetBoundingRadius(20000.0f);
//		s->SetTexture(CloudTex);
//		s->SetMesh(cloudMesh);
//		root->AddChild(s);
//	}
//}
// 
//void Renderer::DrawCloud(SceneNode* n) {
//	modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
//	UpdateShaderMatrices();
//	glUniform4fv(glGetUniformLocation(sceneShader->GetProgram(),
//		"nodeColour"), 1, (float*)&n->GetColour());
//	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(),
//		"cloudTex"), 2);
//	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(),
//		"shadeType"), 2);
//	glActiveTexture(GL_TEXTURE2);
//	glBindTexture(GL_TEXTURE_2D, CloudTex);
//	n->Draw(*this);
//}
