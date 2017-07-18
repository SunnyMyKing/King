#pragma once
#ifndef BASEMATRIX_H
#define BASEMATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

struct Matrix_2D
{
	int height;
	int width;
	Matrix_2D() :height(0), width(0){}
};

class BaseMatrix
{
public:
	BaseMatrix();
	~BaseMatrix();
	void Init();
	void Delete_Matrix();
	void Input_Matrix_Size(const Matrix_2D &_Size);
	void Input_Matrix_Num(const int *_Nums);    //列优先，先调用BaseMatrix::Input_Matrix_Size, 再调用BaseMatrix::Input_Matrix_Num
	Matrix_2D Get_Matrix_Size();
	int Get_Matrix_Height();
	int Get_Matrix_Width();
	int* GetMatrix_Num();    //列优先
	int Get_Matrix_Value( const int height, const int width);
	void Set_Matrix_Value(const int height, const int width, const int _Value);
private:
	int **m_Matrix;
	Matrix_2D m_Matrix_Size;

};

#endif //BASEMATRIX_H
