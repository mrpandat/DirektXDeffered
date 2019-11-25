#pragma once

#include "stdafx.h"

#include "Surface.h"

struct Light
{
    Light()
    {
        ZeroMemory(this, sizeof(Light));
    }

    DirectX::SimpleMath::Vector4 dir;
    DirectX::SimpleMath::Vector4 ambient;
    DirectX::SimpleMath::Vector4 diffuse;
};

struct cbPerObject
{
	DirectX::SimpleMath::Matrix wvp;
    DirectX::SimpleMath::Matrix world;
    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Matrix proj;
};

struct cbPerFrame
{
    Light  light;
};