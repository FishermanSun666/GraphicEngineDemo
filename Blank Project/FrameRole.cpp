//#include "FrameRole.h"
//
//FrameRole::FrameRole(HeightMap* map) {
//	heightMap = map;
//	mesh = Mesh::LoadFromMeshFile("Role_T.msh");
//	anim = new MeshAnimation("Role_T.anm");
//	material = new MeshMaterial("Role_T.mat");
//	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
//		const MeshMaterialEntry* matEntry =
//			material->GetMaterialForLayer(i);
//		const string* filename = nullptr;
//		matEntry->GetEntry("Diffuse", &filename);
//		string path = TEXTUREDIR + *filename;
//		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO,
//			SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
//		matTextures.emplace_back(texID);
//	}
//	currentFrame = 0;
//	frameTime = 0.0f;
//	rolePosition = Vector3(ROLE_POS_X, 0.0f, ROLE_POS_Z) * heightMap->GetHeightmapSize();
//	rolePosition.y = heightMap->GetHeight(rolePosition.x, rolePosition.z);
//}
//
//void FrameRole::Update(float dt) {
//	//role move
//	if (rolePosition.z >= heightMap->GetHeightmapSize().z * ROLE_MOVE_MAX || rolePosition.z < heightMap->GetHeightmapSize().z * ROLE_POS_Z) {
//		roleDir = !roleDir;
//	}
//	if (roleDir) {
//		rolePosition.z -= ROLE_MOVE_SPEED * dt;
//	}
//	else {
//		rolePosition.z += ROLE_MOVE_SPEED * dt;
//	}
//	rolePosition.y = heightMap->GetHeight(rolePosition.x, rolePosition.z);
//	//role frame
//	frameTime -= dt;
//	while (frameTime < 0.0f) {
//		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
//		frameTime += 1.0f / anim->GetFrameRate();
//	}
//}