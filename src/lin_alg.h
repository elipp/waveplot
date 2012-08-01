#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <cstring>
#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <stdio.h>
#include <xmmintrin.h>
#endif

enum { MAT_ZERO = 0x0, MAT_IDENTITY = 0x1 };

#ifdef _WIN32
__declspec(align(16)) // this of course refers to the following struct
struct vec4 {		
	__m128 data;
	vec4(const __m128 &a) : data(a) {} ;
#elif __linux__
struct vec4 {
	float data[4];
#endif
	vec4();	
	vec4(float _x, float _y, float _z, float _w);	
	vec4(const float * const a);
	float& operator() (const int& row);
	void operator*=(const float& scalar);
	vec4 operator+(const vec4& b);

	void print();

};

#ifdef _WIN32
__declspec(align(64))
#endif
struct mat4 {	// column major :D


	float data[4][4];
	float& operator() (unsigned short column, unsigned short row);	// reference & non-const -> modifiable by subscript

	mat4 operator* (mat4 &R);
	vec4 operator* (vec4 &R);
	
	vec4 row(const int& i);
	vec4 column(const int& i);

	mat4(const float *data);
	mat4() { memset(this->data, 0, sizeof(this->data)); }
	mat4(const int main_diagonal_val);

	void identity();	// "in place identity"
	void T();			// "in place transpose"
	
	// benchmarking results for 100000000 transpositions on the same matrix
	// SSE (_MM_TRANSPOSE4_PS):
	//		memcpy/_mm_store_ps:		8.3 s.
	//		_mm_set_ps/_mm_store_ps:	5.6 s.
	//		_mm_set_ps/_mm_storeu_ps:	5.6 s.	identical (?)
	//
	// non-SSE:
	//		the elif linux version:		150.2 s. ! :D


	const float *rawdata() const;	// returns a column-major float[16]
	void printRaw() const;	// prints elements in actual memory order.

	void print();
	void make_proj_orthographic(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	void make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	
};

vec4 ftransform_test(vec4 &);


#endif
