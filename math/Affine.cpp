#include "Affine.h"
#include <cmath.h>

Affine::Affine() {

	this->Afiine_ = {};
	this->Scale_ = { 1.0f,1.0f,1.0f };
	this->Rot_ = { 0.0f,0.0f,0.0f };
	this->Trans_ = { 0.0f,  0.0f, 0.0f };

}

Affine::~Affine() {

}

//���[���h�ϊ��s��

//�g��k��
Matrix4 Affine::Scale(Vector3 Scale) {

	//�X�P�[�����O�s���錾
	Matrix4 matScale;

	//�X�P�[�����O�{�����s���ݒ肷��
	matScale = { Scale.x, 0.0f, 0.0f,    0.0f, 0.0f, Scale.y, 0.0f, 0.0f,
				0.0f,    0.0f, Scale.z, 0.0f, 0.0f, 0.0f,    0.0f, 1.0f };

	return matScale;
};

//��]X
Matrix4 Affine::RotX(float roteX) {

	Matrix4 matRotX;

	float a = (float)cos(roteX);
	float b = (float)sin(roteX);
	float c = (float)-sin(roteX);
	float d = (float)cos(roteX);


	// X����]�s��̊e�v�f��ݒ肷��
	matRotX = { 1.0f, 0.0f,        0.0f,       0.0f, 0.0f,a, b, 0.0f,
			   0.0f, c, d, 0.0f, 0.0f, 0.0f,       0.0f,       1.0f };

	return matRotX;
};

//��]Y
Matrix4 Affine::RotY(float roteY) {

	Matrix4 matRotY;

	float a = (float)cos(roteY);
	float b = (float)-sin(roteY);
	float c = (float)sin(roteY);
	float d = (float)cos(roteY);

	// Y����]�s��̊e�v�f��ݒ肷��

	matRotY = { a, 0.0f, b, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			  c, 0.0f, d,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return matRotY;
};

//��]Z
Matrix4 Affine::RotZ(float roteZ) {

	Matrix4 matRotZ;

	float a = (float)cos(roteZ);
	float b = (float)sin(roteZ);
	float c = (float)-sin(roteZ);
	float d = (float)cos(roteZ);

	// Z����]�s��̊e�v�f��ݒ肷��

	matRotZ = { a, b, 0.0f, 0.0f, c, d, 0.0f, 0.0f,
			   0.0f,       0.0f,       1.0f, 0.0f, 0.0f,        0.0f,       0.0f, 1.0f };

	return matRotZ;
};

//��]����
Matrix4 Affine::Rot(Matrix4 RotX, Matrix4 RotY, Matrix4 RotZ) {

	// �����p��]�s���錾
	Matrix4 matRot;

	//�e���̉�]�s�������
	matRot = (RotZ *= RotX *= RotY);

	return matRot;
};

//���s�ړ�
Matrix4 Affine::Trans(Vector3 Trans) {

	//�X�P�[�����O�s���錾
	Matrix4 matTrans;

	//�X�P�[�����O�{�����s���ݒ肷��
	matTrans = { 1.0f, 0.0f, 0.0f, 0.0f, 
				0.0f,  1.0f,    0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				Trans.x, Trans.y, Trans.z, 1.0f };

	return matTrans;
};

//���[���h�ϊ��s��
Matrix4 Affine::World(Matrix4 Scale, Matrix4 Rot, Matrix4 Trans) {

	Matrix4 worldTransform_;

	//�s��̍���

	//�P�ʍs���������
	worldTransform_ = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.f,  0.0f, 1.0f };

	//�|���Z���đ������
	worldTransform_ *= Scale *= Rot *= Trans;

	////�s��̓]��
	// worldTransform.TransferMatrix();

	return worldTransform_;
};