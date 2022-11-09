#include "Sound.h"

SoundData Sound::SoundLoadWave(const char* filename) {

	HRESULT result;

	//-----�t�@�C���I�[�v��------

	//�t�@�C�����̓X�g���[���̃C���X�^���X
	std::ifstream file;
	//.wav�t�@�C�����o�C�i���[���[�h�ŊJ��
	file.open(filename, std::ios_base::binary);
	//�t�@�C���I�[�v�����s�����o����
	assert(file.is_open());

	//-----.wav�f�[�^�ǂݍ���-----

	//RIFF�w�b�_�[�̓ǂݍ���
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//�t�@�C����RIFF���`�F�b�N
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//�t�@�C����WAVE���`�F�b�N
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//Format�`�����N�̓ǂݍ���
	FormatChunk format = {};
	//�`�����N�w�b�_�\�̊m�F
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//�`�����N�{�̂̓ǂݍ���
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Data�`�����N�̓ǂݍ���
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//JUNK�`�����N�����o�����ꍇ
	if (strncmp(data.id, "JUNK ", 4) == 0) {
		//�ǂݎ��ʒu��JUNK�`�����N�̏I���܂Ői�߂�
		file.seekg(data.size, std::ios_base::cur);
		//�ēǂݍ���
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data ", 4) != 0) {
		assert(0);
	}

	//Data�`�����N�̃f�[�^��(�g�`�f�[�^)�̓ǂݍ���
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Wave�t�@�C�������
	file.close();

	//------�ǂݍ��񂾉����f�[�^��return------

	//return���邽�߂̉����f�[�^
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

//�����f�[�^���
void Sound::SoundUnload(SoundData* soundData) {

	//�o�b�t�@�̃����������
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};

}

//�����Đ�
void Sound::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {

	xAudio2->StartEngine();

	HRESULT result;

	//�g�`�t�H�[�}�b�g�����Ƃ�SoundVoice�̐���
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert((SUCCEEDED(result)));

	//�Đ�����g�`�f�[�^�̐ݒ�
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//�g�`�f�[�^�̍Đ�
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void Sound::SoundLoopPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{

	HRESULT result;

	//�g�`�t�H�[�}�b�g�����Ƃ�SoundVoice�̐���
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert((SUCCEEDED(result)));

	//�Đ�����g�`�f�[�^�̐ݒ�
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//���[�v
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;

	//�g�`�f�[�^�̍Đ�
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

}

void Sound::SoundStopWave(IXAudio2* xAudio2, const SoundData& soundData)
{

	HRESULT result;
	result = pSourceVoice->Stop();

}

void Sound::SoundSetVolume(float volume)
{
	HRESULT result;
	result = pSourceVoice->SetVolume(volume);

}
