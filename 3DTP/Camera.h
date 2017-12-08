#pragma once

namespace IAEngine
{
	class InputManager;
	
	class FreeCamera
	{
	public:

		FreeCamera();
		virtual ~FreeCamera();

		void Update(InputManager* pInputManager, float fElaspedTime);

		const DirectX::SimpleMath::Matrix&		GetViewMatrix() const { return m_mView; }
		const DirectX::SimpleMath::Vector3&	GetPosition() const { return m_vPosition; }

	protected:
		float														m_fAngleZ;
		float														m_fAngleX;
		DirectX::SimpleMath::Vector3		m_vPosition;
		DirectX::SimpleMath::Vector3		m_vDirection;
		DirectX::SimpleMath::Vector3		m_vUp;
		DirectX::SimpleMath::Matrix			m_mView;
	};
}
