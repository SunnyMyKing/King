#include "stdafx.h"
#include "Calculate_Matrix.h"


Calculate_Matrix::Calculate_Matrix()
{
	Init();
}


Calculate_Matrix::~Calculate_Matrix()
{
}

void Calculate_Matrix::Init()
{
	Init_In();
	Init_Out_Size();
}

void Calculate_Matrix::Init_In()
{
	m_In_a.Input_Matrix_Size(m_In_a_Size);
	m_In_b.Input_Matrix_Size(m_In_b_Size);
	m_In_a.Input_Matrix_Num(m_In_a_Num);
	m_In_a.Input_Matrix_Num(m_In_b_Num);
}

void Calculate_Matrix::Calculate_Out_Size()
{
	m_Out_Size.height = m_In_a.Get_Matrix_Height();
	m_Out_Size.width = m_In_b.Get_Matrix_Width();
}

void Calculate_Matrix::Init_Out_Size()
{
	Calculate_Out_Size();
	m_Out_c.Input_Matrix_Size(m_Out_Size);
}

void Calculate_Matrix::Native_Cal()
{
	std::chrono::system_clock::time_point Native_start = std::chrono::system_clock::now();
	for (int i = 0; i < m_In_a.Get_Matrix_Height(); i++)
	{
		for (int j = 0; j < m_In_b.Get_Matrix_Width(); j++)
		{
			for (int p = 0; p < m_In_a.Get_Matrix_Width(); p++)
			{
				m_Out_c.Set_Matrix_Value(i, j, (m_Out_c.Get_Matrix_Value(i, j) + m_In_a.Get_Matrix_Value(i, p) * m_In_b.Get_Matrix_Value(p, j)));
			}
		}
	}
}