#include"Camera.h"
#include"Window.h"
#include<algorithm>
void Camera::UpdateCamera(float dt) {
	CaptureYaw();
	CapturePitch();
	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);
	UpdatePosition(forward, right, CAMERA_SPEED * dt);
}

Matrix4 Camera::BuildViewMatrix() {
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
}

void Camera::UpdatePosition(Vector3 forward, Vector3 right, float pace) {
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * pace;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * pace;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * pace;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * pace;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += pace;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= pace;
	}
}

float Camera::CaptureYaw() {
	yaw -= Window::GetMouse()->GetRelativePosition().x;
	if (yaw < 0.0f) {
		while(yaw < 0.0f) {
			yaw += 360.0f;
		}
	}
	if (yaw > 360.0f) {
		while (yaw > 360.0f) {
			yaw -= 360.0f;
		}
	}
	return yaw;
}

float Camera::CapturePitch() {
	pitch -= Window::GetMouse()->GetRelativePosition().y;
	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);
	return pitch;
}
