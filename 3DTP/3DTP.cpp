#include "stdafx.h"
#include "3DTP.h"
#include "InputManager.h"
#include "D3Dcompiler.h"
#include "Camera.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"

// Global Variables:
HINSTANCE									hInst;			// current instance
HWND											hWnd;				// windows handle used in DirectX initialization
IAEngine::InputManager*		g_pInputManager = NULL;
IDXGISwapChain*						g_pSwapChain = NULL;
ID3D11Device*							g_pDevice = NULL;
ID3D11DeviceContext*			g_pImmediateContext = NULL;
ID3D11RenderTargetView*		g_pRenderTargetView = NULL;

ID3D11Texture2D*					g_pDepthStencilBuffer = NULL;
ID3D11DepthStencilView*		g_pDepthStencilView = NULL;

ID3D11Buffer* g_pVertexBuffer;
ID3D11Buffer* g_pIndexBuffer;

ID3D11Buffer* g_pCbPerObjectBuffer;

cbPerObject g_cbPerObject;

DirectX::SimpleMath::Matrix cube1World;
DirectX::SimpleMath::Matrix cube2World;

DirectX::SimpleMath::Matrix g_rotation;
DirectX::SimpleMath::Matrix g_scale;
DirectX::SimpleMath::Matrix g_translation;
float rot = 0.01f;

DirectX::SimpleMath::Matrix g_WVP;

DirectX::SimpleMath::Matrix g_camView;
DirectX::SimpleMath::Matrix g_camProjection;

DirectX::SimpleMath::Vector3 g_camPosition;
DirectX::SimpleMath::Vector3 g_camTarget;
DirectX::SimpleMath::Vector3 g_camUp;

ID3D11VertexShader* g_pVertexShader;
ID3D11PixelShader* g_pPixelShader;
ID3DBlob* g_pVertexBlob;
ID3DBlob* g_pPixelBlob;

ID3D11InputLayout* g_pInputLayout;

IAEngine::FreeCamera oFreeCamera;
ulong g_iLastTime;

// Descriptions
D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

// Forward declarations
bool				CreateWindows(HINSTANCE, int, HWND& hWnd);
bool				CreateDevice();
bool				CreateDefaultRT();
bool				CompileShader(LPCWSTR pFileName, bool bPixel, LPCSTR pEntrypoint, ID3DBlob** ppCompiledShader);//utiliser un L devant une chaine de caractère pour avoir un wchar* comme L"MonEffet.fx"

bool				InitScene();
void				UpdateScene();
void				DrawScene();

/*
Exemple of possible triangle coodinates in 3D
P0 0.0f, 1.0f, 0.0f,
P1 5.0f, 1.0f, 0.0f,
P2 5.0f, 1.0f, 5.0f,
*/
using namespace DirectX::SimpleMath;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG oMsg;

	hInst = hInstance;
	if (!CreateWindows (hInstance, nCmdShow, hWnd))
	{
		MessageBox(NULL, L"Erreur lors de la création de la fenêtre", L"Error", 0);
		return false;
	}
	g_pInputManager = new IAEngine::InputManager();
	if (!g_pInputManager->Create(hInst, hWnd))
	{
		MessageBox(NULL, L"Erreur lors de la création de l'input manager", L"Error", 0);
		delete g_pInputManager;
		return false;
	}
	if (!CreateDevice())
	{
		MessageBox(NULL, L"Erreur lors de la création du device DirectX 11", L"Error", 0);
		return false;
	}
	if (!CreateDefaultRT())
	{
		MessageBox(NULL, L"Erreur lors de la création des render targets", L"Error", 0);
		return false;
	}


	ID3D11RasterizerState* pRasterizerState;
	D3D11_RASTERIZER_DESC oDesc;
	ZeroMemory(&oDesc, sizeof(D3D11_RASTERIZER_DESC));
	oDesc.FillMode = D3D11_FILL_SOLID;
	oDesc.CullMode = D3D11_CULL_NONE;
	g_pDevice->CreateRasterizerState(&oDesc, &pRasterizerState);
	g_pImmediateContext->RSSetState(pRasterizerState);

	//Create and fill other DirectX Stuffs like Vertex/Index buffer, shaders

	InitScene();

	// END OF CUSTOM CODE

	g_iLastTime = timeGetTime();
	PeekMessage( &oMsg, NULL, 0, 0, PM_NOREMOVE );
	while ( oMsg.message != WM_QUIT )
	{
		if (PeekMessage( &oMsg, NULL, 0, 0, PM_REMOVE )) 
		{
			TranslateMessage( &oMsg );
			DispatchMessage( &oMsg );
		}
		else
		{
			UpdateScene();
			DrawScene();
		}
	}
	//Release D3D objects
	ImGui_ImplDX11_Shutdown();
	g_pRenderTargetView->Release();
	g_pDepthStencilView->Release();
	g_pDepthStencilBuffer->Release();
	pRasterizerState->Release();
	g_pImmediateContext->Release();
	g_pSwapChain->Release();
	g_pDevice->Release();
	g_pVertexBuffer->Release();
	g_pIndexBuffer->Release();
	g_pCbPerObjectBuffer->Release();
	delete g_pInputManager;
	return (int) oMsg.wParam;
}

bool CreateDevice()
{
	UINT Flags = D3D11_CREATE_DEVICE_DEBUG;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;// 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, Flags, NULL, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pDevice, NULL, &g_pImmediateContext);
	if (FAILED(hr))
		return false;
	ImGui_ImplDX11_Init(hWnd, g_pDevice, g_pImmediateContext);

	return true;
}

bool CreateDefaultRT()
{

	ID3D11Texture2D*	pBackBuffer;
	if (FAILED(g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
		return false;

	HRESULT hr = g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = WINDOW_WIDTH;
	descDepth.Height = WINDOW_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencilBuffer);
	if (FAILED(hr))
		return false;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return false;

	return true;
}


bool CompileShader(LPCWSTR pFileName, bool bPixel, LPCSTR pEntrypoint, ID3DBlob** ppCompiledShader)
{
	ID3DBlob* pErrorMsg = NULL;
	HRESULT hr = D3DCompileFromFile(pFileName, NULL, NULL, pEntrypoint, bPixel ? "ps_5_0" : "vs_5_0", 0, 0, ppCompiledShader, &pErrorMsg);

	if (FAILED(hr))
	{
		if (pErrorMsg != NULL)
		{
			OutputDebugStringA((char*)pErrorMsg->GetBufferPointer());
			pErrorMsg->Release();
		}
		return false;
	}
	if (pErrorMsg)
		pErrorMsg->Release();
	return true;
}

bool InitScene()
{
	//Compile Shaders from shader file
	CompileShader(L"..\\3DTP\\Shaders\\HelloShader.hlsl", false, "vxmain", &g_pVertexBlob);
	CompileShader(L"..\\3DTP\\Shaders\\HelloShader.hlsl", true, "pxmain", &g_pPixelBlob);

	//Create the Shader Objects
	g_pDevice->CreateVertexShader(g_pVertexBlob->GetBufferPointer(), g_pVertexBlob->GetBufferSize(), NULL, &g_pVertexShader);
	g_pDevice->CreatePixelShader(g_pPixelBlob->GetBufferPointer(), g_pPixelBlob->GetBufferSize(), NULL, &g_pPixelShader);

	//Set Vertex and Pixel Shaders
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);

	//Create the vertex buffer

	// Triangle
	/*
	Vertex vertices[] =
	{
		Vertex(-0.5f, -0.5f, 0.5f),
		Vertex(-0.5f, 0.5f, 0.5f),
		Vertex(0.5f, 0.5f, 0.5f),
		Vertex(0.5f, -0.5f, 0.5f),
	};

	unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
	*/

	// Cube
	Vertex vertices[] =
	{
		Vertex(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(+0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(+0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+0.5f, -0.5f, +0.5f, 1.0f, 0.0f, 0.0f, 1.0f),
	};

	DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	// Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexBufferData.pSysMem = (void*)vertices;

	HRESULT hr = g_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &g_pVertexBuffer);

	//Set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// Create the index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	// Create the buffer with the device.
	g_pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &g_pIndexBuffer);

	// Set the buffer.
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Create constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(cbPerObject);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;

	hr = g_pDevice->CreateBuffer(&constantBufferDesc, NULL, &g_pCbPerObjectBuffer);

	// Init camera
	g_camPosition = Vector3(0.0f, 10.0f, -80.0f);
	g_camTarget = Vector3(0.0f, 0.0f, 0.0f);
	g_camUp = Vector3(0.0f, 1.0f, 0.0f);

	g_camView = Matrix::CreateLookAt(g_camPosition, g_camTarget, g_camUp);
	g_camProjection = Matrix::CreatePerspectiveFieldOfView(M_PI_4, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 1.0f, 1000.0f);

	//Create the Input Layout
	hr = g_pDevice->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), g_pVertexBlob->GetBufferPointer(),
		g_pVertexBlob->GetBufferSize(), &g_pInputLayout);

	//Set the Input Layout
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	//Set Primitive Topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	//Set the Viewport
	g_pImmediateContext->RSSetViewports(1, &viewport);

	return true;
}

void UpdateScene()
{
	//Keep the cubes rotating
	rot += .0005f;
	if (rot > 2.0f * M_PI)
		rot = 0.0f;

	//Reset cube1World
	cube1World = Matrix();

	//Define cube1's world space matrix
	g_rotation = Matrix::CreateRotationY(rot);
	g_translation = Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 4.0f));

	//Set cube1's world space using the transformations
	// Matrix SRT
	cube1World = g_rotation * g_translation;

	//Reset cube2World
	cube2World = Matrix();

	//Define cube2's world space matrix
	g_rotation = Matrix::CreateRotationY(-rot);
	g_scale = Matrix::CreateScale(Vector3(1.0f, 1.0f, 1.0f));

	//Set cube2's world space matrix
	cube2World = g_scale * g_rotation;


	ulong iTime = timeGetTime();
	ulong iElapsedTime = iTime - g_iLastTime;
	g_iLastTime = iTime;
	float fElapsedTime = iElapsedTime * 0.001f;

	g_pInputManager->Manage();

	oFreeCamera.Update(g_pInputManager, fElapsedTime);

	g_camView = oFreeCamera.GetViewMatrix();
	g_camProjection = Matrix::CreatePerspectiveFieldOfView(M_PI / 4.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 1000.0f);

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
}

void DrawScene()
{
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, bgColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Object Drawing

	// Object 1
	g_WVP = cube1World * g_camView * g_camProjection;
	g_cbPerObject.wvp = g_WVP.Transpose();
	g_pImmediateContext->UpdateSubresource(g_pCbPerObjectBuffer, 0, NULL, &g_cbPerObject, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCbPerObjectBuffer);

	//Draw the first cube
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	g_WVP = cube2World * g_camView * g_camProjection;
	g_cbPerObject.wvp = g_WVP.Transpose();
	g_pImmediateContext->UpdateSubresource(g_pCbPerObjectBuffer, 0, NULL, &g_cbPerObject, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCbPerObjectBuffer);

	//Draw the second cube
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	// End of object drawing

	// ImGUI rendering
	ImGui_ImplDX11_NewFrame();

	bool windowLoaded;
	ImGui::Begin("Statistics", &windowLoaded);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();

	g_pSwapChain->Present(0, 0);
}