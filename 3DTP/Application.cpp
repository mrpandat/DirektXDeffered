#include "stdafx.h"
#include "Application.h"

#include "InputManager.h"
#include "Camera.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"

// Global Variables:
Surface*                    g_pSurface;
IAEngine::InputManager*		g_pInputManager = NULL;

ID3D11Buffer* g_pCbPerObjectBuffer;
ID3D11Buffer* g_pCbPerFrameBuffer;

cbPerObject g_cbPerObject;
cbPerFrame g_cbPerFrame;

Light light;

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

IAEngine::FreeCamera oFreeCamera;
ulong g_iLastTime;

// Forward declarations
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

    try
    {
        g_pSurface = new Surface(hInstance, nCmdShow);

        g_pInputManager = new IAEngine::InputManager();
        if (!g_pInputManager->Create(g_pSurface->GetApplicationInstance(), g_pSurface->GetWindow()))
        {
            delete g_pInputManager;
            throw std::runtime_error("Erreur lors de la création de l'input manager");
        }

        g_pSurface->CreateDevice();
        g_pSurface->GetDevice()->CreateDefaultRT();
        g_pSurface->GetDevice()->InitRasterizerState();
    }
    catch (const std::runtime_error& e)
    {
        LPWSTR message;
        MultiByteToWideChar(0, 0, e.what(), strlen(e.what()), message, strlen(e.what()));
        MessageBox(NULL, message, L"Initialization error", 0);
        return false;
    }

	//Create and fill other DirectX Stuffs like Vertex/Index buffer, shaders
	InitScene();

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
    delete g_pSurface;

	g_pCbPerObjectBuffer->Release();
    g_pCbPerFrameBuffer->Release();

	delete g_pInputManager;
	return (int) oMsg.wParam;
}

bool InitScene()
{
	// Add shaders from shader file
    g_pSurface->GetDevice()->SetShader(new Shader(ShaderType::Vertex, L"..\\3DTP\\HelloShader.hlsl"));
    g_pSurface->GetDevice()->SetShader(new Shader(ShaderType::Pixel, L"..\\3DTP\\HelloShader.hlsl"));

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
		Vertex(-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f),
		Vertex(-0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, -0.5f, +0.5f, -0.5f),
		Vertex(+0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, + 0.5f, +0.5f, -0.5f),
		Vertex(+0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, +0.5f, -0.5f, -0.5f),
		Vertex(-0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 1.0f, 1.0f, -0.5f, -0.5f, +0.5f),
		Vertex(-0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f, 1.0f, -0.5f, +0.5f, +0.5f),
		Vertex(+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f, 1.0f, +0.5f, +0.5f, +0.5f),
		Vertex(+0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 1.0f, 1.0f, +0.5f, -0.5f, +0.5f),
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

    g_pSurface->GetDevice()->AddBuffer(BufferType::Vertex, sizeof (Vertex), vertices);

    g_pSurface->GetDevice()->SetBuffer(BufferType::Index, sizeof (indices), indices);

    // Intialize constant buffers (Per model and per frame)
    g_pSurface->GetDevice()->AddBuffer(BufferType::Constant, sizeof (cbPerObject), NULL);
    g_pSurface->GetDevice()->AddBuffer(BufferType::Constant, sizeof (cbPerFrame), NULL);

	// Init camera
	g_camPosition = Vector3(0.0f, 10.0f, -80.0f);
	g_camTarget = Vector3(0.0f, 0.0f, 0.0f);
	g_camUp = Vector3(0.0f, 1.0f, 0.0f);

	g_camView = Matrix::CreateLookAt(g_camPosition, g_camTarget, g_camUp);
	g_camProjection = Matrix::CreatePerspectiveFieldOfView(M_PI_4, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 1.0f, 1000.0f);

    // Init light
    light.dir = DirectX::SimpleMath::Vector4(0.0f, -5.0f, -10.0f, 0.0f);
    light.ambient = DirectX::SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f);
    light.diffuse = DirectX::SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	//Create the Input Layout
    g_pSurface->GetDevice()->InitInputLayout(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
    g_pSurface->GetDevice()->AddViewport(g_pSurface->GetWindowWidth(), g_pSurface->GetWindowHeight(), 0, 0);
    g_pSurface->GetDevice()->InitViewports();

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
	//g_rotation = Matrix::CreateRotationY(rot);
	g_translation = Matrix::CreateTranslation(Vector3(0.0f, 0.0f, 4.0f));

	//Set cube1's world space using the transformations
	// Matrix SRT
	cube1World = g_rotation * g_translation;

	//Reset cube2World
	cube2World = Matrix();

	//Define cube2's world space matrix
	//g_rotation = Matrix::CreateRotationY(-rot);
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

    g_pSurface->GetDevice()->UpdateRenderTargets();
}

void DrawScene()
{
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
    g_pSurface->GetDevice()->ClearViews(bgColor);

	// Object Drawing

	// Object 1
	g_WVP = cube1World * g_camView * g_camProjection;
    g_cbPerObject.wvp = g_WVP.Transpose();
    g_cbPerObject.world = cube1World.Transpose();
    g_cbPerObject.view = g_camView.Transpose();
    g_cbPerObject.proj = g_camProjection.Transpose();

    g_pSurface->GetDevice()->UpdateConstantBuffer(0, &g_cbPerObject);

	// Draw the first cube
	g_pSurface->GetDevice()->GetContext()->DrawIndexed(36, 0, 0);

    // Object 2
	g_WVP = cube2World * g_camView * g_camProjection;
    g_cbPerObject.wvp = g_WVP.Transpose();
    g_cbPerObject.world = cube2World.Transpose();
    g_cbPerObject.view = g_camView.Transpose();
    g_cbPerObject.proj = g_camProjection.Transpose();
    g_pSurface->GetDevice()->UpdateConstantBuffer(0, &g_cbPerObject);

	// Draw the second cube
    g_pSurface->GetDevice()->GetContext()->DrawIndexed(36, 0, 0);

	// End of object drawing

    // Set lighting
    g_cbPerFrame.light = light;
    g_pSurface->GetDevice()->UpdateConstantBuffer(1, &g_cbPerFrame);

	// ImGUI rendering
	ImGui_ImplDX11_NewFrame();

	bool windowLoaded;
	ImGui::Begin("Statistics", &windowLoaded);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();

	g_pSurface->GetDevice()->Present();
}