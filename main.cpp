#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>
#include <string>

#include <DirectXMath.h>
using namespace DirectX;

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <DirectXTex.h>

#include "Input.h"

#include <wrl.h>

using namespace Microsoft::WRL;

#include <iostream>
#include <memory>
using namespace std;

#include "WinApp.h"

#include "math/Vector3.h"
#include "math/Matrix4.h"

#include <list>

#include <random>

#include <D3dx12.h>

const double PI = 3.141592653589;

// 定数バッファ用データ構造体（マテリアル）,（3D変換行列）
struct ConstBufferData {
	XMFLOAT4 color; // 色 (RGBA)
	XMMATRIX mat; //座標
};

// 定数バッファ用データ構造体（3D変換行列）
struct ConstBufferDataTransform {
	XMMATRIX mat; // 座標
};

//パイプラインセット
struct PipelineSet {

	ComPtr<ID3D12PipelineState> pipelinestate;

	ComPtr<ID3D12RootSignature> rootsignature;

};

PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* dev);

PipelineSet SpriteCreateGraphicsPipeline(ID3D12Device* dev);

//-----------スプライト----------

//スプライト用
struct VertexPosUv {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

//スプライト1枚分のデータ
struct Sprite {
	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	//Z軸回りの回転角
	float rotation = 0.0f;
	//座標
	XMFLOAT3 position = { 0,0,0 };
	//ワールド行列
	XMMATRIX matWorld;

	UINT texNumber = 0;

	XMFLOAT2 size = { 100,100 };
};

//テクスチャの最大枚数
const int spriteSRVCount = 512;

//スプライトの共通データ
struct SpriteCommon {
	//パイプラインセット
	PipelineSet pipelineSet;

	//射影行列
	XMMATRIX matProjrction{};

	//テクスチャ用デスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> descHeap;
	//テクスチャソース(テクスチャバッファ)の配列
	ComPtr<ID3D12Resource> texBuff[spriteSRVCount];

};

//スプライト生成
Sprite SpriteCreate(ID3D12Device* dev, int window_width, int window_height);

//スプライト共通データ生成
SpriteCommon SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height);

//スプライト共通グラフィックスコマンドのセット
void SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12DescriptorHeap* descHeap);

//スプライト単体描画

void SpriteDraw(const Sprite& sprite, ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12Device* dev);

//スプライト単体更新
void SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon);

//スプライト共通テクスチャ読み込み
void SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev);

//スプライト単体頂点バッファの転送
void SpriteTransferVertexBuffer(const Sprite& sprite);

//3Dオブジェクト型
struct Object3d {

	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};
	//親オブジェクトへのポインタ
	Object3d* parent = nullptr;
};

void InitializeObject3d(Object3d* object, ComPtr<ID3D12Device> device);

void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

void DrawObject3d(Object3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize);

//---------------自機-----------------

struct PlayerObject3d {
	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};

	//扇風機の状態 0停止 1右 2左 3発射後1フレーム 4その後
	int mode = 1;

	//キャラクターの移動速さ
	const float speed = 0.2f;

	//移動限界距離
	int moveLimitLeft = -30;
	int moveLimitRight = 30;

	//風量
	float windPower = 0;
	//風量の上昇量
	float powerSpeed = 0.1;
	//風量の限界値
	float maxPower = 10;

	int push = 0;

	int keyCoolTime = 100;

	////弾
	//std::list<std::unique_ptr<PlayerBulletObject3d>> bullets_;
};
//-------------------自機弾------------------

struct PlayerBulletObject3d {
	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};
	//速度
	Vector3 velocity_;

	//寿命
	static const int32_t kLifeTime = 60 * 5;
	//デスタイマー
	int32_t deathTimer_ = kLifeTime;
	//デスフラグ
	bool isDead_ = false;

};
//------------紙----------------

struct PaperObject3d {

	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};

	int flag = 0;

	//あったったか
	int isCol[10] = {};

	//座標
	Vector3 trans[10] = {};
	//角度
	Vector3 rot[10] = {};

	//飛行機型か丸型か
	int type[10] = { 0,0,1,0,0,1,0,0,1,0 };//0飛行機 1丸

	//落下したか
	int isLanding_[10] = {};

	//変わらない値(統一)
	float transY = 0;
	float transZ = 5;

	float rotX = 0;
	float rotZ = 0;

	//配置する位置の端
	float maxLeft = -30.0f;
	float maxRight = 20.0f;

	//配置する最大間隔
	float space = 4.0f;

	//回転差
	float rotdiff = 6;

	//前の座標
	float beforeTrans = 0;

	//サイズ
	int size = 4.5f;

};

//-------------紙飛行機-------------

struct PaperAirplaneObject3d {

	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};

	//0 停止 1 移動
	int move = 0;

	//統合スピード
	Vector3 velocity_ = { 0,0,0 };

	float endY = -20;
	bool isLanding = 0;

	//通常スピード
	float speed = 0.1;

	//落下スピード
	float fallSpeed = 0.1;

	//減速率
	float decelerationRate = 0.001;
};

//---------------丸紙-----------------


struct PaperCircleObject3d {

	//定数バッファ行列用
	ComPtr<ID3D12Resource> constBuffTransform;
	//定数バッファマップ行列用
	ConstBufferDataTransform* constMapTransform = nullptr;
	//アフィン変換情報
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 position = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld = {};

	//0 停止 1 移動
	int move = 0;

	//統合スピード
	Vector3 velocity_ = { 0,0,0 };

	float endY = -20;
	bool isLanding = 0;

	//通常スピード
	float speed = 0.1;

	//落下スピード
	float fallSpeed = 0.1;

	//減速率
	float decelerationRate = 0.001;

};

//---------------自機-----------------

//初期化
void PlayerInitialize(PlayerObject3d* object, ComPtr<ID3D12Device> device);

//更新
void PlayerUpdate(Input* input_, PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device, PlayerBulletObject3d playerBulletObj);

//描画
void PlayerDraw(PlayerObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize);

//旋回
void PlayerRotate(Input* input_, PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

//攻撃
void PlayerAttack(Input* input_, PlayerObject3d* object, ComPtr<ID3D12Device> device, PlayerBulletObject3d playerBulletObj);

//移動
void PlayerMove(PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

//mode変更
void PlayerModeChange(PlayerObject3d* object);

//風量ゲージ(パワーを測る)
void PlayermeasureWindPower(PlayerObject3d* object);

//衝突したら呼び出されるコールバック関数
void PlayerOnCollision();

void PlayerReset(PlayerObject3d* object);

void PlayerSetMode(int i, PlayerObject3d* object);

////-------------------自機弾------------------


void PlayerBulletInitialize(PlayerBulletObject3d* object, ComPtr<ID3D12Device> device, Vector3& position, Vector3& velocity);

void PlayerBulletUpdate(PlayerBulletObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

void PlayerBulletDraw(PlayerBulletObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize);

bool PlayerBulletIsDead(PlayerBulletObject3d* object) { return  object->isDead_; }

//衝突したら呼び出されるコールバック関数
void PlayerBulletOnCollision(PlayerBulletObject3d* object);

//------------紙----------------

void PaperInitialize(PaperObject3d* object, ComPtr<ID3D12Device> device);

void PaperUpdate(PaperObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

void PaperDraw(PaperObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);

//配置
void PaperSet(PaperObject3d* object);

//座標配置
void PaperSetTrans(int i, PaperObject3d* object);

//角度配置
void PaperSetRot(int i, PaperObject3d* object);

void PaperSetIsCol(int i, PaperObject3d* object);

void PaperOnCollision(int i, PaperObject3d* object);

void PaperReset(PaperObject3d* object);

//------------紙飛行機-----------------

void PaperAirplaneInitialize(PaperAirplaneObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device);

void PaperAirplaneUpdate(PaperAirplaneObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

void PaperAirplaneDraw(PaperAirplaneObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize);

//衝突したら呼び出されるコールバック関数
void PaperAirplaneOnCollision(float windPower, Vector3 fanTrans, PaperAirplaneObject3d* object);

//速度計さん
void PaperAirplaneCalculationSpeed(PaperAirplaneObject3d* object);

void PaperAirplaneMove(PaperAirplaneObject3d* object);

//速度などをセット
void PaperAirplaneSet(float windPower, Vector3 fanTrans, PaperAirplaneObject3d* object);

//着地してるかしてないか
void PaperAirplaneLandingJudge(PaperAirplaneObject3d* object);

void PaperAirplaneReset(PaperAirplaneObject3d* object);
//------------丸紙-----------------

void PaperCircleInitialize(PaperCircleObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device);

void PaperCircleUpdate(PaperCircleObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

void PaperCircleDraw(PaperCircleObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize);

//衝突したら呼び出されるコールバック関数
void PaperCircleOnCollision(float windPower, Vector3 fanTrans, PaperCircleObject3d* object);

//速度計さん
void PaperCircleCalculationSpeed(PaperCircleObject3d* object);

void PaperCircleMove(PaperCircleObject3d* object);

//速度などをセット
void PaperCircleSet(float windPower, Vector3 fanTrans, PaperCircleObject3d* object);

//着地してるかしてないか
void PaperCircleLandingJudge(PaperCircleObject3d* object);

void PaperCircleReset(PaperCircleObject3d* object);

// ウィンドウプロシージャ
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


//windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//ポインタ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//WindowsAPIの終了処理
	winApp->Finalize();

	//WindowsAPI解放
	//delete winApp;
	//winApp = nullptr;

	// DirectX初期化処理 ここから

#ifdef _DEBUG
//デバッグレイヤーをオンに
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ComPtr<ID3D12Device> device;
	ComPtr<IDXGIFactory7> dxgiFactory;
	ComPtr<IDXGISwapChain4> swapChain;
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;

	// DXGIファクトリーの生成
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));
	// アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	// ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter4> tmpAdapter;
	// パフォーマンスが高いものから順に、全てのアダプターを列挙する
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
		i++) {
		// 動的配列に追加する
		adapters.push_back(tmpAdapter);
	}

	// 妥当なアダプタを選別する
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		// アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);
		// ソフトウェアデバイスを回避
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// デバイスを採用してループを抜ける
			tmpAdapter = adapters[i];
			break;
		}
	}

	// 対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for (size_t i = 0; i < _countof(levels); i++) {
		// 採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i],
			IID_PPV_ARGS(&device));
		if (result == S_OK) {
			// デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	// コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	assert(SUCCEEDED(result));
	// コマンドリストを生成
	result = device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	// スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色情報の書式
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // バックバッファ用
	swapChainDesc.BufferCount = 2; // バッファ数を2つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//IDXGISwapChain1のComPtrを用意
	ComPtr<IDXGISwapChain1> swapchain1;

	// スワップチェーンの生成
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		&swapchain1);
	//assert(SUCCEEDED(result));

	//生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapchain1.As(&swapChain);

	// デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; // 裏表の2つ
	// デスクリプタヒープの生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	// バックバッファ
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	// スワップチェーンの全てのバッファについて処理する
	for (size_t i = 0; i < backBuffers.size(); i++) {
		// スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		// デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		// 裏か表かでアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		// レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		// シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		// レンダーターゲットビューの生成
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	// フェンスの生成
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	////キーボード処理

	//ポインタ
	Input* input = nullptr;

	//入力の初期化
	input = new Input();
	input->Initialize(winApp);

	////入力開放
	//delete input;

	// DirectX初期化処理 ここまで

	//描画初期化処理　ここから

	//深度バッファ

	//リソース設定
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Width = WinApp::window_width;
	depthResourceDesc.Height = WinApp::window_height;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//深度用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//深度地のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//リソース設定
	ComPtr<ID3D12Resource> depthBuff;
	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);

	//深度ビュー用デスクリプタ―ヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr<ID3D12DescriptorHeap> dsvHeap;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//-----------スプライト----------


	//スプライト共通データ生成
	SpriteCommon spriteCommon;
	spriteCommon = SpriteCommonCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	//スプライトテクスチャ読み込み
	//タイトル
	SpriteCommonLoadTexture(spriteCommon, 0, L"Resources/UI/rule.png", device.Get());
	//クリア画面
	SpriteCommonLoadTexture(spriteCommon, 1, L"Resources/clear.png", device.Get());

	//---UI---

	//パワーゲージ内,外
	SpriteCommonLoadTexture(spriteCommon, 2, L"Resources/UI/powerGaugeIn.png", device.Get());
	//クリア画面
	SpriteCommonLoadTexture(spriteCommon, 3, L"Resources/UI/powerGaugeFlame.png", device.Get());

	//くじ
	//大吉
	SpriteCommonLoadTexture(spriteCommon, 4, L"Resources/UI/kuzi/daikiti.png", device.Get());
	//吉
	SpriteCommonLoadTexture(spriteCommon, 5, L"Resources/UI/kuzi/kiti.png", device.Get());
	//中吉
	SpriteCommonLoadTexture(spriteCommon, 6, L"Resources/UI/kuzi/tyuukiti.png", device.Get());
	//小吉
	SpriteCommonLoadTexture(spriteCommon, 7, L"Resources/UI/kuzi/syoukiti.png", device.Get());
	//凶
	SpriteCommonLoadTexture(spriteCommon, 8, L"Resources/UI/kuzi/kyou.png", device.Get());
	//大凶
	SpriteCommonLoadTexture(spriteCommon, 9, L"Resources/UI/kuzi/daikyou.png", device.Get());

	//数字
	//0～9
	SpriteCommonLoadTexture(spriteCommon, 10, L"Resources/UI/number/0.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 11, L"Resources/UI/number/1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 12, L"Resources/UI/number/2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 13, L"Resources/UI/number/3.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 14, L"Resources/UI/number/4.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 15, L"Resources/UI/number/5.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 16, L"Resources/UI/number/6.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 17, L"Resources/UI/number/7.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 18, L"Resources/UI/number/8.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 19, L"Resources/UI/number/9.png", device.Get());

	//くじ内容

	//大吉
	SpriteCommonLoadTexture(spriteCommon, 20, L"Resources/UI/kuzi/comment/daikiti1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 21, L"Resources/UI/kuzi/comment/daikiti2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 22, L"Resources/UI/kuzi/comment/daikiti3.png", device.Get());

	//吉
	SpriteCommonLoadTexture(spriteCommon, 23, L"Resources/UI/kuzi/comment/kiti1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 24, L"Resources/UI/kuzi/comment/kiti2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 25, L"Resources/UI/kuzi/comment/kiti3.png", device.Get());

	//中吉
	SpriteCommonLoadTexture(spriteCommon, 26, L"Resources/UI/kuzi/comment/tyuukiti1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 27, L"Resources/UI/kuzi/comment/tyuukiti2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 28, L"Resources/UI/kuzi/comment/tyuukiti3.png", device.Get());

	//小吉
	SpriteCommonLoadTexture(spriteCommon, 29, L"Resources/UI/kuzi/comment/syoukiti1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 30, L"Resources/UI/kuzi/comment/syoukiti2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 31, L"Resources/UI/kuzi/comment/syoukiti3.png", device.Get());

	//凶
	SpriteCommonLoadTexture(spriteCommon, 32, L"Resources/UI/kuzi/comment/kyou1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 33, L"Resources/UI/kuzi/comment/kyou2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 34, L"Resources/UI/kuzi/comment/kyou3.png", device.Get());

	//大凶
	SpriteCommonLoadTexture(spriteCommon, 35, L"Resources/UI/kuzi/comment/daikyou1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 36, L"Resources/UI/kuzi/comment/daikyou2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 37, L"Resources/UI/kuzi/comment/daikyou3.png", device.Get());

	//タイトル
	SpriteCommonLoadTexture(spriteCommon, 38, L"Resources/title/1.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 39, L"Resources/title/2.png", device.Get());
	SpriteCommonLoadTexture(spriteCommon, 40, L"Resources/title/3.png", device.Get());

	Sprite sprite;

	sprite.texNumber = 0;

	//スプライトの生成
	sprite = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	sprite.size.x = WinApp::window_width;
	sprite.size.y = WinApp::window_height;
	//反映
	SpriteTransferVertexBuffer(sprite);

	//タイトル

	Sprite titleSprite[3];

	for (int i = 0; i < 3; i++) {
		//スプライトの生成
		titleSprite[i] = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

		titleSprite[i].texNumber = 38 + i;

		titleSprite[i].size.x = WinApp::window_width;
		titleSprite[i].size.y = WinApp::window_height;
		//反映
		SpriteTransferVertexBuffer(titleSprite[i]);
	}

	//クリア

	Sprite clearSprite;

	//スプライトの生成
	clearSprite = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	clearSprite.texNumber = 1;

	clearSprite.size.x = WinApp::window_width / 2;
	clearSprite.size.y = WinApp::window_height;
	//反映
	SpriteTransferVertexBuffer(clearSprite);

	//---UI---

	//操作説明

	Sprite ruleSprite;

	//スプライトの生成
	ruleSprite = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	ruleSprite.texNumber = 0;

	ruleSprite.size.x = 800;
	ruleSprite.size.y = 64;
	//反映
	SpriteTransferVertexBuffer(ruleSprite);


	//パワーゲージ中

	Sprite powerGaugeInSprite;

	//スプライトの生成
	powerGaugeInSprite = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	powerGaugeInSprite.texNumber = 2;

	powerGaugeInSprite.size.x = 16;
	powerGaugeInSprite.size.y = 16;
	//反映
	SpriteTransferVertexBuffer(powerGaugeInSprite);

	//パワーゲージ外

	Sprite powerGaugeFlameSprite;

	//スプライトの生成
	powerGaugeFlameSprite = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

	powerGaugeFlameSprite.texNumber = 3;

	powerGaugeFlameSprite.size.x = 600;
	powerGaugeFlameSprite.size.y = 30;
	powerGaugeFlameSprite.position.x = 300;
	powerGaugeFlameSprite.position.y = 100;

	//反映
	SpriteTransferVertexBuffer(powerGaugeFlameSprite);

	//スコア

	// 0 shot1 1 shot2 2 shot3 3ハイスコア
	Sprite scoreSprite[4][4];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			//スプライトの生成
			scoreSprite[j][i] = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

			scoreSprite[j][i].texNumber = 10;

			scoreSprite[j][i].size.x = 64;
			scoreSprite[j][i].size.y = 64;
			scoreSprite[j][i].position.x = 300 + (i * 64);
			scoreSprite[j][i].position.y = 100;

			//反映
			SpriteTransferVertexBuffer(scoreSprite[j][i]);
		}
	}

	//くじ内容
	//0 大吉 1 吉 2 中吉 3小吉 4凶 5大凶
	Sprite kuziCommentSprite[18];

	for (int i = 0; i < 18; i++) {
		//スプライトの生成
		kuziCommentSprite[i] = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

		kuziCommentSprite[i].texNumber = i + 20;

		kuziCommentSprite[i].size.x = 540;
		kuziCommentSprite[i].size.y = 120;
		kuziCommentSprite[i].position.x = 370;
		kuziCommentSprite[i].position.y = 200;

		//反映
		SpriteTransferVertexBuffer(kuziCommentSprite[i]);

	}

	//くじ
//0 大吉 1 吉 2 中吉 3小吉 4凶 5大凶
	Sprite kuziSprite[6];

	for (int i = 0; i < 6; i++) {
		//スプライトの生成
		kuziSprite[i] = SpriteCreate(device.Get(), WinApp::window_width, WinApp::window_height);

		kuziSprite[i].texNumber = 4 + i;

		kuziSprite[i].size.x = 128;
		kuziSprite[i].size.y = 64;
		kuziSprite[i].position.x = 580;
		kuziSprite[i].position.y = 80;

		//反映
		SpriteTransferVertexBuffer(kuziSprite[i]);

	}
	//----------3dobjecct---------

	// 頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT3 normal; //法線ベクトル
		XMFLOAT2 uv;  // uv座標
	};

	// 頂点データ
	Vertex vertices[] = {
		// x      y     z     法線  u     v
		//前
		{{-1.0f,-1.0f, -1.0f}, {}, {0.0f, 1.0f}}, // 左下
		{{-1.0f, 1.0f, -1.0f},{}, {0.0f, 0.0f}}, // 左上
		{{1.0f, -1.0f, -1.0f}, {},{1.0f, 1.0f}}, // 右下
		{{1.0f, 1.0f, -1.0f},{}, {1.0f, 0.0f}}, // 右上

		//後
		{{-1.0f, 1.0f, 1.0f},{}, {0.0f, 0.0f}}, // 左上
		{{-1.0f,-1.0f, 1.0f}, {},{0.0f, 1.0f}}, // 左下
		{{1.0f, 1.0f,  1.0f}, {},{1.0f, 0.0f}}, // 右上
		{{1.0f, -1.0f, 1.0f}, {},{1.0f, 1.0f}}, // 右下

		//左
		{{-1.0f,-1.0f, -1.0f},{}, {0.0f, 1.0f}}, // 左下
		{{-1.0f, -1.0f, 1.0f}, {},{0.0f, 0.0f}}, // 左上
		{{-1.0f, 1.0f, -1.0f},{}, {1.0f, 1.0f}}, // 右下
		{{-1.0f, 1.0f, 1.0f}, {},{1.0f, 0.0f}}, // 右上

		//右
		{{1.0f, -1.0f, 1.0f}, {},{0.0f, 0.0f}}, // 左上
		{{1.0f,-1.0f, -1.0f}, {},{0.0f, 1.0f}}, // 左下
		{{1.0f, 1.0f, 1.0f}, {},{1.0f, 0.0f}}, // 右上
		{{1.0f, 1.0f, -1.0f},{}, {1.0f, 1.0f}}, // 右下

		//下
		{{-1.0f,1.0f, -1.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-1.0f, 1.0f, 1.0f}, {},{0.0f, 0.0f}}, // 左上
		{{1.0f, 1.0f, -1.0f},{}, {1.0f, 1.0f}}, // 右下
		{{1.0f, 1.0f, 1.0f}, {},{1.0f, 0.0f}}, // 右上

		//上
		{{-1.0f, -1.0f, -1.0f},{}, {0.0f, 0.0f}}, // 左上
		{{-1.0f, -1.0f, 1.0f},{}, {0.0f, 0.0f}}, // 左下
		{{1.0f, -1.0f,  -1.0f}, {},{1.0f, 0.0f}}, // 右上
		{{1.0f, -1.0f, 1.0f},{}, {1.0f, 0.0f}}, // 右下

	};


	// インデックスデータ
	unsigned short indices[] = {
		//前
		0, 1, 2, // 三角形1つ目
		2, 1, 3, // 三角形2つ目
		//後
		4,5,6,
		6,5,7,
		//左
		 8,9,10,
		 10,9,11,
		 //右
		 12,13,14,
		 14,13,15,
		 //下
		 16,17,18,
		 18,17,19,
		 //上
		 20,21,22,
		 22,21,23
	};

	// 頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	int sizeID = static_cast<int>(sizeof(indices[0]) * _countof(indices)) / 2;

	//法線の実装
	for (int i = 0; i < sizeID / 3; i++) {
		//3角形1つごとに計算してく
		//三角形のインデックスを取り出して一時的な変数に入れる
		unsigned short indices0 = indices[i * 3 + 0];
		unsigned short indices1 = indices[i * 3 + 1];
		unsigned short indices2 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices2].pos);
		//p0→p1ベクトル,p0→p2ベクトルを計算 (ベクトルの減算)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//正規化
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[indices0].normal, normal);
		XMStoreFloat3(&vertices[indices1].normal, normal);
		XMStoreFloat3(&vertices[indices2].normal, normal);

	}

	// 頂点バッファの設定
	D3D12_HEAP_PROPERTIES heapProp{}; // ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB; // 頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 頂点バッファの生成
	ComPtr<ID3D12Resource> vertBuff;
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// GPU上のバッファに対応した仮想メモリ(メインメモリ上)を取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	assert(SUCCEEDED(result));
	// 全頂点に対して
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i]; // 座標をコピー
	}
	// 繋がりを解除
	vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	// 頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	// 頂点1つ分のデータサイズ
	vbView.StrideInBytes = sizeof(vertices[0]);

	// インデックスデータ全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	// リソース設定
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; // インデックス情報が入る分のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// インデックスバッファの生成
	ComPtr<ID3D12Resource> indexBuff;
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	// インデックスバッファビューの作成
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	// インデックスバッファをマッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// 全インデックスに対して
	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];   // インデックスをコピー
	}
	// マッピング解除
	indexBuff->Unmap(0, nullptr);

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferData) + 0xff) & ~0xff;   // 256バイトアラインメント
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ComPtr<ID3D12Resource> constBuffMaterial;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial));
	assert(SUCCEEDED(result));

	// 定数バッファのマッピング
	ConstBufferData* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // マッピング
	assert(SUCCEEDED(result));

	////3Dオブジェクトの数
	//const size_t kObjectCount = 50;
	//3dオブジェクトの配列
	Object3d groundObj;
	InitializeObject3d(&groundObj, device);

	groundObj.scale = { 50.0f,1.0f, 1000.0f };
	groundObj.rotation = { 0.0f,0.0f,0.0f };
	groundObj.position = { 0.0f,-21.0f,0.0f };

	////配列内の全オブジェクトに対して
	//for (int i = 0; i < _countof(object3ds); i++) {
	//	//初期化
	//	InitializeObject3d(&object3ds[i], device);

	//	//先頭以外なら
	//	if (i > 0) {
	//		//ひとつ前のオブジェクトを親オブジェクトとする
	//		object3ds[i].parent = &object3ds[i - 1];

	//		object3ds[i].scale = { 0.9f, 0.9f, 0.9f };
	//		object3ds[i].rotation = { 0.0f,0.0f,30.0f };
	//		object3ds[i].position = { 0.0f,0.0f,-8.0f };
	//	}
	//}

	//------自機の初期化--------
	PlayerObject3d playerObj;
	PlayerInitialize(&playerObj, device);

	//--------自機弾初期化---------
	PlayerBulletObject3d playerBulletObj[3];

	//PlayerBulletInitialize(&playerBulletObj, device);

	//---------紙初期化---------

	//3Dオブジェクトの数
	const size_t paperObjCount = 10;
	//3dオブジェクトの配列
	PaperObject3d  paperObj;

	////配列内の全オブジェクトに対して
	//for (int i = 0; i < _countof(paperObjs); i++) {
	//	//初期化
	PaperInitialize(&paperObj, device);
	//}
	//---------紙飛行機初期化---------

	//3dオブジェクトの配列
	PaperAirplaneObject3d  paperAirplaneObjs[paperObjCount];

	//配列内の全オブジェクトに対して
	//for (int i = 0; i < _countof(paperAirplaneObjs); i++) {
	//	//初期化
	//	PaperAirplaneInitialize(paperAirplaneObjs, device);
	//}

	//---------丸紙初期化---------

	//3dオブジェクトの配列
	PaperCircleObject3d paperCircleObjs[paperObjCount];

	//配列内の全オブジェクトに対して
	//for (int i = 0; i < _countof(paperCircleObjs); i++) {
	//	//初期化
	//	PaperCircleInitialize(paperCircleObjs, device);
	//}

	//gamescene初期化
	//GameScene gamescene;

	//gamescene(ゲームシーン管理系,カメラ関係,当たり判定)


	//カメラモード
	int cameramode = 0;//0 上から //1横から

	//何番の紙飛行機が当たったか
	int touchPaperNum = 0;

	//0　タイトル　1射撃準備 2紙飛行機飛び 3リザルト
	int scene = 0;

	//何回打ったか
	int shot = 0;
	//地面についてるか
	int isLand = 0;

	//何番をもう打ったか
	int beginshot[3] = {};

	int highScore = 0;

	//スコア
	int score[3] = {};

	int kuzi = 0;

	int kuziComment = 0;

	//タイトル画像変更
	int titleNum = 0;

	int titleCoolTimer = 0;
	int titleCoolTime = 100;

	//座標変換

	//2D座標変換

	//単位行列を代入
	//constMapTransform->mat = XMMatrixIdentity();

	//constMapTransform->mat.r[0].m128_f32[0] = 2.0f / window_width;
	//constMapTransform->mat.r[1].m128_f32[1] = -2.0f / window_height;

	//constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
	//constMapTransform->mat.r[3].m128_f32[1] = 1.0f;

	//平行射影行列の計算(左端,右端,下端,上端,前端,奥端)
	//constMapTransform->mat = XMMatrixOrthographicOffCenterLH(0.0f, window_width, window_height,0.0f,0.0f,1.0f);

	//透視投影行列の計算
	XMMATRIX matProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), //上下画角45度
		(float)winApp->window_width / winApp->window_height,//アスペクト比(画面縦幅/画面縦幅)
		0.1f, 1000.0f);//前端,奥端

	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye(0, 150, -10);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, -0);
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	float angle = 0.0f; // カメラの回転角


	// 値を書き込むと自動的に転送される
	constMapMaterial->color = XMFLOAT4(1, 1, 1, 0.5f);              // RGBAで半透明の赤

	//画像ファイルの用意

	TexMetadata metadata{};
	ScratchImage scratchImg{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/paperAirplane.png",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata, scratchImg);

	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	//テクスチャマッピング↓

	// ヒープ設定
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	// リソース設定
	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;

	// テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	//// 元データ解放
	//delete metadata;

	//2枚目

	TexMetadata metadata2{};
	ScratchImage scratchImg2{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/paperCircle.png",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata2, scratchImg2);

	ScratchImage mipChain2{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg2.GetImages(), scratchImg2.GetImageCount(), scratchImg2.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain2);
	if (SUCCEEDED(result)) {
		scratchImg2 = std::move(mipChain2);
		metadata2 = scratchImg2.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata2.format = MakeSRGB(metadata2.format);

	//テクスチャマッピング↓

	// リソース設定
	D3D12_RESOURCE_DESC textureResourceDesc2{};
	textureResourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc2.Format = metadata2.format;
	textureResourceDesc2.Width = metadata2.width;
	textureResourceDesc2.Height = (UINT)metadata2.height;
	textureResourceDesc2.DepthOrArraySize = (UINT16)metadata2.arraySize;
	textureResourceDesc2.MipLevels = (UINT16)metadata2.mipLevels;
	textureResourceDesc2.SampleDesc.Count = 1;

	// テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff2;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff2));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata2.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg2.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff2->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	//3枚目

	TexMetadata metadata3{};
	ScratchImage scratchImg3{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/player.png",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata3, scratchImg3);

	ScratchImage mipChain3{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg3.GetImages(), scratchImg3.GetImageCount(), scratchImg3.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain3);
	if (SUCCEEDED(result)) {
		scratchImg3 = std::move(mipChain3);
		metadata3 = scratchImg3.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata3.format = MakeSRGB(metadata3.format);

	//テクスチャマッピング↓

	// リソース設定
	D3D12_RESOURCE_DESC textureResourceDesc3{};
	textureResourceDesc3.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc3.Format = metadata3.format;
	textureResourceDesc3.Width = metadata3.width;
	textureResourceDesc3.Height = (UINT)metadata3.height;
	textureResourceDesc3.DepthOrArraySize = (UINT16)metadata3.arraySize;
	textureResourceDesc3.MipLevels = (UINT16)metadata3.mipLevels;
	textureResourceDesc3.SampleDesc.Count = 1;

	// テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff3;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc3,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff3));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata3.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg3.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff3->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	//4枚目

	TexMetadata metadata4{};
	ScratchImage scratchImg4{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/playerBullet.png",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata4, scratchImg4);

	ScratchImage mipChain4{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg4.GetImages(), scratchImg4.GetImageCount(), scratchImg4.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain4);
	if (SUCCEEDED(result)) {
		scratchImg4 = std::move(mipChain4);
		metadata4 = scratchImg4.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata4.format = MakeSRGB(metadata4.format);

	//テクスチャマッピング↓

	// リソース設定
	D3D12_RESOURCE_DESC textureResourceDesc4{};
	textureResourceDesc4.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc4.Format = metadata4.format;
	textureResourceDesc4.Width = metadata4.width;
	textureResourceDesc4.Height = (UINT)metadata4.height;
	textureResourceDesc4.DepthOrArraySize = (UINT16)metadata4.arraySize;
	textureResourceDesc4.MipLevels = (UINT16)metadata4.mipLevels;
	textureResourceDesc4.SampleDesc.Count = 1;

	// テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff4;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc4,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff4));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata4.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg4.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff4->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	//5枚目

	TexMetadata metadata5{};
	ScratchImage scratchImg5{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/ground.png",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata5, scratchImg5);

	ScratchImage mipChain5{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg5.GetImages(), scratchImg5.GetImageCount(), scratchImg5.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain5);
	if (SUCCEEDED(result)) {
		scratchImg5 = std::move(mipChain5);
		metadata5 = scratchImg5.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata5.format = MakeSRGB(metadata5.format);

	//テクスチャマッピング↓

	// リソース設定
	D3D12_RESOURCE_DESC textureResourceDesc5{};
	textureResourceDesc5.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc5.Format = metadata5.format;
	textureResourceDesc5.Width = metadata5.width;
	textureResourceDesc5.Height = (UINT)metadata5.height;
	textureResourceDesc5.DepthOrArraySize = (UINT16)metadata5.arraySize;
	textureResourceDesc5.MipLevels = (UINT16)metadata5.mipLevels;
	textureResourceDesc5.SampleDesc.Count = 1;

	// テクスチャバッファの生成
	ComPtr<ID3D12Resource> texBuff5;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc5,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff5));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata5.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg5.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff5->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}
	// SRVの最大個数
	const size_t kMaxSRVCount = 2056;

	// デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	// 設定を元にSRV用デスクリプタヒープを生成
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	//SRVヒープの先頭ハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff.Get(), &srvDesc, srvHandle);

	//2つ目

	//デスクリプタ 一つハンドルを進める
	UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += incrementSize;

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = textureResourceDesc2.Format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = textureResourceDesc2.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff2.Get(), &srvDesc2, srvHandle);

	//3つ目

	//デスクリプタ 一つハンドルを進める
	//UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += incrementSize;

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{};
	srvDesc3.Format = textureResourceDesc3.Format;
	srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc3.Texture2D.MipLevels = textureResourceDesc3.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff3.Get(), &srvDesc3, srvHandle);

	//4つ目

	//デスクリプタ 一つハンドルを進める
	//UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += incrementSize;

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc4{};
	srvDesc4.Format = textureResourceDesc4.Format;
	srvDesc4.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc4.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc4.Texture2D.MipLevels = textureResourceDesc4.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff4.Get(), &srvDesc4, srvHandle);

	//5つ目

	//デスクリプタ 一つハンドルを進める
	//UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += incrementSize;

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc5{};
	srvDesc5.Format = textureResourceDesc5.Format;
	srvDesc5.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc5.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc5.Texture2D.MipLevels = textureResourceDesc5.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff5.Get(), &srvDesc5, srvHandle);



	// CBV,SRV,UAVの1個分のサイズを取得
	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ハンドルを1つ進める（SRVの位置）
	srvHandle.ptr += descriptorSize * 1;

	// CBV(コンスタントバッファビュー)の設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	//cbvDescの値設定（省略）
	// 定数バッファビュー生成
	device->CreateConstantBufferView(&cbvDesc, srvHandle);

	//3dオブジェクト用パイプライン生成呼び出し
	PipelineSet object3dPipelineSet = Object3dCreateGraphicsPipeline(device.Get());

	//スプライト用パイプライン生成呼び出し
	PipelineSet spritePipelineSet = SpriteCreateGraphicsPipeline(device.Get());

	//描画初期化処理　ここまで

	// ゲームループ
	while (true) {

		//Windowsのメッセージ処理
		if (winApp->processMessage()) {
			//ゲームループを抜ける
			break;
		}

		// DirectX毎フレーム処理 ここから

		input->Update();

		//更新処理-ここから

		//射影変換

		if (input->PushKey(DIK_D) || input->PushKey(DIK_A)) {
			if (input->PushKey(DIK_D)) { angle += XMConvertToRadians(1.0f); }
			else if (input->PushKey(DIK_A)) { angle -= XMConvertToRadians(1.0f); }

			//angleラジアンだけY軸回りに回転.半径は-100
			eye.x = -100 * sinf(angle);
			eye.z = -100 * cosf(angle);
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		}

		//---------------カメラ-------------

		if (scene == 0) {


			//タイトル画像変換

			titleCoolTimer++;
			if (titleCoolTimer >= titleCoolTime) {
				titleCoolTimer = 0;
				if (titleNum != 2) {
					titleNum++;
				}
				else {
					titleNum = 0;
				}
			}

		}
		else if (scene == 1) {

			cameramode = 0;

			//自機
			PlayerUpdate(input, &playerObj, matView, matProjection, device, playerBulletObj[shot]);

			//弾

			if (playerObj.mode == 0) {

				//弾の速度
				const float kBulletSpeed = 1.0f;
				Vector3 velocity(0, 0, kBulletSpeed);

				//速度ベクトルを自機の向きに合わせて回転させる
				velocity = transform(velocity, rotateX(playerBulletObj[shot].rotation.x));

				Vector3 pos(playerObj.position.x, playerObj.position.y, playerObj.position.z);

				//弾を生成し初期化
				PlayerBulletInitialize(&playerBulletObj[shot], device, pos, velocity);

			}
			else if (playerObj.mode == 3) {

				if (playerBulletObj[shot].isDead_ == false) {
					PlayerBulletUpdate(&playerBulletObj[shot], matView, matProjection);
				}
				else {

				}
			}

			if (shot >= 1) {

			}

			//紙
			PaperUpdate(&paperObj, matView, matProjection);

			for (size_t i = 0; i < 10; i++) {
				//リセット
				if (paperObj.flag == 0) {
					if (paperObj.type[i] == 0) {
						PaperAirplaneReset(&paperAirplaneObjs[i]);
					}
					else {
						PaperCircleReset(&paperCircleObjs[i]);
					}
				}

				//初期化
				else if (paperObj.flag == 1) {
					if (paperObj.type[i] == 0) {
						PaperAirplaneInitialize(&paperAirplaneObjs[i], matView, matProjection, device);
						paperAirplaneObjs[i].position.z = paperObj.trans[i].z;
						paperAirplaneObjs[i].position.x = paperObj.trans[i].x;
						paperAirplaneObjs[i].position.y = paperObj.trans[i].y;

						paperAirplaneObjs[i].rotation.y = paperObj.rot[i].y;

					}
					else {
						PaperCircleInitialize(&paperCircleObjs[i], matView, matProjection, device);
						paperCircleObjs[i].position.z = paperObj.trans[i].z;
						paperCircleObjs[i].position.x = paperObj.trans[i].x;
						paperCircleObjs[i].position.y = paperObj.trans[i].y;

						paperCircleObjs[i].rotation.y = paperObj.rot[i].y;
					}

					if (paperObj.type[i] == 0) {
						paperCircleObjs[i].position.z = -1000;
						paperCircleObjs[i].position.x = -1000;
						paperCircleObjs[i].position.y = -1000;
					}
					else {
						paperAirplaneObjs[i].position.z = -1000;
						paperAirplaneObjs[i].position.x = -1000;
						paperAirplaneObjs[i].position.y = -1000;
					}

				}

				//Update

				else {
					if (paperObj.type[i] == 0) {
						PaperAirplaneUpdate(&paperAirplaneObjs[i], matView, matProjection);
						paperObj.isLanding_[i] = paperAirplaneObjs[i].isLanding;
					}
					else {
						PaperCircleUpdate(&paperCircleObjs[i], matView, matProjection);
						paperObj.isLanding_[i] = paperCircleObjs[i].isLanding;
					}
				}

				if (paperObj.isCol[i] == 1) {
					if (paperObj.type[i] == 0) {
						PaperAirplaneOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperAirplaneObjs[i]);
					}
					else {
						PaperCircleOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperCircleObjs[i]);
					}
				}

			}

			//弾を外した時用
			if (playerBulletObj[shot].position.z >= 10) {

				playerBulletObj[shot].position.z = -100;
				playerBulletObj[shot].position.y = 100;
				playerBulletObj[shot].isDead_ = true;
				shot++;
				isLand = 1;
				scene = 2;

			}
		}
		else if (scene == 2) {

			//紙
			PaperUpdate(&paperObj, matView, matProjection);

			for (size_t i = 0; i < 10; i++) {
				//リセット
				if (paperObj.flag == 0) {
					if (paperObj.type[i] == 0) {
						PaperAirplaneReset(&paperAirplaneObjs[i]);
					}
					else {
						PaperCircleReset(&paperCircleObjs[i]);
					}
				}

				//Update

				else {
					if (paperObj.type[i] == 0) {
						PaperAirplaneUpdate(&paperAirplaneObjs[i], matView, matProjection);
						paperObj.isLanding_[i] = paperAirplaneObjs[i].isLanding;
					}
					else {
						PaperCircleUpdate(&paperCircleObjs[i], matView, matProjection);
						paperObj.isLanding_[i] = paperCircleObjs[i].isLanding;
					}
				}

				if (paperObj.isCol[i] == 1) {
					if (paperObj.type[i] == 0) {
						PaperAirplaneOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperAirplaneObjs[i]);
					}
					else {
						PaperCircleOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperCircleObjs[i]);
					}
				}

			}

			//if (input->PushKey(DIK_1)) {
			//	cameramode = 1;
			//}
			//else if (input->PushKey(DIK_2)) {
			//	cameramode = 2;
			//}

			//地面についたらshotを増やす,以前に落ちた弾は除外


			if (paperObj.type[touchPaperNum] == 0) {
				if (paperAirplaneObjs[touchPaperNum].isLanding == 1) {
					if (isLand == 0) {
						isLand = 1;
						beginshot[shot] = touchPaperNum;
						shot++;
						PaperSetIsCol(touchPaperNum, &paperObj);
					}
				}
			}
			else {
				if (paperCircleObjs[touchPaperNum].isLanding == 1) {
					if (isLand == 0) {
						isLand = 1;
						beginshot[shot] = touchPaperNum;
						shot++;
						PaperSetIsCol(touchPaperNum, &paperObj);
					}
				}
			}






			////弾を外した場合
			//const std::list<std::unique_ptr<FanWind>>& fanWinds = fan_->GetBullets();

			//for (const std::unique_ptr<FanWind>& fanwind : fanWinds) {
			//	if (fanwind->GetWorldPosition().z >= 100) {
			//		shot++;
			//	}
			//}

		}
		else if (scene == 3) {

		}

		//カメラ

		//if (input_->PushKey(DIK_0)) {
		//	cameramode = 0;
		//}
		//else if (input_->PushKey(DIK_1)) {
		//	cameramode = 1;
		//}
		//else if (input_->PushKey(DIK_2)) {
		//	cameramode = 2;
		//}

		if (cameramode == 0) {
			//視点座標
			eye = { 0,20,-30 };
			target = { 0, 0,0 };


			//viewProjection_.eye.z = paper_->GetWorldPosition(touchPaperNum).z + (-30);
			//viewProjection_.target.z = paper_->GetWorldPosition(touchPaperNum).z;
			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

		}
		else if (cameramode == 1) {
			//視点座標
			eye = { 50,0,3 };
			//viewProjection_.target = { 30, 0,0 };

			if (paperObj.type[touchPaperNum] == 0) {
				eye.z = paperAirplaneObjs[touchPaperNum].position.z;
				target.z = paperAirplaneObjs[touchPaperNum].position.z;
			}
			else {
				eye.z = paperCircleObjs[touchPaperNum].position.z;
				target.z = paperCircleObjs[touchPaperNum].position.z;
			}

			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

		}
		else if (cameramode == 2) {
			//視点座標
			eye = { 50,0,3 };
			target = { 30, 0,0 };

			if (paperObj.type[touchPaperNum] == 0) {
				eye.z = paperAirplaneObjs[touchPaperNum].position.z;
				target.z = paperAirplaneObjs[touchPaperNum].position.z;
			}
			else {
				eye.z = paperCircleObjs[touchPaperNum].position.z;
				target.z = paperCircleObjs[touchPaperNum].position.z;
			}

			matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

		}

		//debugText_->SetPos(0, 80);
		//debugText_->Printf("PaperTrans[%d] = (%f,%f,%f)", touchPaperNum, paper_->GetWorldPosition(touchPaperNum).x, paper_->GetWorldPosition(touchPaperNum).y, paper_->GetWorldPosition(touchPaperNum).z);

		//debugText_->SetPos(0, 140);
		//debugText_->Printf("scene = %d", scene);
		//debugText_->SetPos(0, 160);
		//debugText_->Printf("shot = %d", shot);
		//debugText_->SetPos(0, 180);
		//debugText_->Printf("isLand = %d", isLand);

		//シーン切り替え

		if (scene == 0) {
			if (input->TriggerKey(DIK_SPACE)) {
				scene = 1;
			}
		}
		else if (scene == 1 && paperObj.isCol[touchPaperNum] == 1) {
			scene = 2;
			cameramode = 2;
		}
		else if (scene == 2 && shot < 3 && isLand == 1) {

			scene = 1;
			PlayerReset(&playerObj);
			isLand = 0;
		}
		else if (scene == 2 && shot >= 3 && isLand == 1) {

			scene = 3;
			isLand = 0;

			//乱数シード生成器
			std::random_device seed_gen;
			//メルセンヌ・ツイスター
			std::mt19937_64 engine(seed_gen());

			//乱数範囲(くじ用)
			std::uniform_real_distribution<float> kuziDist(0, 5.9);

			//乱数範囲(くじ内容用)
			std::uniform_real_distribution<float> kuziCommentDist(0, 2.9);

			//くじ抽選
			kuzi = kuziDist(engine);

			//くじコメント抽選
			kuziComment = kuziCommentDist(engine);

			kuziComment = (kuzi * 3) + kuziComment;

			SpriteUpdate(kuziSprite[kuzi], spriteCommon);

			SpriteUpdate(kuziCommentSprite[kuziComment], spriteCommon);

		}
		else if (scene == 3) {
			if (input->TriggerKey(DIK_SPACE)) {
				scene = 0;
				shot = 0;
				PaperReset(&paperObj);
				for (int i = 0; i < 10; i++) {
					PaperAirplaneReset(&paperAirplaneObjs[i]);
					PaperCircleReset(&paperCircleObjs[i]);
				}

				PlayerReset(&playerObj);

				for (int i = 0; i < 3; i++) {
					playerBulletObj[i].isDead_ = false;
					playerBulletObj[i].position.y = 1000;
				}

				for (int i = 0; i < 3; i++) {
					score[i] = 0;
				}
			}

		}

		//if (input->TriggerKey(DIK_SPACE)) {
		//	if (incrementSize != 0) {
		//		incrementSize = 0;
		//	}
		//	else {
		//		incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//	}
		//}

		//------------スプライト-------------

		//sprite.rotation = 45;

		//パワーゲージUI
		//powerGaugeInSprite.position.x = powerGaugeFlameSprite.position.x + (playerObj.windPower * 58) + 7;

		powerGaugeInSprite.position.x = powerGaugeFlameSprite.position.x + 7;
		powerGaugeInSprite.position.y = powerGaugeFlameSprite.position.y + 7;
		SpriteUpdate(powerGaugeInSprite, spriteCommon);

		powerGaugeInSprite.size.x = (playerObj.windPower * 60);
		SpriteTransferVertexBuffer(powerGaugeInSprite);

		SpriteUpdate(powerGaugeFlameSprite, spriteCommon);

		clearSprite.position.x = WinApp::window_width / 4;
		SpriteUpdate(clearSprite, spriteCommon);
		//操作説明スプライト
		ruleSprite.position.x = 220;
		ruleSprite.position.y = 550;
		SpriteUpdate(ruleSprite, spriteCommon);

		//スコア

		if (scene == 2) { //スコアをshotごとに取る
			if (paperObj.type[touchPaperNum] == 0) {
				score[shot] = paperAirplaneObjs[touchPaperNum].position.z;
			}
			else {
				score[shot] = paperCircleObjs[touchPaperNum].position.z;
			}
		}
		else if (scene == 3) { //ハイスコア算出

			if (paperObj.type[beginshot[0]] == 0) {
				highScore = paperAirplaneObjs[beginshot[0]].position.z;
			}
			else {
				highScore = paperCircleObjs[beginshot[0]].position.z;
			}

			if (paperObj.type[beginshot[1]] == 0) {
				if (paperAirplaneObjs[beginshot[1]].position.z > highScore) {
					highScore = paperAirplaneObjs[beginshot[1]].position.z;
				}
			}
			else {
				if (paperCircleObjs[beginshot[1]].position.z > highScore) {
					highScore = paperCircleObjs[beginshot[1]].position.z;
				}
			}

			if (paperObj.type[beginshot[2]] == 0) {
				if (paperAirplaneObjs[beginshot[2]].position.z > highScore) {
					highScore = paperAirplaneObjs[beginshot[2]].position.z;
				}
			}
			else {
				if (paperCircleObjs[beginshot[2]].position.z > highScore) {
					highScore = paperCircleObjs[beginshot[2]].position.z;
				}
			}

			scoreSprite[3][0].texNumber = (highScore / 1000) + 10;
			scoreSprite[3][1].texNumber = (highScore / 100) + 10;
			scoreSprite[3][2].texNumber = (highScore / 10 % 10) + 10;
			scoreSprite[3][3].texNumber = (highScore % 10) + 10;

			if (scoreSprite[3][1].texNumber >= 10) {
				scoreSprite[3][1].texNumber - 10;
			}
		}

		//スコアをセット
		if (shot != 3) {
			scoreSprite[shot][0].texNumber = (score[shot] / 1000) + 10;
			scoreSprite[shot][1].texNumber = (score[shot] / 100) + 10;
			scoreSprite[shot][2].texNumber = (score[shot] / 10 % 10) + 10;
			scoreSprite[shot][3].texNumber = (score[shot] % 10) + 10;

			if (scoreSprite[shot][1].texNumber >= 10) {
				scoreSprite[shot][1].texNumber - 10;
			}
		}
		if (scene == 1 || scene == 2) {
			//座標と更新
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (shot == j) {
						scoreSprite[j][i].position.x = 500 + ((i + 1) * 64);
						scoreSprite[j][i].position.y = 100;
					}
					else {
						scoreSprite[j][i].position.x = 0 + ((i) * 64);
						scoreSprite[j][i].position.y = 0 + +(j * 64);
					}

					if (j == 3) {
						scoreSprite[j][i].position.x = 450 + ((i + 1) * 64);
						scoreSprite[j][i].position.y = 460;
					}

					SpriteUpdate(scoreSprite[j][i], spriteCommon);

				}
			}
		}
		//------------当たり判定---------------

		if (scene == 1 && paperObj.flag >= 3) {

			//判定対象AとBの座標
			Vector3 posA, posB;

			////自弾リストの取得
			//const std::list<std::unique_ptr<FanWind>>& fanWinds = fan_->GetBullets();

#pragma region 自弾と敵弾の当たり判定

			//自弾と敵弾の当たり判定
			for (int i = 0; i < 10; i++) {

				//自弾の座標
				posA = Vector3(playerBulletObj[shot].position.x, playerBulletObj[shot].position.y, playerBulletObj[shot].position.z);
				//敵弾の座標
				if (paperObj.type[i] == 0) {
					posB = Vector3(paperAirplaneObjs[i].position.x, paperAirplaneObjs[i].position.y, paperAirplaneObjs[i].position.z);
				}
				else {
					posB = Vector3(paperCircleObjs[i].position.x, paperCircleObjs[i].position.y, paperCircleObjs[i].position.z);
				}

				//半径
				float posAR = 1;
				float posBR = 1;

				if (((posA.x - posB.x) * (posA.x - posB.x)) + ((posA.y - posB.y) * (posA.y - posB.y)) + ((posA.z - posB.z) * (posA.z - posB.z)) <= ((posAR + posBR) * (posAR + posBR))) {


					PaperOnCollision(i, &paperObj);
					//自弾の衝突時コールバックを呼び出す
					playerBulletObj[shot].isDead_ = true;
					//敵弾の衝突時コールバックを呼び出す
					if (paperObj.type[i] == 0) {
						PaperAirplaneOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperAirplaneObjs[i]);
					}
					else {
						PaperCircleOnCollision(playerObj.windPower, Vector3(playerObj.position.x, playerObj.position.y, playerObj.position.z), &paperCircleObjs[i]);
					}

					touchPaperNum = i;

					//debugText_->SetPos(0, 40);
					//debugText_->Printf("atatta");
				}

			}
#pragma endregion
		}

		UpdateObject3d(&groundObj, matView, matProjection);

		//更新処理-ここまで

		// バックバッファの番号を取得（2つなので0番か1番）
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		// １．リソースバリアで書き込み可能に変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get(); // バックバッファを指定
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;      // 表示状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // 描画状態へ
		commandList->ResourceBarrier(1, &barrierDesc);

		// ２．描画先の変更
		// レンダーターゲットビューのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//深度ステンシルビューようでスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		// ３．画面クリア           R     G     B    A
		FLOAT clearColor[] = { 0.1f,0.25f, 0.5f,0.0f }; // 青っぽい色
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// ４．描画コマンドここから

		// ビューポート設定コマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = winApp->window_width;
		viewport.Height = winApp->window_height;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// ビューポート設定コマンドを、コマンドリストに積む
		commandList->RSSetViewports(1, &viewport);

		// シザー矩形
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;                                       // 切り抜き座標左
		scissorRect.right = winApp->window_width;        // 切り抜き座標右
		scissorRect.top = 0;                                        // 切り抜き座標上
		scissorRect.bottom = winApp->window_height;       // 切り抜き座標下
		// シザー矩形設定コマンドを、コマンドリストに積む
		commandList->RSSetScissorRects(1, &scissorRect);

		// パイプラインステートとルートシグネチャの設定コマンド
		commandList->SetPipelineState(object3dPipelineSet.pipelinestate.Get());
		commandList->SetGraphicsRootSignature(object3dPipelineSet.rootsignature.Get());

		// プリミティブ形状の設定コマンド
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 三角形リスト

		// 頂点バッファビューの設定コマンド
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// 定数バッファビュー(CBV)の設定コマンド
		commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
		// SRVヒープの設定コマンド
		commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());

		// SRVヒープの先頭ハンドルを取得（SRVを指しているはず）
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();

		// SRVヒープの先頭にあるSRVをルートパラメータ1番に設定
		//srvGpuHandle.ptr += incrementSize;
		//commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);


		//for (int i = 0; i < _countof(object3ds); i++) {
		//	DrawObject3d(&object3ds[i], commandList, vbView, ibView, _countof(indices));
		//}

		incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		DrawObject3d(&groundObj, commandList, vbView, ibView, _countof(indices), srvGpuHandle, incrementSize);

		////自機描画

		if (scene == 1 || scene == 2) {

			if (scene != 2) {
				incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				PlayerDraw(&playerObj, commandList, vbView, ibView, _countof(indices), srvGpuHandle, incrementSize);
				if (playerObj.mode == 3 && shot <= 2) {
					if (playerBulletObj[shot].isDead_ == false) {
						incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						PlayerBulletDraw(&playerBulletObj[shot], commandList, vbView, ibView, _countof(indices), srvGpuHandle, incrementSize);
					}
				}
			}

			if (paperObj.flag >= 3) {

				for (int i = 0; i < 10; i++) {

					if (paperObj.type[i] == 0) {
						incrementSize = 0;
						PaperAirplaneDraw(&paperAirplaneObjs[i], commandList, vbView, ibView, _countof(indices), srvGpuHandle, incrementSize);
					}
					else {
						incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
						PaperCircleDraw(&paperCircleObjs[i], commandList, vbView, ibView, _countof(indices), srvGpuHandle, incrementSize);
					}

				}
			}
		}

		//スプライト共通コマンド
		SpriteCommonBeginDraw(commandList.Get(), spriteCommon, srvHeap.Get());

		////スプライト描画
		if (scene == 0) {
			//タイトル
			SpriteDraw(titleSprite[titleNum], commandList.Get(), spriteCommon, device.Get());
		}
		else if (scene == 3) {
			//クリア
			SpriteDraw(clearSprite, commandList.Get(), spriteCommon, device.Get());
		}

		//UI

		if (scene == 1) {
			//パワーゲージ
			SpriteDraw(powerGaugeFlameSprite, commandList.Get(), spriteCommon, device.Get());
			SpriteDraw(powerGaugeInSprite, commandList.Get(), spriteCommon, device.Get());

			//操作説明
			if (shot == 0) {
				SpriteDraw(ruleSprite, commandList.Get(), spriteCommon, device.Get());
			}
		}

		//スコア
		if (scene == 1) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (j < shot && shot != 0) {
						if (j != 3) {
							SpriteDraw(scoreSprite[j][i], commandList.Get(), spriteCommon, device.Get());
						}
					}
				}
			}
		}
		else if (scene == 2) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (j <= shot) {
						if (j != 3) {
							SpriteDraw(scoreSprite[j][i], commandList.Get(), spriteCommon, device.Get());
						}
					}
				}
			}

		}
		else if (scene == 3) {
			for (int i = 0; i < 4; i++) {
				SpriteDraw(scoreSprite[3][i], commandList.Get(), spriteCommon, device.Get());
			}
			//くじ

			SpriteDraw(kuziSprite[kuzi], commandList.Get(), spriteCommon, device.Get());
			SpriteDraw(kuziCommentSprite[kuziComment], commandList.Get(), spriteCommon, device.Get());
		}

		// ４．描画コマンドここまで

		// ５．リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // 描画状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;        // 表示状態へ
		commandList->ResourceBarrier(1, &barrierDesc);

		// 命令のクローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		// コマンドリストの実行
		ID3D12CommandList* commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(1, commandLists);

		// 画面に表示するバッファをフリップ（裏表の入替え）
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		// コマンドの実行完了を待つ
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			if (event != 0) {
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
		}

		// キューをクリア
		result = cmdAllocator->Reset();
		assert(SUCCEEDED(result));
		// 再びコマンドリストを貯める準備
		result = commandList->Reset(cmdAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));

		// DirectX毎フレーム処理 ここまで

	}

	//コンソールへの文字出力
	OutputDebugStringA("Hello,DirectX!!\n");

	return 0;
}

//オブジェクト初期化処理
void InitializeObject3d(Object3d* object, ComPtr<ID3D12Device> device) {

	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));
}

//オブジェクト更新処理
void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {
	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//親オブジェクトがあれば
	if (object->parent != nullptr) {
		//親オブジェクトのワールド行列を掛ける
		object->matWorld *= object->parent->matWorld;
	}

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;
}

//オブジェクト描画処理
void DrawObject3d(Object3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize) {

	srvGpuHandle.ptr += (incrementSize * 4);
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

}

//---------------自機----------------------------

//初期化
void PlayerInitialize(PlayerObject3d* object, ComPtr<ID3D12Device> device) {

	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));
}

//更新
void PlayerUpdate(Input* input_, PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device, PlayerBulletObject3d playerBulletObj) {

	////デスフラグの立った球を削除
	//bullets_.remove_if([](std::unique_ptr<FanWind>& bullet) {
	//	return bullet->IsDead();
	//	});

	PlayerMove(object, matView, matProjection);
	//debugText_->SetPos(0, 0);
	//debugText_->Printf("FanPos(%f,%f,%f)", worldtransform_.translation_.x, worldtransform_.translation_.y, worldtransform_.translation_.z);
	//debugText_->SetPos(0, 20);
	//debugText_->Printf("power = %f", windPower);

	PlayerRotate(input_, object, matView, matProjection);

	//キャラクター攻撃処理
	PlayerAttack(input_, object, device, playerBulletObj);

	////弾更新
	//for (std::unique_ptr<FanWind>& bullet : bullets_) {
	//	bullet->Update();
	//}

	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;

};

//描画
void PlayerDraw(PlayerObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize) {

	srvGpuHandle.ptr += (incrementSize * 2);
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

	////弾描画
	//for (std::unique_ptr<FanWind>& bullet : bullets_) {
	//	bullet->Draw(viewProjection_);
	//}
}

void PlayerRotate(Input* input_, PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {


	//キャラクターの移動ベクトル
	Vector3 rot = { 0.0f,0.0f,0.0f };

	//キャラクターの移動速さ
	const float rotY = 0.01f;

	//移動ベクトルの変更
	if (input_->PushKey(DIK_R)) {
		rot = { 0.0f,rotY,0.0f };
	}
	else if (input_->PushKey(DIK_T)) {
		rot = { 0.0f,-rotY,0.0f };
	}

	//ベクトルの加算

	object->rotation.x += rot.x;
	object->rotation.y += rot.y;
	object->rotation.z += rot.z;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;

	//debugText_->SetPos(0, 20);
	//debugText_->Printf("FanRot(%f,%f,%f)", worldtransform_.rotation_.x, worldtransform_.rotation_.y, worldtransform_.rotation_.z);


};

void PlayerAttack(Input* input_, PlayerObject3d* object, ComPtr<ID3D12Device> device, PlayerBulletObject3d playerBulletObj) {

	object->keyCoolTime--;

	if (input_->PushKey(DIK_SPACE) && object->keyCoolTime <= 0) {

		object->push = 1;

		if (object->mode != 0 && object->mode != 4) {
			object->mode = 0;
			object->windPower = 0;
		}
	}
	else {
		object->push = 0;
	}

	//debugText_->SetPos(0, 100);
	//debugText_->Printf("%d", push);

	//停止中にゲージを付ける
	if (object->mode == 0) {
		PlayermeasureWindPower(object);
	}


	if (object->mode == 0 && object->push == 0) {

		object->mode = 3;

	}

}

void PlayerOnCollision() {

}

void PlayerReset(PlayerObject3d* object)
{
	////ワールド変換初期化
	//worldtransform_.translation_ = Vector3(0,0,0);
	//worldtransform_.matWorld_ = affine_->World(affine_->Scale(affine_->Scale_), affine_->Rot(affine_->RotX(worldtransform_.rotation_.x), affine_->RotY(worldtransform_.rotation_.y), affine_->RotZ(worldtransform_.rotation_.z)), affine_->Trans(worldtransform_.translation_));
	//worldtransform_.TransferMatrix();
	////ワールド変換初期化
	//worldtransform_.Initialize();

	object->mode = 1;
	object->push = 0;
	object->keyCoolTime = 100;

}

void PlayerSetMode(int i, PlayerObject3d* object)
{
	object->mode = i;
}

void PlayerMove(PlayerObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {

	PlayerModeChange(object);

	//キャラクターの移動ベクトル
	Vector3 move = { 0.0f,0.0f,0.0f };

	//移動ベクトルの変更
	if (object->mode == 1) {
		move = { (-object->speed),0,0 };
	}
	else if (object->mode == 2) {
		move = { object->speed,0,0 };
	}

	//ベクトルの加算
	object->position.x += move.x;
	object->position.y += move.y;
	object->position.z += move.z;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;

}

void PlayerModeChange(PlayerObject3d* object) {

	//プレイヤーの座標が限界値に行ったら向きを変更する
	if (object->position.x <= object->moveLimitLeft) {
		object->mode = 2;
	}
	else if (object->position.x >= object->moveLimitRight) {
		object->mode = 1;
	}

}


void PlayermeasureWindPower(PlayerObject3d* object) {

	if (object->windPower >= object->maxPower) {
		object->windPower = 0;
	}

	object->windPower += object->powerSpeed;

}

//-----------------自機弾-------------

void PlayerBulletInitialize(PlayerBulletObject3d* object, ComPtr<ID3D12Device> device, Vector3& position, Vector3& velocity) {

	object->velocity_.x = velocity.x;
	object->velocity_.y = velocity.y;
	object->velocity_.z = velocity.z;

	object->position.x = position.x;
	object->position.y = position.y;
	object->position.z = position.z;


	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));
}

void PlayerBulletUpdate(PlayerBulletObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {

	//座標を移動させる
	//ベクトルの加算
	object->position.x += object->velocity_.x;
	object->position.y += object->velocity_.y;
	object->position.z += object->velocity_.z;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;

	object->deathTimer_--;

	if ((object->deathTimer_) <= 0) {
		object->isDead_ = true;
	}

	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;
}

void PlayerBulletDraw(PlayerBulletObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize) {

	srvGpuHandle.ptr += (incrementSize * 3);
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}

//衝突したら呼び出されるコールバック関数
void PlayerBullletOnCollision(PlayerBulletObject3d* object);

//------------紙----------------

void PaperInitialize(PaperObject3d* object, ComPtr<ID3D12Device> device) {

	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));

	//object->paperAirplane_[i] = new PaperAirplane;
	//object->paperCircle_[i] = new PaperCircle;
}

void PaperUpdate(PaperObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {

	object->flag++;

	if (object->flag == 1) {
		PaperSet(object);
	}

	for (int i = 0; i < 10; i++) {
		//if (object->type[i] == 0) {
		//	object->paperAirplane_[i]->Update();
		//	object->isLanding_[i] = paperAirplane_[i]->GetIsLanding();
		//}
		//else {
		//	object->paperCircle_[i]->Update();
		//	object->isLanding_[i] = paperCircle_[i]->GetIsLanding();
		//}



	}
	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	////定数バッファにデータ転送
	//object->constMapTransform->mat = object->matWorld * matView * matProjection;
}

void PaperDraw(PaperObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices) {

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

}

//配置
void PaperSet(PaperObject3d* object) {

	for (int i = 0; i < 10; i++) {

		PaperSetTrans(i, object);
		PaperSetRot(i, object);

		//飛行機か丸型かランダム
		//乱数シード生成器
		std::random_device seed_gen;
		//メルセンヌ・ツイスター
		std::mt19937_64 engine(seed_gen());
		//乱数範囲
		std::uniform_real_distribution<float> typeDist(0, 1.2);

		float num = typeDist(engine);

		if (num < 1.0f) {
			object->type[i] = 0;
		}
		else {
			object->type[i] = 1;
		}

		//if (object->type[i] == 0) {
		//	object->paperAirplane_[i] = PaperAirplaneInitialize(planeModel_, planeTextureHandle_, trans[i], rot[i]);
		//}
		//else {
		//	object->paperCircle_[i] = PaperCircleInitialize(planeModel_, planeTextureHandle_, trans[i], rot[i]);
		//}
	}

}

//座標配置
void PaperSetTrans(int i, PaperObject3d* object) {
	//乱数シード生成器
	std::random_device seed_gen;
	//メルセンヌ・ツイスター
	std::mt19937_64 engine(seed_gen());
	//乱数範囲(座標用)
	std::uniform_real_distribution<float> transDist(0.0f, object->space);


	//乱数を配置する間隔の最大値と最小値で乱数を取る(*100と/100はint→float)
	object->trans[i].x = transDist(engine);

	if (i == 0) {
		object->trans[0].x = object->maxLeft;
	}

	//前の座標を足して座標を移動
	object->trans[i].x += object->beforeTrans;
	//今の座標を記録 + sizeでかぶりをなくす
	object->beforeTrans = (object->trans[i].x + object->size);

	object->trans[i].y = object->transY;
	object->trans[i].z = object->transZ;
}

//角度配置
void PaperSetRot(int i, PaperObject3d* object) {
	//乱数シード生成器
	std::random_device seed_gen;
	//メルセンヌ・ツイスター
	std::mt19937_64 engine(seed_gen());
	//乱数範囲(回転角用)
	std::uniform_real_distribution<float> rotDist(0.0f, 2 * PI);


	//乱数を配置する間隔の最大値と最小値で乱数を取る(*100と/100はint→float)
	object->rot[i].y = rotDist(engine);

	object->rot->x = object->rotX;
	object->rot->z = object->rotZ;

}

void PaperSetIsCol(int i, PaperObject3d* object) {
	object->isCol[i] = 2;
}

void PaperOnCollision(int i, PaperObject3d* object) {

	object->isCol[i] = 1;
}

void PaperReset(PaperObject3d* object) {
	object->flag = 0;
	object->beforeTrans = 0;

}

//------------紙飛行機-----------------

void PaperAirplaneInitialize(PaperAirplaneObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device) {

	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));

}


void PaperAirplaneUpdate(PaperAirplaneObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {

	PaperAirplaneCalculationSpeed(object);

	if (object->move == 1) {
		PaperAirplaneMove(object);
	}

	//落下判定
	PaperAirplaneLandingJudge(object);

	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;
}


void PaperAirplaneDraw(PaperAirplaneObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize) {

	srvGpuHandle.ptr += incrementSize;
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

}

//衝突したら呼び出されるコールバック関数
void PaperAirplaneOnCollision(float windPower, Vector3 fanTrans, PaperAirplaneObject3d* object) {
	object->move = 1;
	PaperAirplaneSet(windPower, fanTrans, object);
}

//速度計さん
void PaperAirplaneCalculationSpeed(PaperAirplaneObject3d* object) {
	//速度を計算
	object->velocity_ = { 0,(-object->fallSpeed),object->speed };

	//減速
	if (object->speed >= 0.1) {
		object->speed -= object->decelerationRate;
	}
}

void PaperAirplaneMove(PaperAirplaneObject3d* object) {
	//行列更新
	object->position.x += object->velocity_.x;
	object->position.y += object->velocity_.y;
	object->position.z += object->velocity_.z;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld;

}

//速度などをセット
void PaperAirplaneSet(float windPower, Vector3 fanTrans, PaperAirplaneObject3d* object) {
	//速度,落下スピード,減速をセット

//風速をそのままスピードに代入
	object->speed = (windPower / 10) + 0.01;//0.1～1.1の範囲

	//紙飛行機と扇風機の座標の差が落下スピード
	//if文でマイナスにならないようにする
	if ((fanTrans.x - object->position.x) > 0) {
		object->fallSpeed = (fanTrans.x - object->position.x) / 10;
	}
	else {
		object->fallSpeed = (object->position.x - fanTrans.x) / 10;
	}

	if (object->fallSpeed <= 0.02) {
		object->fallSpeed = 0.02;
	}

	//減速率を計算,角度が0(まっすぐ)に近いほど減速率は少ない
	if (object->rotation.y < PI) {
		object->decelerationRate = (object->rotation.y);
	}
	else {
		object->decelerationRate = (object->rotation.y - PI);

		if (object->decelerationRate < (PI / 2)) {
			object->decelerationRate = (PI - object->decelerationRate);
		}
		else {
			object->decelerationRate = (PI - object->decelerationRate);
			//decelerationRate_ += (decelerationRate_ + decelerationRate_);
		}

	}

	object->decelerationRate = object->decelerationRate / 100;
}

//着地してるかしてないか
void PaperAirplaneLandingJudge(PaperAirplaneObject3d* object) {
	if (object->position.y <= object->endY) {
		object->isLanding = 1;
		object->move = 0;
	}
}

void PaperAirplaneReset(PaperAirplaneObject3d* object) {
	//0 停止 1 移動
	object->move = 0;
	object->isLanding = 0;
}

//------------丸紙-----------------

void PaperCircleInitialize(PaperCircleObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection, ComPtr<ID3D12Device> device) {

	HRESULT result;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resdesc{};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));
}


void PaperCircleUpdate(PaperCircleObject3d* object, XMMATRIX& matView, XMMATRIX& matProjection) {

	PaperCircleCalculationSpeed(object);

	if (object->move == 1) {
		PaperCircleMove(object);
	}

	//落下判定
	PaperCircleLandingJudge(object);

	XMMATRIX matScale, matRot, matTrans;

	//スケール,回転,平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	//回転角
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	//座標
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld * matView * matProjection;

}


void PaperCircleDraw(PaperCircleObject3d* object, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle, UINT incrementSize) {

	srvGpuHandle.ptr += incrementSize;
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 頂点バッファビューの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);

}

//衝突したら呼び出されるコールバック関数
void PaperCircleOnCollision(float windPower, Vector3 fanTrans, PaperCircleObject3d* object) {
	object->move = 1;
	PaperCircleSet(windPower, fanTrans, object);
}

//速度計さん
void PaperCircleCalculationSpeed(PaperCircleObject3d* object) {
	//速度を計算
	object->velocity_ = { 0,(-object->fallSpeed),object->speed };

	//減速
	if (object->speed >= 0.1) {
		object->speed -= object->decelerationRate;
	}
}

void PaperCircleMove(PaperCircleObject3d* object) {

	//行列更新
	object->position.x += object->velocity_.x;
	object->position.y += object->velocity_.y;
	object->position.z += object->velocity_.z;

	//定数バッファにデータ転送
	object->constMapTransform->mat = object->matWorld;

}

//速度などをセット
void PaperCircleSet(float windPower, Vector3 fanTrans, PaperCircleObject3d* object) {
	//速度,落下スピード,減速をセット

	//風速をそのままスピードに代入
	object->speed = (windPower / 10) + 0.01;//0.1～1.1の範囲

	//紙飛行機と扇風機の座標の差が落下スピード
	//if文でマイナスにならないようにする
	if ((fanTrans.x - object->position.x) > 0) {
		object->fallSpeed = (fanTrans.x - object->position.x) / 10;
	}
	else {
		object->fallSpeed = (object->position.x - fanTrans.x) / 10;
	}

	if (object->fallSpeed <= 0.02) {
		object->fallSpeed = 0.02;
	}

	//減速率を計算,角度が0(まっすぐ)に近いほど減速率は少ない


	object->decelerationRate = 0.01;
}

//着地してるかしてないか
void PaperCircleLandingJudge(PaperCircleObject3d* object) {
	if (object->position.y <= object->endY) {
		object->isLanding = 1;
		object->move = 0;
	}
}

void PaperCircleReset(PaperCircleObject3d* object) {
	//0 停止 1 移動
	object->move = 0;
	object->isLanding = 0;
}

PipelineSet Object3dCreateGraphicsPipeline(ID3D12Device* dev) {


	HRESULT result;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/BasicVS.hlsl", // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);

	// エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/BasicPS.hlsl",   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	// エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xyz座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//法線ベクトル
			"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
			D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},

		{ // uv座標(1行で書いたほうが見やすい)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};




	// デスクリプタレンジの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;         //一度の描画に使うテクスチャが1枚なので1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;     //テクスチャレジスタ0番
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメータの設定
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	// 定数バッファ0番
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // 種類
	rootParams[0].Descriptor.ShaderRegister = 0;                   // 定数バッファ番号
	rootParams[0].Descriptor.RegisterSpace = 0;                    // デフォルト値
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // 全てのシェーダから見える
	// テクスチャレジスタ0番
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   //種類
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;		  //デスクリプタレンジ
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;              		  //デスクリプタレンジ数
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;               //全てのシェーダから見える
	// 定数バッファ1番
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //種類
	rootParams[2].Descriptor.ShaderRegister = 1;		  //デスクリプタレンジ
	rootParams[2].Descriptor.RegisterSpace = 0;              		  //デスクリプタレンジ数
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;                // デフォルト値

	// テクスチャサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //横繰り返し（タイリング）
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //縦繰り返し（タイリング）
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //奥行繰り返し（タイリング）
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //全てリニア補間
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;                                              //ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //ピクセルシェーダからのみ使用可能

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// シェーダーの設定
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// サンプルマスクの設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	// ラスタライザの設定
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;  // カリングしない
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ポリゴン内塗りつぶし
	pipelineDesc.RasterizerState.DepthClipEnable = true; // 深度クリッピングを有効に

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA全てのチャンネルを描画

	//blenddesc.BlendEnable = true;                   // ブレンドを有効にする
	//blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    // 加算
	//blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      // ソースの値を100% 使う
	//blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    // デストの値を  0% 使う

	//// 半透明合成
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // 加算
	//blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // ソースのアルファ値
	//blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;    // 1.0f-ソースのアルファ値

	// 頂点レイアウトの設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// その他の設定
	pipelineDesc.NumRenderTargets = 1; // 描画対象は1つ
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	//デプスステンシルステートの設定
	pipelineDesc.DepthStencilState.DepthEnable = true;
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// ルートシグネチャ
	//ComPtr<ID3D12RootSignature> rootSignature;
	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = _countof(rootParams);        //ルートパラメータ数

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//パイプラインと√シグネチャのセット
	PipelineSet pipelineSet;

	// ルートシグネチャのシリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineSet.rootsignature));
	assert(SUCCEEDED(result));

	// パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = pipelineSet.rootsignature.Get();

	// パイプランステートの生成
	result = dev->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineSet.pipelinestate));
	assert(SUCCEEDED(result));


	//パイプラインとルートシグネチャを返す
	return pipelineSet;
}

PipelineSet SpriteCreateGraphicsPipeline(ID3D12Device* dev) {

	HRESULT result;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/SpriteVS.hlsl", // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);

	// エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/SpritePS.hlsl",   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	// エラーなら
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xyz座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},

		{ // uv座標(1行で書いたほうが見やすい)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};




	// デスクリプタレンジの設定
	CD3DX12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	//descriptorRange.NumDescriptors = 1;         //一度の描画に使うテクスチャが1枚なので1
	//descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//descriptorRange.BaseShaderRegister = 0;     //テクスチャレジスタ0番
	//descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//// ルートパラメータの設定
	//D3D12_ROOT_PARAMETER rootParams[3] = {};
	//// 定数バッファ0番
	//rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // 種類
	//rootParams[0].Descriptor.ShaderRegister = 0;                   // 定数バッファ番号
	//rootParams[0].Descriptor.RegisterSpace = 0;                    // デフォルト値
	//rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // 全てのシェーダから見える
	//// テクスチャレジスタ0番
	//rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   //種類
	//rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;		  //デスクリプタレンジ
	//rootParams[1].DescriptorTable.NumDescriptorRanges = 1;              		  //デスクリプタレンジ数
	//rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;               //全てのシェーダから見える
	//// 定数バッファ1番
	//rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //種類
	//rootParams[2].Descriptor.ShaderRegister = 1;		  //デスクリプタレンジ
	//rootParams[2].Descriptor.RegisterSpace = 0;              		  //デスクリプタレンジ数
	//rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;                // デフォルト値

	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsConstantBufferView(0);
	rootParams[1].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_ALL);

	// テクスチャサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //横繰り返し（タイリング）
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //縦繰り返し（タイリング）
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //奥行繰り返し（タイリング）
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //全てリニア補間
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;                                              //ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //ピクセルシェーダからのみ使用可能

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// シェーダーの設定
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// サンプルマスクの設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	// ラスタライザの設定
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // カリングしない
	//pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ポリゴン内塗りつぶし
	//pipelineDesc.RasterizerState.DepthClipEnable = true; // 深度クリッピングを有効に

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA全てのチャンネルを描画

	//blenddesc.BlendEnable = true;                   // ブレンドを有効にする
	//blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    // 加算
	//blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      // ソースの値を100% 使う
	//blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    // デストの値を  0% 使う

	//// 半透明合成
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // 加算
	//blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // ソースのアルファ値
	//blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;    // 1.0f-ソースのアルファ値

	// 頂点レイアウトの設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// その他の設定
	pipelineDesc.NumRenderTargets = 1; // 描画対象は1つ
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	//デプスステンシルステートの設定
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // 常に上書きルール

	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DepthStencilState.DepthEnable = false;
	//pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// ルートシグネチャ
	//ComPtr<ID3D12RootSignature> rootSignature;
	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = _countof(rootParams);        //ルートパラメータ数

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//パイプラインと√シグネチャのセット
	PipelineSet pipelineSet;

	// ルートシグネチャのシリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineSet.rootsignature));
	assert(SUCCEEDED(result));

	// パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = pipelineSet.rootsignature.Get();

	// パイプランステートの生成
	result = dev->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineSet.pipelinestate));
	assert(SUCCEEDED(result));


	//パイプラインとルートシグネチャを返す
	return pipelineSet;
}

Sprite SpriteCreate(ID3D12Device* dev, int window_width, int window_height) {

	HRESULT result = S_FALSE;

	//新しいスプライトを作る
	Sprite sprite{};

	//頂点データ
	VertexPosUv vertices[] = {
		{{	0.0f, 100.0f,	0.0f},{0.0f,1.0f}},
		{{	0.0f,	0.0f,	0.0f},{0.0f,0.0f}},
		{{100.0f, 100.0f,	0.0f},{1.0f,1.0f}},
		{{100.0f,	0.0f,	0.0f},{1.0f,0.0f}},
	};

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapPropsVertexBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDescVertexBuffer =
		CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv));

	//頂点バッファ生成
	result = dev->CreateCommittedResource(
		&heapPropsVertexBuffer, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resourceDescVertexBuffer, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.vertBuff));
	assert(SUCCEEDED(result));

	//頂点バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	// GPU仮想アドレス
	sprite.vbView.BufferLocation = sprite.vertBuff->GetGPUVirtualAddress();
	// 頂点バッファのサイズ
	sprite.vbView.SizeInBytes = sizeof(vertices);
	// 頂点1つ分のデータサイズ
	sprite.vbView.StrideInBytes = sizeof(vertices[0]);

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapPropsConstantBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDescConstantBuffer =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

	// 定数バッファの生成
	result = dev->CreateCommittedResource(
		&heapPropsConstantBuffer, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resourceDescConstantBuffer, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.constBuff));
	assert(SUCCEEDED(result));

	// 定数バッファにデータ転送
	ConstBufferData* constMap = nullptr;
	result = sprite.constBuff->Map(0, nullptr, (void**)&constMap); // マッピング
	constMap->color = XMFLOAT4(1, 1, 1, 1);
	assert(SUCCEEDED(result));

	//平行投影行列
	constMap->mat = XMMatrixOrthographicOffCenterLH(0.0f, window_width, window_height, 0.0f, 0.0f, 1.0f);
	sprite.constBuff->Unmap(0, nullptr);

	return sprite;
}
//スプライト共通グラフィックスコマンドのセット
void SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12DescriptorHeap* descHeap) {

	// パイプラインステートとルートシグネチャの設定コマンド
	cmdList->SetPipelineState(spriteCommon.pipelineSet.pipelinestate.Get());
	cmdList->SetGraphicsRootSignature(spriteCommon.pipelineSet.rootsignature.Get());

	// プリミティブ形状の設定コマンド
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // 三角形リスト

	//テクスチャ用でスクリプタヒープの設定
	ID3D12DescriptorHeap* ppHeaps[] = { spriteCommon.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

//スプライト単体描画

void SpriteDraw(const Sprite& sprite, ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon, ID3D12Device* dev) {

	// 頂点バッファの設定コマンド
	cmdList->IASetVertexBuffers(0, 1, &sprite.vbView);

	// 定数バッファ(CBV)の設定コマンド
	cmdList->SetGraphicsRootConstantBufferView(0, sprite.constBuff->GetGPUVirtualAddress());

	//シェーダーリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(1, CD3DX12_GPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetGPUDescriptorHandleForHeapStart(),
		sprite.texNumber, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

	//ポリゴンの描画
	cmdList->DrawInstanced(4, 1, 0, 0);

}

SpriteCommon SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height) {
	HRESULT result = S_FALSE;

	//新たなスプライト共通データを生成
	SpriteCommon spriteCommon{};

	//スプライト用パイプライン生成
	spriteCommon.pipelineSet = SpriteCreateGraphicsPipeline(dev);

	//平行投影行列生成
	spriteCommon.matProjrction = XMMatrixOrthographicOffCenterLH(0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);

	//HRESULT result = S_FALSE;

	//デスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = spriteSRVCount;
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&spriteCommon.descHeap));

	//生成したスプライト今日てううでーたを返す
	return spriteCommon;
}

void SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon) {

	sprite.matWorld = XMMatrixIdentity();

	sprite.matWorld *= XMMatrixRotationZ(XMConvertToRadians(sprite.rotation));

	sprite.matWorld *= XMMatrixTranslation(sprite.position.x, sprite.position.y, sprite.position.z);


	// 定数バッファにデータ転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = sprite.constBuff->Map(0, nullptr, (void**)&constMap); // マッピング
	constMap->mat = sprite.matWorld * spriteCommon.matProjrction;
	sprite.constBuff->Unmap(0, nullptr);
}

void SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev) {

	assert(texnumber <= spriteSRVCount - 1);

	HRESULT result;
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// WICテクスチャのロード
	result = LoadFromWICFile(filename, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps =
		CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// テクスチャ用バッファの生成
	result = dev->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr, IID_PPV_ARGS(&spriteCommon.texBuff[texnumber]));
	assert(SUCCEEDED(result));

	// テクスチャバッファにデータ転送
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		const Image* img = scratchImg.GetImage(i, 0, 0); // 生データ抽出
		result = spriteCommon.texBuff[texnumber]->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	// シェーダリソースビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = spriteCommon.texBuff[texnumber]->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(spriteCommon.texBuff[texnumber].Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		CD3DX12_CPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetCPUDescriptorHandleForHeapStart(), texnumber, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	);

	//return S_OK;
}

void SpriteTransferVertexBuffer(const Sprite& sprite) {

	HRESULT result = S_FALSE;

	//頂点データ
	VertexPosUv vertices[] = {
		{{},{0.0f,1.0f}},
		{{},{0.0f,0.0f}},
		{{},{1.0f,1.0f}},
		{{},{1.0f,0.0f}},
	};

	enum { LB, LT, RB, RT };

	vertices[LB].pos = { 0.0f,sprite.size.y,0.0f };
	vertices[LT].pos = { 0.0f,0.0f,0.0f };
	vertices[RB].pos = { sprite.size.x,sprite.size.y,0.0f };
	vertices[RT].pos = { sprite.size.x,0.0f,0.0f };

	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);
}
