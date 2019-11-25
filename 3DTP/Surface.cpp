#include "stdafx.h"
#include "Surface.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Surface::Surface(HINSTANCE hInstance, int nCmdShow)
    : m_applicationInstance (hInstance)
    , m_width       (WINDOW_DEFAULT_WIDTH)
    , m_height      (WINDOW_DEFAULT_HEIGHT)
    , m_device      (nullptr)
    , m_context     (nullptr)
{
    createWindow(nCmdShow);
}

Surface::Surface(HINSTANCE hInstance, int nCmdShow, int width, int height)
    : m_applicationInstance (hInstance)
    , m_width       (width)
    , m_height      (height)
    , m_device      (nullptr)
    , m_context     (nullptr)
{
    createWindow(nCmdShow);
}

const HINSTANCE & Surface::GetApplicationInstance()
{
    return m_applicationInstance;
}

const HWND & Surface::GetWindow()
{
    return m_winWindow;
}

int Surface::GetWindowWidth()
{
    return m_width;
}

int Surface::GetWindowHeight()
{
    return m_height;
}

void Surface::CreateDevice()
{
    m_device = new Device(m_width, m_height, m_winWindow);
}

Device* Surface::GetDevice()
{
    return m_device;
}

Context* Surface::GetContext()
{
    return m_context;
}

void Surface::createWindow(int nCmdShow)
{
    WNDCLASSEX wcex;
    memset(&wcex, 0, sizeof(WNDCLASSEX));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_applicationInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"ApplicationClassName";

    if (RegisterClassEx(&wcex) == 0)
        throw std::runtime_error("Unable to register window class.");

    m_winWindow = CreateWindow(L"ApplicationClassName", L"Awesome application", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, m_applicationInstance, NULL);

    if (!m_winWindow)
        throw std::runtime_error("Unable to create window.");

    ShowWindow(m_winWindow, nCmdShow);
    UpdateWindow(m_winWindow);
}

//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplDX11_WndProcHandler(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        break;
    case WM_KEYUP:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            PostQuitMessage(0);
            break;
        }
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

Surface::~Surface()
{
    delete m_device;
    delete m_context;
}
