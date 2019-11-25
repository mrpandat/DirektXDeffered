#include "stdafx.h"
#include "Device.h"


Device::Device(int width, int height, const HWND & winWindow)
    : m_windowWidth     (width)
    , m_windowHeight    (height)
    , m_constantBuffers (std::vector<ID3D11Buffer*>())
    , m_vertexBuffers   (std::vector<ID3D11Buffer*>())
{
    UINT Flags = D3D11_CREATE_DEVICE_DEBUG;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_windowWidth;
    sd.BufferDesc.Height = m_windowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;            // 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = winWindow;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, Flags, NULL, 0, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, NULL, &m_deviceContext);
    if (FAILED(hr))
        throw std::runtime_error("Erreur lors de la création du device DirectX 11.");

    ImGui_ImplDX11_Init(winWindow, m_device, m_deviceContext);
}

void Device::CreateDefaultRT()
{
    ID3D11Texture2D*    pBackBuffer;
    if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
        throw std::runtime_error("Erreur lors de la création des render targets.");

    HRESULT hr = m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
    pBackBuffer->Release();

    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = m_windowWidth;
    descDepth.Height = m_windowHeight;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_device->CreateTexture2D(&descDepth, NULL, &m_depthStencilBuffer);
    if (FAILED(hr))
        throw std::runtime_error("Erreur lors de la création des render targets.");

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_device->CreateDepthStencilView(m_depthStencilBuffer, &descDSV, &m_depthStencilView);
    if (FAILED(hr))
        throw std::runtime_error("Erreur lors de la création des render targets.");
}

void Device::InitRasterizerState()
{
    D3D11_RASTERIZER_DESC oDesc;
    ZeroMemory(&oDesc, sizeof(D3D11_RASTERIZER_DESC));
    oDesc.FillMode = D3D11_FILL_SOLID;
    oDesc.CullMode = D3D11_CULL_NONE;

    m_device->CreateRasterizerState(&oDesc, &m_rasterizerState);
    m_deviceContext->RSSetState(m_rasterizerState);
}

void Device::InitInputLayout(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    HRESULT hr = m_device->CreateInputLayout(INPUT_LAYOUT_DESC, ARRAYSIZE(INPUT_LAYOUT_DESC), GetShader(ShaderType::Vertex)->getBlob()->GetBufferPointer(),
        GetShader(ShaderType::Vertex)->getBlob()->GetBufferSize(), &m_inputLayout);

    //Set the Input Layout
    m_deviceContext->IASetInputLayout(m_inputLayout);

    //Set Primitive Topology
    m_deviceContext->IASetPrimitiveTopology(primitiveTopology);
}

void Device::AddViewport(int width, int height, int topLeftX, int topLeftY)
{
    //Create the Viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = topLeftX;
    viewport.TopLeftY = topLeftY;

    m_viewports.push_back(viewport);
}

void Device::InitViewports()
{
    //Set the Viewport
    m_deviceContext->RSSetViewports(m_viewports.size(), &m_viewports[0]);
}

void Device::UpdateRenderTargets()
{
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

void Device::UpdateConstantBuffer(int bufferPos, void* data)
{
    m_deviceContext->UpdateSubresource(m_constantBuffers[bufferPos], 0, NULL, &data, 0, 0);
    m_deviceContext->VSSetConstantBuffers(0, m_constantBuffers.size(), &m_constantBuffers[0]);
}

void Device::ClearViews(float backgroundColor[4])
{
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, backgroundColor);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Device::Present()
{
    m_swapChain->Present(0, 0);
}

void Device::SetShader(Shader * shader)
{
    switch (shader->getType())
    {
    case ShaderType::Vertex:
        m_vertexShaderObject = shader;
        m_device->CreateVertexShader(shader->getBlob()->GetBufferPointer(), shader->getBlob()->GetBufferSize(), NULL, &m_vertexShader);
        m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
        break;
    case ShaderType::Pixel:
        m_pixelShaderObject = shader;
        m_device->CreatePixelShader(m_pixelShaderObject->getBlob()->GetBufferPointer(), m_pixelShaderObject->getBlob()->GetBufferSize(), NULL, &m_pixelShader);
        m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
        break;
    default:
        throw std::runtime_error("Unknown shader type specified.");
    }
}

Shader* Device::GetShader(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        return m_vertexShaderObject;
    case ShaderType::Pixel:
        return m_pixelShaderObject;
    default:
        throw std::runtime_error("Unknown shader type specified.");
    }
}

ID3D11VertexShader* Device::GetVertexShader()
{
    return m_vertexShader;
}

ID3D11PixelShader* Device::GetPixelShader()
{
    return m_pixelShader;
}

ID3D11Buffer* Device::CreateBuffer(BufferType type, const size_t typeLength, void* data)
{
    ID3D11Buffer* pBuffer;
    D3D11_BIND_FLAG bindFlags;
    D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE;

    switch (type)
    {
    case BufferType::Vertex:
        bindFlags = D3D11_BIND_VERTEX_BUFFER;
        break;
    case BufferType::Index:
        bindFlags = D3D11_BIND_INDEX_BUFFER;
        break;
    case BufferType::Constant:
        bindFlags = D3D11_BIND_CONSTANT_BUFFER;
        usage = D3D11_USAGE_DEFAULT;
        break;
    default:
        throw std::runtime_error("Unknown buffer type specified.");
    }

    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = typeLength;
    bufferDesc.BindFlags = bindFlags;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    bufferDesc.Usage = usage;

    D3D11_SUBRESOURCE_DATA bufferData;
    ZeroMemory(&bufferData, sizeof(D3D11_SUBRESOURCE_DATA));
    bufferData.pSysMem = data;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &bufferData, &pBuffer);

    return pBuffer;
}

void Device::AddBuffer(BufferType type, const size_t typeLength, void* data)
{
    ID3D11Buffer* pBuffer = CreateBuffer(type, typeLength, data);
    UINT stride;
    size_t bufferSize;
    void* bufferData;

    switch (type)
    {
    case BufferType::Vertex:
        m_vertexBuffers.push_back(pBuffer);
        stride = m_vertexBuffers.size() * typeLength;
        bufferSize = m_vertexBuffers.size();
        bufferData = malloc(bufferSize * sizeof (ID3D11Buffer*));
        memcpy(bufferData, &m_vertexBuffers[0], sizeof(ID3D11Buffer*) * bufferSize);
        m_deviceContext->IASetVertexBuffers(0, m_vertexBuffers.size(), (ID3D11Buffer** const)bufferData, &stride, 0);
        free(bufferData);
        break;
    case BufferType::Index:
        m_indexBuffer = pBuffer;
        m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        break;
    case BufferType::Constant:
        m_constantBuffers.push_back(pBuffer);
        stride = m_constantBuffers.size() * typeLength;
        m_deviceContext->VSSetConstantBuffers(0, m_constantBuffers.size(), &m_constantBuffers[0]);
        break;
    default:
        throw std::runtime_error("Unknown buffer type specified.");
    }
}

void Device::SetBuffer(BufferType type, const size_t typeLength, void* data)
{
    switch (type)
    {
    case BufferType::Vertex:
        m_vertexBuffers.clear();
        AddBuffer(type, typeLength, data);
        break;
    case BufferType::Index:
        AddBuffer(type, typeLength, data);
        break;
    case BufferType::Constant:
        m_constantBuffers.clear();
        AddBuffer(type, typeLength, data);
        break;
    default:
        throw std::runtime_error("Unknown buffer type specified.");
    }
}

ID3D11Buffer* const Device::GetBuffer(BufferType type, int pos)
{
    void** data;
    ID3D11Buffer* buffer;

    switch (type)
    {
    case BufferType::Vertex:
        buffer = m_vertexBuffers[pos];
        break;
    case BufferType::Index:
        buffer = m_indexBuffer;
        break;
    case BufferType::Constant:
        buffer = m_constantBuffers[pos];
        break;
    default:
        throw std::runtime_error("Unknown buffer type specified.");
    }

    return buffer;
}

Device::~Device()
{
    delete m_vertexShaderObject;
    delete m_pixelShaderObject;
    m_vertexShader->Release();
    m_pixelShader->Release();

    m_renderTargetView->Release();
    m_depthStencilView->Release();
    m_depthStencilBuffer->Release();
    m_rasterizerState->Release();
    m_deviceContext->Release();
    m_swapChain->Release();
    m_device->Release();
}
