#include "StdAfx.h"
#include "InputManager.h"

namespace IAEngine
{
	static uint s_iMouseDataSize = 16;
	static uint s_iKeyboardDataSize = 32;
	InputManager::InputManager()
	{
		m_bLeftMouseClick = false;
		m_bLeftMouseDown = false;
		m_bRightMouseClick = false;
		m_bRightMouseDown = false;
		memset( m_pKeysStates, 0, 256 * sizeof(uchar) );
		memset( m_pKeysPressed, false, 256 * sizeof(uchar) );
	}

	InputManager::~InputManager()
	{
	}

	bool	InputManager::Create(HINSTANCE iHinstance, HWND iHwnd)
	{
		if(FAILED(DirectInput8Create(iHinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pInput, NULL)))
			return false;
		if (CreateMouse(iHwnd) == false)
			return false;
		if (CreateKeyBoard(iHwnd) == false)
			return false;
		return true;
	}

	bool	InputManager::CreateMouse(HWND iHwnd)
	{
		if (FAILED(m_pInput->CreateDevice(GUID_SysMouse, &m_pMouse, NULL)))
			return false;
		if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse)))
			return false;
		if (FAILED(m_pMouse->SetCooperativeLevel(iHwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
			return false;
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize				= sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize	= sizeof(DIPROPHEADER);
		dipdw.diph.dwObj				= 0;
		dipdw.diph.dwHow				= DIPH_DEVICE;
		dipdw.dwData						= s_iMouseDataSize;
		if (FAILED(m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			return false;
		if (FAILED(m_pMouse->Acquire()))
			return false;
		return true;
	}

	bool	InputManager::CreateKeyBoard(HWND iHwnd)
	{
		if (FAILED(m_pInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard,NULL)))
			return false;
		if (FAILED(m_pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
			return false;
		if (FAILED(m_pKeyboard->SetCooperativeLevel(iHwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
			return false;
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = s_iKeyboardDataSize;
		if (FAILED(m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
			return false;
		if(FAILED(m_pKeyboard->Acquire()))
			return false;
		return true;
	}

	void	InputManager::Destroy()
	{
		if (m_pKeyboard)
		{
			m_pKeyboard->Unacquire();
			m_pKeyboard->Release();
			m_pKeyboard = NULL;
		}
		if (m_pMouse)
		{
			m_pMouse->Unacquire();
			m_pMouse->Release();
			m_pMouse = NULL;
		}
		if (m_pInput)
		{
			m_pInput->Release();
			m_pInput = NULL;
		}
	}

	void	InputManager::Manage()
	{
		ManageMouse();
		ManageKeyBoard();
	}

	void	InputManager::ManageMouse()
	{
		DWORD dwItems = s_iMouseDataSize;
		DIDEVICEOBJECTDATA mouseBuffer[16];//s_iMouseDataSize
		m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), mouseBuffer, &dwItems, 0);

		m_iMouseMoveX = 0;
		m_iMouseMoveY = 0;
		m_bLeftMouseClick = false;
		m_bRightMouseClick = false;

		for (DWORD i = 0; i < dwItems; ++i)
		{
			switch (mouseBuffer[i].dwOfs)
			{
				case DIMOFS_BUTTON0:
					if(mouseBuffer[i].dwData & 0x80) // le bouton gauche est enfoncé
						m_bLeftMouseDown = true;
					else // le bouton gauche est relaché
					{
						m_bLeftMouseClick = m_bLeftMouseDown;
						m_bLeftMouseDown = false;
					}
					break;
				case DIMOFS_BUTTON1:
					if(mouseBuffer[i].dwData & 0x80) // idem pour le bouton droit
						m_bRightMouseDown = true;
					else
					{
						m_bRightMouseClick = m_bRightMouseDown;
						m_bRightMouseDown = false;
					}
					break;
				case DIMOFS_X: // on a modifié la position horizontale de la souris
					m_iMouseMoveX += mouseBuffer[i].dwData;
					break;
				case DIMOFS_Y: // on a modifié la position verticale de la souris
					m_iMouseMoveY += mouseBuffer[i].dwData;
					break;
			}
		}
	}

	void	InputManager::ManageKeyBoard()
	{
		uchar	pKeysStates[256];
		m_pKeyboard->GetDeviceState(256 * sizeof(uchar), (LPVOID)pKeysStates);
		uint32* pNewKeyStates		= (uint32*)pKeysStates;
		uint32* pKeyStates			= (uint32*)m_pKeysStates;
		uint32* pKeyPressed			= (uint32*)m_pKeysPressed;
		for (uint i = 0; i < 64; ++i)
		{
			pKeyPressed[i] = pKeyStates[i] & ~pNewKeyStates[i];
			pKeyStates[i] = pNewKeyStates[i];
		}
	}

}
