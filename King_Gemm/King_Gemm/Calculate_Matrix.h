#pragma once
#ifndef __CALCULATE_MATRIX_H__
#define __CALCULATE_MATRIX_H__

#include "BaseMatrix.h"

class Calculate_Matrix
{
public:
	Calculate_Matrix();
	~Calculate_Matrix();
	void Init();
	void Calculate_Out_Size();
	void Native_Cal();
protected:
	void Init_In();
	void Init_In_Size();
	void Init_In_Num();
	void Init_Out_Size();
private:
	BaseMatrix m_In_a;
	BaseMatrix m_In_b;
	BaseMatrix m_Out_c;
	Matrix_2D m_Out_Size;
	Matrix_2D m_In_a_Size;
	Matrix_2D m_In_b_Size;
	int *m_In_a_Num;
	int *m_In_b_Num;
};


#endif //CALCULATE_MATRIX_H
