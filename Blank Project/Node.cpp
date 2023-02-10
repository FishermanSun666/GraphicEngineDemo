#include"Renderer.h"

void Renderer::LoadNodes(void) {
	root = new SceneNode();
	LoadRole();
	LoadTrees();
	//LoadCloud();
}

void Renderer::LoadRole() {
	Mesh* roleMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	auto anim = new MeshAnimation("Role_T.anm");
	auto material = new MeshMaterial("Role_T.mat");
	vector<GLuint> matTextures;
	//load sub mesh
	for (int i = 0; i < roleMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry =	material->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}
	RoleNode* role = new RoleNode(roleMesh, anim, material, matTextures, heightMap, sceneShader);
	if (role) {
		root->AddChild(role);
	}
}

void Renderer::LoadTrees() {
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
	if (!root || !heightMap || !sceneShader) {
		return;
	}
	Vector3 mapSize = heightMap->GetHeightmapSize();
	float size = 0;
	for (int i = 0; i < TREE_NUM; i++) {
		TreeNode* s = new TreeNode(textures, sceneShader);
		size = rand() % (TREE_RANGE);
		float nSize = size + TREE_MIN;
		float nx = (rand() + TREE_MIN_SPACE) % (int)mapSize.x * 0.7f;
		float nz = (rand() + TREE_MIN_SPACE) % (int)mapSize.z;
		s->SetTransform(Matrix4::Translation(Vector3(nx, heightMap->GetHeight(nx, nz), nz)));
		s->SetModelScale(Vector3(nSize, nSize, nSize));
		s->SetBoundingRadius(20000.0f);
		s->SetMesh(mesh);
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

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		n->Draw(*this);
	}
}

void Renderer::DrawNodes() {
	BindShader(sceneShader);
	UpdateShaderMatrices();
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
	ClearNodeLists();
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