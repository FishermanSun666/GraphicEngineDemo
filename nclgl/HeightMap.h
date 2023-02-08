#pragma once
#include<string>
#include<map>
#include"Mesh.h"

const enum {
	MAP_SCALE = 16,
};

const Vector3 g_vertexScale = Vector3((float)MAP_SCALE, 1.0f, (float)MAP_SCALE);

class HeightMap : public Mesh {
public:
	HeightMap(const std::string& name);
	~HeightMap(void) {};

	Vector3 GetHeightmapSize() const { return heightmapSize; }
	float HeightMap::GetHeight(float x, float z);
protected:
	Vector3 heightmapSize;
	std::map<float, std::map<float, float>> heights;
};