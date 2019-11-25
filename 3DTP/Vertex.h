#pragma once

struct Vertex
{
    Vertex()
    {}

    Vertex(float x, float y, float z)
        : position(x, y, z)
        , color(255.0f, 255.0f, 255.0f, 1.0f)
    {}

    Vertex(float x, float y, float z,
        float cr, float cg, float cb, float ca,
        float nx, float ny, float nz)
        : position(x, y, z)
        , color(cr, cg, cb, ca)
        , normal(nx, ny, nz)
    {}

    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector4 color;
    DirectX::SimpleMath::Vector3 normal;
};