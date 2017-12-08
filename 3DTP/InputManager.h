#pragma once

namespace IAEngine
{
	class InputManager
	{
	public:

		InputManager();
		virtual ~InputManager();

		bool	Create(HINSTANCE iHinstance, HWND	iHwnd);
		void	Destroy();

		void Manage();

		long GetMouseMoveX() const { return m_iMouseMoveX; }
		long GetMouseMoveY() const { return m_iMouseMoveY; }
		bool IsLeftButtonClick() const { return m_bLeftMouseClick; }
		bool IsKeyDone(DWORD iKeyCode) const { return (m_pKeysStates[iKeyCode] & 0x80) != 0; }
		bool IsKeyPressed(DWORD iKeyCode) const { return m_pKeysPressed[iKeyCode]; }

	protected:
		bool CreateMouse(HWND iHwnd);
		bool CreateKeyBoard(HWND iHwnd);
		void ManageMouse();
		void ManageKeyBoard();

		IDirectInput*				m_pInput;

		//KeyBoard
		IDirectInputDevice*	m_pKeyboard;
		uchar								m_pKeysStates[256];
		bool								m_pKeysPressed[256];

		//Mouse
		IDirectInputDevice*	m_pMouse;
		bool								m_bLeftMouseClick;
		bool								m_bLeftMouseDown;
		bool								m_bRightMouseClick;
		bool								m_bRightMouseDown;
		long								m_iMouseMoveX;
		long								m_iMouseMoveY;
	};
}
