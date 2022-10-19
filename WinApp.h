#pragma once
#include <wrl.h>

using namespace Microsoft::WRL;

class WinApp
{
public:
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


public:

	void Initialize();

	//�I��
	void Finalize();

	//���b�Z�[�W�̏���
	bool processMessage();

	//getter
	HWND GetHwnd() const { return hwnd; }

	HINSTANCE GetHInstance() const { return w.hInstance; }
private:
	//�E�B���h�E�n���h��
	HWND hwnd = nullptr;

	// �E�B���h�E�N���X�̐ݒ�
	WNDCLASSEX w{};
public:
	// �E�B���h�E�T�C�Y
	static const int window_width = 1280; // ����
	static const int window_height = 720; // �c��
};
