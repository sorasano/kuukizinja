#include <cassert>

#include <wrl.h>

using namespace Microsoft::WRL;

#pragma once
#define DIRECTINPUT_VERSION     0x0800   // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <Windows.h>
#include "WinApp.h"

class Input
{
public:

	//namespace省略
	template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:

	void Initialize(WinApp* winApp);

	void Update();

	//キーの押下をチェック
	bool PushKey(BYTE keyNumber);

	//キーのトリガーをチェック
	bool TriggerKey(BYTE keyNumber);

private:

	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInput8> directInput;

	BYTE key[256] = {};
	BYTE keyPre[256] = {};

	//WindowsAPI
	WinApp* winApp_ = nullptr;
};

