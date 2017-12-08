#include "StdAfx.h"
#include "Camera.h"
#include "InputManager.h"

using namespace DirectX::SimpleMath;

namespace IAEngine
{

	FreeCamera::FreeCamera()
		: m_fAngleZ(0.0f)
		, m_fAngleX(0.0f)
	{
		m_vPosition = Vector3(0.0f, 0.0f, 0.0f);
		m_vDirection = Vector3(0.0f, 1.0f, 0.0f);
		m_vUp = Vector3(0.0f, 0.0f, 1.0f);
		m_mView = Matrix::CreateLookAt(m_vPosition, m_vPosition + m_vDirection, m_vUp);
	}

	FreeCamera::~FreeCamera()
	{
	}

	void FreeCamera::Update(InputManager* pInputManager, float fElaspedTime)
	{
		//freecam update
		float fMoveSensibility = 30.0f * fElaspedTime;
		float fMouseSensibility = 0.9f * fElaspedTime;
		//update angle
		m_fAngleZ += pInputManager->GetMouseMoveX() * fMouseSensibility;
		m_fAngleX += -pInputManager->GetMouseMoveY() * fMouseSensibility;

		m_vDirection.x = sin(m_fAngleZ) * cos(m_fAngleX);
		m_vDirection.y = cos(m_fAngleZ) * cos(m_fAngleX);
		m_vDirection.z = sin(m_fAngleX);
		m_vDirection.Normalize();
		//update 
		Vector3 vRight;
		m_vDirection.Cross(m_vUp, vRight);

		if (pInputManager->IsKeyDone(DIK_W))
			m_vPosition += m_vDirection * fMoveSensibility;
		if (pInputManager->IsKeyDone(DIK_S))
			m_vPosition -= m_vDirection * fMoveSensibility;
		if (pInputManager->IsKeyDone(DIK_A))
			m_vPosition -= vRight * fMoveSensibility;
		if (pInputManager->IsKeyDone(DIK_D))
			m_vPosition += vRight * fMoveSensibility;

		m_mView = Matrix::CreateLookAt(m_vPosition, m_vPosition + m_vDirection, m_vUp);
	}
}
