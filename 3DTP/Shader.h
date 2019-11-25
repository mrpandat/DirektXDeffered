#pragma once

#include "stdafx.h"

#include "D3Dcompiler.h"

enum class ShaderType
{
    Vertex  = 1,
    Pixel   = 2
};

class Shader
{
public:
    Shader(ShaderType type, LPCWSTR filepath);
    ShaderType getType();
    ID3DBlob * getBlob();
    ~Shader();
private:
    ShaderType m_type;
    ID3DBlob * m_shaderBlob;
};

