#pragma once

#include "Matrix4.h"
#include "Vector3.h"


class Affine
{

public:

	Matrix4 Afiine_;
	Vector3 Scale_;
	Vector3 Rot_;
	Vector3 Trans_;

public:

	//コンストラクタ
	Affine();

	//デストラクタ
	~Affine();

	//ワールド変換行列

	//拡大縮小
	Matrix4 Scale(Vector3 Scale);

	//回転X
	Matrix4 RotX(float roteX);

	//回転Y
	Matrix4 RotY(float roteY);

	//回転Z
	Matrix4 RotZ(float roteZ);

	//回転合成
	Matrix4 Rot(Matrix4 RotX, Matrix4 RotY, Matrix4 RotZ);

	//平行移動
	Matrix4 Trans(Vector3 Trans);

	//ワールド変換行列
	Matrix4 World(Matrix4 Scale, Matrix4 Rot, Matrix4 Trans);

};

