#include "RoleNode.h"

AnimatedNode::AnimatedNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, vector<GLuint> matTextures, HeightMap* map) : SceneNode(mesh) {
	if (!anim || !material || !map || 0 == matTextures.size()) {
		return;
	}
	//init
	currentFrame = 0;
	frameTime = 0.0f;
	this->anim = anim;
	this->material = material;
	this->matTextures = matTextures;
	heightMap = map;
	//Keeping the role on the ground
	float px = ROLE_POS_X * heightMap->GetHeightmapSize().x;
	float pz = ROLE_POS_Z* heightMap->GetHeightmapSize().z;
	worldTransform.SetPositionVector(Vector3(px, heightMap->GetHeight(px, pz), pz));
	modelScale = Vector3(ROLE_SCALE, ROLE_SCALE, ROLE_SCALE);
}

void AnimatedNode::Update(float dt) {
	//role turn
	Vector3 position = worldTransform.GetPositionVector();
	if (position.z > heightMap->GetHeightmapSize().z * ROLE_MOVE_MAX || position.z < heightMap->GetHeightmapSize().z * ROLE_POS_Z - 1) {
		worldTransform = worldTransform.Rotation(90.0f + 90.0f*direction, Vector3(0.0f, 1.0f, 0.0f));
		direction *= -1;
	}
	//move
	position.z += ROLE_MOVE_SPEED * dt * direction;
	position.y = heightMap->GetHeight(position.x, position.z);
	worldTransform.SetPositionVector(position);
	//role frame
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}
}

void AnimatedNode::Draw(OGLRenderer& r) {
	auto shader = r.GetCurrentShader();
	if (!shader) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "texture"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "animate"), true);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "transparent"), false);
	r.UpdateModelMatrix(worldTransform * Matrix4::Scale(modelScale));
	r.UpdateShaderMatrices();
	//join frame
	vector <Matrix4> frameMatrices;
	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = anim->GetJointData(currentFrame);
	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}
	int j = glGetUniformLocation(shader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		mesh->DrawSubMesh(i);
	}
}