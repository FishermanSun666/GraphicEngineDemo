#include "RoleNode.h"

RoleNode::RoleNode(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, vector<GLuint> matTextures) : SceneNode(mesh) {
	if (!anim || !material || 0 == matTextures.size()) {
		return;
	}
	//init
	currentFrame = 0;
	frameTime = 0.0f;
	this->anim = anim;
	this->material = material;
	this->matTextures = matTextures;
}

void RoleNode::Update(float dt) {
	if (DEFAULT_VIEW_MODE != viewMode) {
		return;
	}
	//role turn
	moveTime += dt;
	Vector3 position = worldTransform.GetPositionVector();
	
	if (moveTime >= (float)ROLE_MOVE_TIME) {
		direction *= -1;
		moveTime = 0.0f;
	}
	worldTransform = worldTransform.Rotation(90.0f - 90.0f * direction, Vector3(0.0f, 1.0f, 0.0f));
	//move
	position.z += ROLE_MOVE_SPEED * dt * direction;
	position.y = heightMap->GetHeight(position.x, position.z);
	worldTransform.SetPositionVector(position);
	//role frame
	ExecuteFrame(dt);
}

void RoleNode::ExecuteFrame(float dt) {
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}
}

void RoleNode::Draw(OGLRenderer& r) {
	if (FIRST_PERSON_VIEW_MODE == viewMode) {
		return;
	}
	auto shader = r.GetCurrentShader();
	if (!shader) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "animate"), true);
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
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "animate"), false);
}

void RoleNode::DrawShadow(OGLRenderer& r) {
	auto shader = r.GetCurrentShader();
	if (!shader) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "animate"), true);
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
		mesh->DrawSubMesh(i);
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "animate"), false);
}

void RoleNode::Move(float dt, float dir) {
	Matrix4 rotation = Matrix4::Rotation(dir, Vector3(0, 1, 0));
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);
	Vector3 position = worldTransform.GetPositionVector();
	float pace = ROLE_MOVE_SPEED * dt;
	bool move = false;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * pace;
		move = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * pace;
		move = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * pace;
		move = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * pace;
		move = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += pace;
		move = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= pace;
		move = true;
	}
	if (move) {
		position.y = heightMap->GetHeight(position.x, position.z);
		worldTransform.SetPositionVector(position);
		ExecuteFrame(dt);
	}
	
}