#include"HeightMap.h"
#include<iostream>

using namespace std;

HeightMap::HeightMap(const std::string& name) {
	int iWidth, iHeight, iChans;
	unsigned char* data = SOIL_load_image(name.c_str(),
		&iWidth, &iHeight, &iChans, 1); //从纹理中加载，最后一个参数中的 "1 "将强制SOIL给我们提供单通道格式的数据
	if (!data) {
		std::cout << "Heightmap can't load file!\n";
		return;
	}
	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	/*Vector3 vertexScale = Vector3(16.0f, 1.0f, 16.0f);*/
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);
	for (int z = 0; z < iHeight; ++z) {
		for (int x = 0; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;
			auto vertex = Vector3(x, data[offset] * 1.8, z) * g_vertexScale;
			vertices[offset] = vertex;	//2维数组转变为1维
			textureCoords[offset] = Vector2(x, z) * textureScale;
			//y index
			heights[vertex.x][vertex.z] = vertex.y;
		}
	}
	SOIL_free_image_data(data); //删除原始图层数据,返回的是一个单一的内存分配
	int i = 0;

	for (int z = 0; z < iHeight - 1; ++z) {
		for (int x = 0; x < iWidth - 1; ++x) {
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = ((z + 1) * (iWidth))+(x + 1);
			int d = ((z + 1) * (iWidth)) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}
	//light
	GenerateNormals();
	//tangent mapping
	GenerateTangents();

	BufferData();
	
	heightmapSize.x = g_vertexScale.x * (iWidth - 1);
	heightmapSize.y = g_vertexScale.y * 255.0f; //each height is a byte!
	heightmapSize.z = g_vertexScale.z * (iHeight - 1);
}

float HeightMap::GetHeight(float x, float z) {
	float xi = (float)(MAP_SCALE * ((int)x / MAP_SCALE + 1));
	if (heights.find(xi) == heights.end()) {
		return 0;
	}
	auto values = heights[xi];
	float zi = (float)(MAP_SCALE * ((int)z / MAP_SCALE));
	return values[zi];
}