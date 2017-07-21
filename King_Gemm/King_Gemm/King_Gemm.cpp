// King_Gemm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

struct Matrix_2D
{
	int width;
	int height;
	Matrix_2D() :width(0), height(0){}
};

struct Input_Matrix_2D
{
	Matrix_2D A_2D;
	Matrix_2D B_2D;
	Matrix_2D C_2D;
	Input_Matrix_2D(){}
};

void swap(double &a, double &b)
{
	a = a + b;
	b = a - b;
	a = a - b;
}

void Calculate_Matrix_1x4(const Matrix_2D& A_2D, const Matrix_2D& B_2D, const Matrix_2D& C_2D,double **A,double **B, double **C);//采用1x4进行加速
void Calculate_Matrix_4x4(const Matrix_2D& A_2D, const Matrix_2D& B_2D, const Matrix_2D& C_2D, double **A, double **B, double **C);
void AddDot1x4(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C); //100*100耗时：0.013s  1000*1000耗时6.674s
void AddDot1x4_Register(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C);  //100*100耗时：0.0156s  1000*1000耗时：3.179s
void AddDot1x4_Register_Point(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C);  //100*100耗时：Move:0s Calculate:0.01s      400*400耗时：Move:0.0156s Calculate:0.1674s      1000*1000耗时：Move:0.0312s Calculate:1.6154s
void AddDot1x4_Register_Point_Unroll(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C);  //100*100耗时：Move:0s Calculate:0.0156s   400*400耗时：Move:0.0156s Calculate:0.1404s    1000*1000耗时：Move:0.02s  Calculate:1.5424s
void AddDot4x4_Register(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C);  //100*100耗时：0s  200*200耗时：0.0468s  400*400耗时：0.1414   500*500耗时：0.2682s
void AddDot4x4_Register_Point(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C); //100*100耗时：move:0s  Calculate:0.007s  400*400耗时：Move:0.003s Calculate:0.0966s   1000*1000耗时：Move:0.0156s Calculate:1.0572s
void AddDot4x4_SIMD(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C); //100*100耗时：move：0s Calculate:0.0156s   400*400耗时：Move:0s Calculate:0.1248s    1000*1000耗时：Move:0.032s Calculate:2.2048s
void Move_Matrix(const Matrix_2D& Size, double **A)   //矩阵转置
{
	for (int i = 0; i < Size.height; i++)
	{
		for (int j = 0; j < i; j++)
		{
			swap(A[i][j], A[j][i]);
		}
	}
}  
void addMatrix()
{
    register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, a_0p_reg;
}

void Input_Matrix(const Matrix_2D &B_2D, double **Matrix)
{
	for (int i = 0; i < B_2D.height; i++)
	{
		for (int j = 0; j < B_2D.width; j++)
		{
			Matrix[i][j] = std::rand() % 100;
			//std::cin >> Matrix[i][j];
		}
	}
}

void Test_Point(double **A)
{
	double *ptr = &A[0][0];
	std::cout << *ptr << std::endl;

	ptr += 3;
	std::cout << *ptr << std::endl;
}

void Calculate_C_Size(const Matrix_2D &A_2D, const Matrix_2D &B_2D, Matrix_2D &C_2D)
{
	C_2D.height = A_2D.height;
	C_2D.width = B_2D.width;
}

void Native_Calculate_C_Matrix(const Matrix_2D &A_2D, const Matrix_2D &B_2D, const Matrix_2D &C_2D,double **A, double **B, double **C)    //100*100耗时：0.0156s   1000*1000耗时11.0136s
{
	std::chrono::system_clock::time_point Native_cal_start = std::chrono::system_clock::now();
	for (int i = 0; i < A_2D.height; i++)
	{
		for (int j = 0; j < B_2D.width; j++)
		{
			for (int p = 0; p < B_2D.height; p++)
			{
				C[i][j] = C[i][j] + A[i][p] * B[p][j];
			}
		}
	}
	std::chrono::system_clock::time_point Native_cal_end = std::chrono::system_clock::now();
	std::cout << double(std::chrono::duration_cast<std::chrono::microseconds>(Native_cal_end - Native_cal_start).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
}

double **Init_Matrix(const Matrix_2D &MatrixSize)
{
	double **Init_Matrix = NULL;
	if (MatrixSize.height > 0 && MatrixSize.width > 0)
	{
		Init_Matrix = (double **)_aligned_malloc(sizeof(double *) * MatrixSize.height, 16);
		for (int i = 0; i < MatrixSize.height; i++)
		{
			Init_Matrix[i] = (double *)_aligned_malloc(sizeof(double) * MatrixSize.width, 16);
			memset(Init_Matrix[i], 0, sizeof(double) * MatrixSize.width);
		}
	}
	return Init_Matrix;
}

void Delete_Matrix(const Matrix_2D &MatrixSize, double **Matrix)
{
	for (int i = 0; i < MatrixSize.height; i++)
	{
		free(Matrix[i]);
	}
	free(Matrix);
}

void Output_Matrix(const Matrix_2D &MatrixSize, double **Matrix)
{
	for (int i = 0; i < MatrixSize.height; i++)
	{
		for (int j = 0; j < MatrixSize.width; j++)
		{
			std::cout << Matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	Matrix_2D A_2D, B_2D, C_2D;
	double **A, **B, **C;
	A = NULL;
	B = NULL;
	C = NULL;

	std::cin >> A_2D.height >> A_2D.width;
	std::cin >> B_2D.height >> B_2D.width;

	A = Init_Matrix(A_2D);
	B = Init_Matrix(B_2D);
	Calculate_C_Size(A_2D, B_2D, C_2D);
	C = Init_Matrix(C_2D);

	//std::cin >> A[0][0];
	std::cout << A << std::endl;
	Input_Matrix(A_2D, A);
	Input_Matrix(B_2D, B);
	//Calculate_Matrix_1x4(A_2D, B_2D, C_2D, A, B, C); 
	Calculate_Matrix_4x4(A_2D, B_2D, C_2D, A, B, C);
	//Native_Calculate_C_Matrix(A_2D, B_2D, C_2D, A, B, C);  //100*100耗时：0.0156s   1000*1000耗时11.0136s
	//Output_Matrix(C_2D, C);

	return 0;
}

void AddDot1x4(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D,  double **A, double **B, double **C)
{
	if (j + 4 <= B_2D.width)  //B矩阵未计算列数大于等于4
	{
		for (int p = 0; p < A_2D.width; p++)
	    {
		    C[i][j+0] += A[i][p] * B[p][j+0];
		    C[i][j+1] += A[i][p] * B[p][j+1];
			C[i][j+2] += A[i][p] * B[p][j+2];
			C[i][j+3] += A[i][p] * B[p][j+3];
	    }
	}
	else if (j < B_2D.width)
	{
		int num = B_2D.width - j; //B矩阵剩余列数
		switch (num)
		{
		case 1:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				C[i][j + 0] += A[i][p] * B[p][j + 0];
			}
			break;
		}
		case 2:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				C[i][j + 0] += A[i][p] * B[p][j + 0];
				C[i][j + 1] += A[i][p] * B[p][j + 1];
			}
			break;
		}
		case 3:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				C[i][j + 0] += A[i][p] * B[p][j + 0];
				C[i][j + 1] += A[i][p] * B[p][j + 1];
				C[i][j + 2] += A[i][p] * B[p][j + 2];
			}
			break;
		}
		default:
			break;
		}
	}
	
}

void Calculate_Matrix_1x4(const Matrix_2D& A_2D, const Matrix_2D& B_2D, const Matrix_2D& C_2D,  double **A,double **B, double **C)
{
	std::chrono::system_clock::time_point Cal_1x4_start = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point Move_start = std::chrono::system_clock::now();
	Move_Matrix(B_2D, B);
	std::chrono::system_clock::time_point Move_end = std::chrono::system_clock::now();
	std::cout << "Move:" << double(std::chrono::duration_cast<std::chrono::microseconds>(Move_end - Move_start).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
	for (int j = 0; j < C_2D.width; j += 4)
	{
		for (int i = 0; i < A_2D.height; i++)
		{
			AddDot1x4_Register_Point_Unroll(j, i, A_2D, B_2D, A, B, C);
		}
	}
	std::chrono::system_clock::time_point Cal_1x4_end = std::chrono::system_clock::now();
	std::cout << double(std::chrono::duration_cast<std::chrono::microseconds>(Cal_1x4_end - Cal_1x4_start).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
}

void AddDot1x4_Register(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, a_0p_reg;
	c_00_reg = 0.0;
	c_01_reg = 0.0;
	c_02_reg = 0.0;
	c_03_reg = 0.0;

	if (j + 4 <= B_2D.width) //B矩阵剩余列数是否大于四
	{
		for (int p = 0; p < A_2D.width; p++)
		{
			a_0p_reg = A[i][p];
			c_00_reg += a_0p_reg * B[p][j + 0];
			c_01_reg += a_0p_reg * B[p][j + 1];
			c_02_reg += a_0p_reg * B[p][j + 2];
			c_03_reg += a_0p_reg * B[p][j + 3];
		}
	}
	else if (j < B_2D.width)
	{
		int num = B_2D.width - j;   //计算B矩阵剩余列数
		switch (num)
		{
		case 0:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * B[p][j + 0];
			}
			break;
		}
		case 1:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * B[p][j + 0];
				c_01_reg += a_0p_reg * B[p][j + 1];
			}
			break;
		}
		case 2:
		{
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * B[p][j + 0];
				c_01_reg += a_0p_reg * B[p][j + 1];
				c_02_reg += a_0p_reg * B[p][j + 2];
			}
			break;
		}
		default:
			break;
		}
	}
}

void AddDot1x4_Register_Point(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, a_0p_reg;
	c_00_reg = 0.0;
	c_01_reg = 0.0;
	c_02_reg = 0.0;
	c_03_reg = 0.0;

	int diff = B_2D.width;   //指向B数组列的指针每次偏移量
	if (j + 4 <= B_2D.width)
	{
		double *bp0_ptr = &B[j + 0][0];
		double *bp1_ptr = &B[j + 1][0];
		double *bp2_ptr = &B[j + 2][0];
		double *bp3_ptr = &B[j + 3][0];
		for (int p = 0; p < A_2D.width; p++)
		{
			a_0p_reg = A[i][p];
			c_00_reg += a_0p_reg * *bp0_ptr++;
			c_01_reg += a_0p_reg * *bp1_ptr++;
			c_02_reg += a_0p_reg * *bp2_ptr++;
			c_03_reg += a_0p_reg * *bp3_ptr++;
		}
	}
	else if (j < B_2D.width)
	{
		int num = B_2D.width - j;
		switch (num)
		{
		case 0:
		{
			double *bp0_ptr = &B[0][j + 0];
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * *bp0_ptr;
				bp0_ptr += diff;
			}
			break;
		}
		case 1:
		{
			double *bp0_ptr = &B[0][j + 0];
			double *bp1_ptr = &B[0][j + 1];
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * *bp0_ptr;
				c_01_reg += a_0p_reg * *bp1_ptr;
			}
			break;
		}
		case 2:
		{
			double *bp0_ptr = &B[0][j + 0];
			double *bp1_ptr = &B[0][j + 1];
			double *bp2_ptr = &B[0][j + 2];
			for (int p = 0; p < A_2D.width; p++)
			{
				a_0p_reg = A[i][p];
				c_00_reg += a_0p_reg * *bp0_ptr;
				c_01_reg += a_0p_reg * *bp1_ptr;
				c_02_reg += a_0p_reg * *bp2_ptr;
				bp0_ptr += diff;
				bp1_ptr += diff;
				bp2_ptr += diff;
			}
			break;
		}
		default:
			break;
		}
	}
}

void AddDot1x4_Register_Point_Unroll(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, a_0p_reg;
	c_00_reg = 0.0;
	c_01_reg = 0.0;
	c_02_reg = 0.0;
	c_03_reg = 0.0;

	int diff = B_2D.width;
	if (j + 4 <= B_2D.width)
	{
		double *bp0_ptr = &B[j + 0][0];
		double *bp1_ptr = &B[j + 1][0];
		double *bp2_ptr = &B[j + 2][0];
		double *bp3_ptr = &B[j + 3][0];
		for (int p = 0; p < A_2D.width; p+=4)
		{
			a_0p_reg = A[i][p+ 0];
			c_00_reg += a_0p_reg * *(bp0_ptr);
			c_01_reg += a_0p_reg * *(bp1_ptr);
			c_02_reg += a_0p_reg * *(bp2_ptr);
			c_03_reg += a_0p_reg * *(bp3_ptr);

			a_0p_reg = A[i][p + 1];
			c_00_reg += a_0p_reg * *(++bp0_ptr);
			c_01_reg += a_0p_reg * *(++bp1_ptr);
			c_02_reg += a_0p_reg * *(++bp2_ptr);
			c_03_reg += a_0p_reg * *(++bp3_ptr);

			a_0p_reg = A[i][p + 2];
			c_00_reg += a_0p_reg * *(++bp0_ptr);
			c_01_reg += a_0p_reg * *(++bp1_ptr);
			c_02_reg += a_0p_reg * *(++bp2_ptr);
			c_03_reg += a_0p_reg * *(++bp3_ptr);

			a_0p_reg = A[i][p + 3];
			c_00_reg += a_0p_reg * *(++bp0_ptr);
			c_01_reg += a_0p_reg * *(++bp1_ptr);
			c_02_reg += a_0p_reg * *(++bp2_ptr);
			c_03_reg += a_0p_reg * *(++bp3_ptr);

			bp0_ptr += 1;
			bp1_ptr += 1;
			bp2_ptr += 1;
			bp3_ptr += 1;
		}
	}
}

void AddDot4x4_Register(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, c_10_reg, c_11_reg, c_12_reg, c_13_reg, c_20_reg, c_21_reg, c_22_reg, c_23_reg, c_30_reg, c_31_reg, c_32_reg, c_33_reg;
	register double a_0p_reg, a_2p_reg, a_3p_reg, a_1p_reg;
	c_00_reg = 0.0, c_01_reg = 0.0, c_02_reg = 0.0, c_03_reg = 0.0;
	c_10_reg = 0.0, c_11_reg = 0.0, c_12_reg = 0.0, c_13_reg = 0.0;
	c_20_reg = 0.0, c_21_reg = 0.0, c_22_reg = 0.0, c_23_reg = 0.0;
	c_30_reg = 0.0, c_31_reg = 0.0, c_32_reg = 0.0, c_33_reg = 0.0;
	if (j + 4 <= B_2D.width && i + 4 <= A_2D.height)
	{
		for (int p = 0; p < A_2D.width; p ++)
		{
			a_0p_reg = A[i + 0][p];
			a_1p_reg = A[i + 1][p];
			a_2p_reg = A[i + 2][p];
			a_3p_reg = A[i + 3][p];

			c_00_reg += a_0p_reg * B[p][j + 0];
			c_01_reg += a_0p_reg * B[p][j + 1];
			c_02_reg += a_0p_reg * B[p][j + 2];
			c_03_reg += a_0p_reg * B[p][j + 3];

			c_10_reg += a_1p_reg * B[p][j + 0];
			c_11_reg += a_1p_reg * B[p][j + 1];
			c_12_reg += a_1p_reg * B[p][j + 2];
			c_13_reg += a_1p_reg * B[p][j + 3];

			c_20_reg += a_2p_reg * B[p][j + 0];
			c_21_reg += a_2p_reg * B[p][j + 1];
			c_22_reg += a_2p_reg * B[p][j + 2];
			c_23_reg += a_2p_reg * B[p][j + 3];

			c_30_reg += a_3p_reg * B[p][j + 0];
			c_31_reg += a_3p_reg * B[p][j + 1];
			c_32_reg += a_3p_reg * B[p][j + 2];
			c_33_reg += a_3p_reg * B[p][j + 3];
		}
	}
}

void Calculate_Matrix_4x4(const Matrix_2D& A_2D, const Matrix_2D& B_2D, const Matrix_2D& C_2D, double **A, double **B, double **C)
{
	std::chrono::system_clock::time_point Cal_4x4_start = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point Move_start = std::chrono::system_clock::now();
	Move_Matrix(B_2D, B);
	Move_Matrix(A_2D, A);
	std::chrono::system_clock::time_point Move_end = std::chrono::system_clock::now();
	std::cout << "Move:" << double(std::chrono::duration_cast<std::chrono::microseconds>(Move_end - Move_start).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
	for (int j = 0; j < C_2D.width; j += 4)
	{
		for (int i = 0; i < A_2D.height; i += 4)
		{
			AddDot4x4_SIMD(j, i, A_2D, B_2D, A, B, C);
		}
	}
	std::chrono::system_clock::time_point Cal_4x4_end = std::chrono::system_clock::now();
	std::cout << double(std::chrono::duration_cast<std::chrono::microseconds>(Cal_4x4_end - Cal_4x4_start).count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒" << std::endl;
}

void AddDot4x4_Register_Point(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	register double c_00_reg, c_01_reg, c_02_reg, c_03_reg, c_10_reg, c_11_reg, c_12_reg, c_13_reg, c_20_reg, c_21_reg, c_22_reg, c_23_reg, c_30_reg, c_31_reg, c_32_reg, c_33_reg;
	register double a_0p_reg, a_2p_reg, a_3p_reg, a_1p_reg;
	c_00_reg = 0.0, c_01_reg = 0.0, c_02_reg = 0.0, c_03_reg = 0.0;
	c_10_reg = 0.0, c_11_reg = 0.0, c_12_reg = 0.0, c_13_reg = 0.0;
	c_20_reg = 0.0, c_21_reg = 0.0, c_22_reg = 0.0, c_23_reg = 0.0;
	c_30_reg = 0.0, c_31_reg = 0.0, c_32_reg = 0.0, c_33_reg = 0.0;
	if (j + 4 <= B_2D.width && i + 4 <= A_2D.height)
	{
		double *bp0_ptr = &B[j + 0][0];
		double *bp1_ptr = &B[j + 1][0];
		double *bp2_ptr = &B[j + 2][0];
		double *bp3_ptr = &B[j + 3][0];
		for (int p = 0; p < A_2D.width; p++)
		{
			a_0p_reg = A[i + 0][p];
			a_1p_reg = A[i + 1][p];
			a_2p_reg = A[i + 2][p];
			a_3p_reg = A[i + 3][p];

			c_00_reg += a_0p_reg * *bp0_ptr;
			c_01_reg += a_0p_reg * *bp1_ptr;
			c_02_reg += a_0p_reg * *bp2_ptr;
			c_03_reg += a_0p_reg * *bp3_ptr;

			c_10_reg += a_1p_reg * *bp0_ptr;
			c_11_reg += a_1p_reg * *bp1_ptr;
			c_12_reg += a_1p_reg * *bp2_ptr;
			c_13_reg += a_1p_reg * *bp3_ptr;

			c_20_reg += a_2p_reg * *bp0_ptr;
			c_21_reg += a_2p_reg * *bp1_ptr;
			c_22_reg += a_2p_reg * *bp2_ptr;
			c_23_reg += a_2p_reg * *bp3_ptr;

			c_30_reg += a_3p_reg * *bp0_ptr++;
			c_31_reg += a_3p_reg * *bp1_ptr++;
			c_32_reg += a_3p_reg * *bp2_ptr++;
			c_33_reg += a_3p_reg * *bp3_ptr++;
		}
	}
}

void AddDot4x4_SIMD(const int j, const int i, const Matrix_2D& A_2D, const Matrix_2D& B_2D, double **A, double **B, double **C)
{
	__m128d a_0p_a_1p_verg, a_2p_a_3p_verg,
	 b_p0_verg, b_p1_verg, b_p2_verg, b_p3_verg,
	 c_00_c_10_verg, c_01_c_11_verg, c_02_c_12_verg, c_03_c_13_verg,
	 c_20_c_30_verg, c_21_c_31_verg, c_22_c_32_verg, c_23_c_33_verg;
	double  Zero =  0.0;
	double *Init;
	Init = &Zero;

	c_00_c_10_verg = _mm_loaddup_pd(Init);
	c_01_c_11_verg = _mm_loaddup_pd(Init);
	c_02_c_12_verg = _mm_loaddup_pd(Init);
	c_03_c_13_verg = _mm_loaddup_pd(Init);

	c_20_c_30_verg = _mm_loaddup_pd(Init);
	c_21_c_31_verg = _mm_loaddup_pd(Init);
	c_22_c_32_verg = _mm_loaddup_pd(Init);
	c_23_c_33_verg = _mm_loaddup_pd(Init);

	if (j + 4 <= B_2D.width && i + 4 <= A_2D.height)
	{
		double *bp0_ptr = &B[j + 0][0];
		double *bp1_ptr = &B[j + 1][0];
		double *bp2_ptr = &B[j + 2][0];
		double *bp3_ptr = &B[j + 3][0];
		for (int p = 0; p < A_2D.width; p++)
		{
			a_0p_a_1p_verg = _mm_load_pd(&A[p][i]);
			a_2p_a_3p_verg = _mm_load_pd((double *)&A[p][i + 2]);
		  
			b_p0_verg = _mm_loaddup_pd((double *)bp0_ptr++);
			b_p1_verg = _mm_loaddup_pd((double *)bp1_ptr++);
			b_p2_verg = _mm_loaddup_pd((double *)bp2_ptr++);
			b_p3_verg = _mm_loaddup_pd((double *)bp3_ptr++);

			c_00_c_10_verg = _mm_add_pd(c_00_c_10_verg, _mm_mul_pd(a_0p_a_1p_verg, b_p0_verg));
			c_01_c_11_verg = _mm_add_pd(c_01_c_11_verg, _mm_mul_pd(a_0p_a_1p_verg, b_p1_verg));
			c_02_c_12_verg = _mm_add_pd(c_02_c_12_verg, _mm_mul_pd(a_0p_a_1p_verg, b_p2_verg));
			c_03_c_13_verg = _mm_add_pd(c_03_c_13_verg, _mm_mul_pd(a_0p_a_1p_verg, b_p3_verg));

			c_20_c_30_verg = _mm_add_pd(c_20_c_30_verg, _mm_mul_pd(a_2p_a_3p_verg, b_p0_verg));
			c_21_c_31_verg = _mm_add_pd(c_21_c_31_verg, _mm_mul_pd(a_2p_a_3p_verg, b_p1_verg));
			c_22_c_32_verg = _mm_add_pd(c_22_c_32_verg, _mm_mul_pd(a_2p_a_3p_verg, b_p2_verg));
			c_23_c_33_verg = _mm_add_pd(c_23_c_33_verg, _mm_mul_pd(a_2p_a_3p_verg, b_p3_verg));
		}
	}
}