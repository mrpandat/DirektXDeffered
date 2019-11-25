#include "stdafx.h"
#include "Shader.h"

// bool CompileShader(LPCWSTR pFileName, bool bPixel, LPCSTR pEntrypoint, ID3DBlob** ppCompiledShader)

Shader::Shader(ShaderType type, LPCWSTR filepath)
    : m_type(type)
{
    ID3DBlob* pErrorMsg = NULL;
    HRESULT hr;

    switch (type)
    {
    case ShaderType::Vertex:
        hr = D3DCompileFromFile(filepath, NULL, NULL, "VS_main", "vs_5_0", 0, 0, &m_shaderBlob, &pErrorMsg);
        break;
    case ShaderType::Pixel:
        hr = D3DCompileFromFile(filepath, NULL, NULL, "PS_main", "ps_5_0", 0, 0, &m_shaderBlob, &pErrorMsg);
        break;
    default:
        throw std::runtime_error("Unknown shader type specified.");
    }

    if (FAILED(hr))
    {
        if (pErrorMsg != NULL)
        {
            OutputDebugStringA((char*)pErrorMsg->GetBufferPointer());
            pErrorMsg->Release();
        }
        throw std::runtime_error("Shader compilation failed. Please check if HLSL file path is correct or check compiler output.");
    }

    if (pErrorMsg)
        pErrorMsg->Release();
}

ShaderType Shader::getType()
{
    return m_type;
}

ID3DBlob * Shader::getBlob()
{
    return m_shaderBlob;
}


Shader::~Shader()
{
    m_shaderBlob->Release();
}
