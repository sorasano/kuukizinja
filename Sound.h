#pragma once
#include <cassert>

//��

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <fstream>

//�����f�[�^�ǂݍ���(.wav)

//�`�����N�w�b�_
struct ChunkHeader {
	char id[4]; // �`�����N����ID
	int32_t size;// �`�����N�T�C�Y
};

//RIFF�w�b�_�`�����N
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

//FMT�`�����N
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt; //�@�g�`�t�H�[�}�b�g
};

//�����f�[�^
struct SoundData {

	//�@�g�`�t�H�[�}�b�g
	WAVEFORMATEX wfex;
	// �o�b�t�@�̐擪�A�h���X
	BYTE* pBuffer;
	// �o�b�t�@�̃T�C�Y
	unsigned int bufferSize;

};

class Sound
{

public:

	//�����f�[�^�ǂݍ���
	SoundData SoundLoadWave(const char* filename);

	//�����f�[�^���
	void SoundUnload(SoundData* soundData);

	//�����Đ�

	//1��̂ݍĐ�
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	//���[�v�Đ�
	void SoundLoopPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	//��~
	void SoundStopWave(IXAudio2* xAudio2, const SoundData& soundData);

	//���ʒ���
	void SoundSetVolume(float volume);

private:

	IXAudio2SourceVoice* pSourceVoice = nullptr;

	XAUDIO2_BUFFER buf{};

};

