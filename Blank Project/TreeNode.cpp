#include "TreeNode.h"

void TreeNode::Draw(OGLRenderer& r) {
	if (!shader || !mesh) {
		return;
	}
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "treeTex"), 1);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "shadeType"), 1);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&this->GetColour());
	r.UpdateModelMatrix(worldTransform * Matrix4::Scale(modelScale));
	r.UpdateShaderMatrices();
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		mesh->DrawSubMesh(i);
	}
}