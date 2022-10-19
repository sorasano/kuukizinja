#pragma once
#include <wrl.h>

using namespace Microsoft::WRL;

class WinApp
{
public:
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


public:

	void Initialize();

	//終了
	void Finalize();

	//メッセージの処理
	bool processMessage();

	//getter
	HWND GetHwnd() const { return hwnd; }

	HINSTANCE GetHInstance() const { return w.hInstance; }
private:
	//ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASSEX w{};
public:
	// ウィンドウサイズ
	static const int window_width = 1280; // 横幅
	static const int window_height = 720; // 縦幅
};

