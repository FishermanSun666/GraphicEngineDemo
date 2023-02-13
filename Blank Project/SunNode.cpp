#include "SunNode.h"

void SunNode::Update(float dt) {
	angle += dt * 10.0f;
	if (angle >= 360.0f) {
		angle -= 360.0f;
	}
	if (!light) {
		return;
	}
	worldTransform.SetPositionVector(light->GetPosition());
	worldTransform.Rotation(angle, Vector3(1.0f, 1.0f, 1.0f));
}

void SunNode::Draw(OGLRenderer& r) {
	if (!mesh) {
		return;
	}
	auto shader = r.GetCurrentShader();
	if (!shader) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "transparent"), true);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&colour);
	r.UpdateModelMatrix(worldTransform * Matrix4::Scale(modelScale));
	r.UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	mesh->Draw();
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "transparent"), false);
}