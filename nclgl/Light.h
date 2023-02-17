#pragma once

#include"Vector4.h"
#include"Vector3.h"
#include"Matrix4.h"

#define LIGHT_BREATH_REDUCE -0.3f
#define LIGHT_BREATH_INCREASE 1.0f
#define LIGHT_BREATH_MAX 1.0f
#define LIGHT_BREATH_MIN 0.5f

class Light {
public:
	Light(){}
	Light(const Vector3& position, const Vector4& colour, float radius) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
		radiusBase = radius;
		defaultPos = position;
	}

	~Light(void) {};
	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }
	void Rotation(float dt, const Vector3 normal) {
		position = Matrix4::Rotation(dt, normal)*position;
	}

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }
	
	void ResetPosition() { position = defaultPos; }
	void Breath(float dt) {
		if (LIGHT_BREATH_MAX <= breathFactor) {
			breathFactor = LIGHT_BREATH_MAX;
			breathOffSet = LIGHT_BREATH_REDUCE;
		}
		if (LIGHT_BREATH_MIN >= breathFactor) {
			breathFactor = LIGHT_BREATH_MIN;
			breathOffSet = LIGHT_BREATH_INCREASE;
		}
		breathFactor += breathOffSet * dt;
		radius = radiusBase * breathFactor;
	}
	void Night() {
		if (0.0f < position.y) {
			position.y = -position.y;
		}
	}
protected:
	Vector3 position;
	float radius;
	Vector4 colour;
	Vector3 defaultPos;
	float radiusBase;
	float breathFactor = 1.0f;
	float breathOffSet = 0.1f;
};