#include "Plane.h"

//@param normal - 法线
//@param distance - 与原点的距离
//@param normalise - 是否归一化
Plane::Plane(const Vector3& normal, float distance, bool normalise) {
	if (normalise) {
		float length = sqrt(Vector3::Dot(normal, normal));

		this->normal = normal / length;
		this->distance = distance / length;
	}
	else {
		this->normal = normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3& position, float radius) const {
	if (Vector3::Dot(position, normal) + distance <= -radius) {
		return false;
	}
	return true;
}