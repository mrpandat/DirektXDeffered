#pragma once

struct Vertex
{
	Vertex()
	{}

	Vertex(float x, float y, float z)
		: position(x, y, z)
		, color(255.0f, 255.0f, 255.0f, 1.0f)
	{}

	Vertex(float x, float y, float z, float cr, float cg, float cb, float ca)
		: position(x, y, z)
		, color(cr, cg, cb, ca)
	{}

	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector4 color;
};

struct cbPerObject
{
	DirectX::SimpleMath::Matrix wvp;
};