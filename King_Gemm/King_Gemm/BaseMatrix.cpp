#include "stdafx.h"
#include "BaseMatrix.h"


BaseMatrix::BaseMatrix()
{

}

BaseMatrix::~BaseMatrix()
{
	Delete_Matrix();
}

void BaseMatrix::Input_Matrix_Size(const Matrix_2D& _Size)
{
	m_Matrix_Size.height = _Size.height;
	m_Matrix_Size.width = _Size.width;
	Init();
}

void BaseMatrix::Input_Matrix_Num(const int *_Nums)
{
	for (int i = 0; i < m_Matrix_Size.height; i++)
	{
		for (int j = 0; j < m_Matrix_Size.width; j++)
		{
			m_Matrix[i][j] = _Nums[i*m_Matrix_Size.width + j];
		}
	}
}

void BaseMatrix::Init()
{
	m_Matrix = (int **)malloc(sizeof(int *) * m_Matrix_Size.height);
	for (int i = 0; i < m_Matrix_Size.height; i++)
	{
		m_Matrix[i] = (int *)malloc(sizeof(int) * m_Matrix_Size.width);
		memset(m_Matrix[i], 0, sizeof(int) * m_Matrix_Size.width);
	}
}

void BaseMatrix::Delete_Matrix()
{
	for (int i = 0; i < m_Matrix_Size.height; i++)
	{
		free(m_Matrix[i]);
	}
	free(m_Matrix);
}

Matrix_2D BaseMatrix::Get_Matrix_Size()
{
	return m_Matrix_Size;
}

int *BaseMatrix::GetMatrix_Num()
{
	int *Get_Num;
	Get_Num = (int *)malloc(sizeof(int) * m_Matrix_Size.height * m_Matrix_Size.width);

	return Get_Num;
}

int BaseMatrix::Get_Matrix_Height()
{
	return m_Matrix_Size.height;
}

int BaseMatrix::Get_Matrix_Width()
{
	return m_Matrix_Size.width;
}

int BaseMatrix::Get_Matrix_Value(const int height, const int width)
{
	return m_Matrix[height][width];
}

void BaseMatrix::Set_Matrix_Value(const int height, const int width, const int value)
{
	m_Matrix[height][width] = value;
}