#include"Camera.h"
#include"Window.h"
#include<algorithm>
void Camera::UpdateCamera(float dt) {
	//auto
	auto pMove = Window::GetMouse()->GetRelativePosition().y;
	auto yMove = Window::GetMouse()->GetRelativePosition().x;
	if (0 == pMove && 0 == yMove) {
		pMove = autoPDir * dt;
		yMove = autoYDir * dt;
	}
	pitch -= pMove;
	yaw -= yMove;

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	ChangeAutoView();

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	//auto default speed;
	float deSpeed = 2.0f;
	float zSpeed = CAMERA_Z_SPEED * dt;
	float xSpeed = CAMERA_X_SPEED * dt;
	float ySpeed = CAMERA_Y_SPEED * dt;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * zSpeed;
	}
	else {
		position += forward * deSpeed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * zSpeed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * xSpeed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * xSpeed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += ySpeed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= ySpeed;
	}
}

Matrix4 Camera::BuildViewMatrix() {
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
}

void Camera::ChangeAutoView() {
	if (-20 <= pitch) {
		autoPDir = g_camAutoPSp;
	}
	if (-30 >= pitch) {
		autoPDir = -g_camAutoPSp;
	}
	autoYDir = 180 <= yaw ? g_camAutoYSp : -g_camAutoYSp;
}

