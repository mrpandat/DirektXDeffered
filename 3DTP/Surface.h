#pragma once

#include "stdafx.h"

#include "Device.h"
#include "Context.h"

#define WINDOW_DEFAULT_WIDTH   1280
#define WINDOW_DEFAULT_HEIGHT  720

class Surface
{
public:
    Surface(HINSTANCE hInstance, int nCmdShow);
    Surface(HINSTANCE hInstance, int nCmdShow, int width, int height);
    const HINSTANCE &   GetApplicationInstance();
    const HWND &        GetWindow();
    int                 GetWindowWidth();
    int                 GetWindowHeight();
    void                CreateDevice();
    Device*             GetDevice();
    Context*            GetContext();

    void Draw();

    ~Surface();

private:
    HINSTANCE   m_applicationInstance;
    HWND        m_winWindow;
    int         m_width;
    int         m_height;
    Device*     m_device;
    Context*    m_context;

    void        createWindow(int nCmdShow);
};

