// SIMD_Test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <xmmintrin.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>



int _tmain(int argc, _TCHAR* argv[])
{
	__m128d c_00;
	double Init = 0.0;
	double *test;
	test = &Init;
	c_00 = _mm_load_pd(test);
	std::cout << "Ok" << std::endl;

	return 0;
}

