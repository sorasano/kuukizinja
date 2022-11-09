#pragma once
#include <cassert>

//音

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <fstream>

//音声データ読み込み(.wav)

//チャンクヘッダ
struct ChunkHeader {
	char id[4]; // チャンク毎のID
	int32_t size;// チャンクサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt; //　波形フォーマット
};

//音声データ
struct SoundData {

	//　波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;

};

class Sound
{

public:

	//音声データ読み込み
	SoundData SoundLoadWave(const char* filename);

	//音声データ解放
	void SoundUnload(SoundData* soundData);

	//音声再生

	//1回のみ再生
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	//ループ再生
	void SoundLoopPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	//停止
	void SoundStopWave(IXAudio2* xAudio2, const SoundData& soundData);

	//音量調節
	void SoundSetVolume(float volume);

private:

	IXAudio2SourceVoice* pSourceVoice = nullptr;

	XAUDIO2_BUFFER buf{};

};

