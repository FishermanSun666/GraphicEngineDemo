#include "MaterialNode.h"

void MaterialNode::Draw(OGLRenderer& r) {
	if (!mesh) {
		return;
	}
	auto shader = r.GetCurrentShader();
	if (!shader) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	//glUniform4fv(glGetUniformLocation(shader->GetProgram(), "colour"), 1, (float*)&this->GetColour());
	r.UpdateModelMatrix(worldTransform * Matrix4::Scale(modelScale));
	r.UpdateShaderMatrices();
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		mesh->DrawSubMesh(i);
	}
}