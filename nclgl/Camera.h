#pragma once
#include"Matrix3.h"
#include"Vector3.h"


enum CameraSpeed {
	CAMERA_Z_SPEED = 200,
	CAMERA_X_SPEED = 200,
	CAMERA_Y_SPEED = 400,
	
};

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

	void ChangeAutoView();

protected:
	const float g_camAutoPSp = 2.0f;
	const float g_camAutoYSp = 5.0f;

	float autoPDir = g_camAutoPSp;
	float autoYDir = g_camAutoYSp;

	float yaw;
	float pitch;
	Vector3 position; //Set to 0,0,0 by Vector3 constructor ;)
};