#pragma once

#include "stdafx.h"

#include "Shader.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"

enum class BufferType
{
    Vertex = 1,
    Index = 2,
    Constant = 3
};

static D3D11_INPUT_ELEMENT_DESC INPUT_LAYOUT_DESC[] = {
    { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 28,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

class Device
{
public:
    Device(int width, int height, const HWND & winWindow);
    void CreateDefaultRT();

    void InitRasterizerState();
    void InitInputLayout(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);
    void InitViewports();
    
    void UpdateRenderTargets();
    void UpdateConstantBuffer(int bufferPos, void* data);
    void ClearViews(float backgroundColor[4]);
    void Present();

    void AddViewport(int width, int height, int topLeftX, int topLeftY);

    void                    SetShader(Shader* shader);
    Shader*                 GetShader(ShaderType type);

    ID3D11VertexShader * GetVertexShader();
    ID3D11PixelShader * GetPixelShader();

    ID3D11DeviceContext* GetContext() { return m_deviceContext; }

    ID3D11Buffer * const GetBuffer(BufferType type, int pos);
    void AddBuffer(BufferType type, const size_t typeLength, void * data);
    void SetBuffer(BufferType type, const size_t typeLength, void* data);

    ~Device();

private:
    int                         m_windowWidth;
    int                         m_windowHeight;
    ID3D11Device *              m_device;
    ID3D11DeviceContext *       m_deviceContext;
    IDXGISwapChain *            m_swapChain;
    ID3D11Texture2D *           m_backBuffer;
    ID3D11RenderTargetView*     m_renderTargetView;
    ID3D11Texture2D*	        m_depthStencilBuffer;
    ID3D11DepthStencilView*	    m_depthStencilView;
    ID3D11RasterizerState*      m_rasterizerState;
    ID3D11InputLayout*          m_inputLayout;

    Shader*                     m_vertexShaderObject;
    Shader*                     m_pixelShaderObject;

    ID3D11VertexShader*         m_vertexShader;
    ID3D11PixelShader*          m_pixelShader;

    std::vector<ID3D11Buffer*>  m_constantBuffers;
    std::vector<ID3D11Buffer*>  m_vertexBuffers;
    ID3D11Buffer*               m_indexBuffer;

    std::vector<D3D11_VIEWPORT> m_viewports;

    ID3D11Buffer * CreateBuffer(BufferType type, const size_t typeLength, void * data);
};

