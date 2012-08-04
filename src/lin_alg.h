#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <cstring>
#include <cmath>
#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <stdio.h>
#include <intrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#endif

enum { MAT_ZERO = 0x0, MAT_IDENTITY = 0x1 };

const char* checkCPUCapabilities();

#ifdef _WIN32
__declspec(align(16)) // to ensure 16-byte alignment in memory
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
	float elementAt(const int& row) const;	

	// see also: vec4 operator*(const float& scalar, vec4& v);

	void operator*=(const float& scalar);
	vec4 operator*(const float& scalar);
	void operator+=(const vec4& b);
	vec4 operator+(const vec4& b);

	float length3() const;
	float length4() const;
	void normalize();

	void print();

};

vec4 operator*(const float& scalar, vec4& v);	// convenience overload :P

// NOTE: the dot function doesn't perform an actual dot product computation of two R^4 vectors,
// as the type of the arguments misleadingly suggests. Instead it computes a truncated dot product,
// including only the first 3 components (i.e. x,y,z)

float dot(const vec4 &a, const vec4 &b);
// dot benchmarks for 100000000 iterations:
// naive:			20.9s
// DPPS:			2.9s
// MULPS:			3.0s

vec4 cross(const vec4 &a,  const vec4 &b);	// not really vec4, since cross product for such vectors isn't defined

#ifdef _WIN32
__declspec(align(64))
struct mat4 {	// column major 
	__m128 data[4];	// each holds a column vector
#elif __linux__
struct mat4 {
	float data[4][4];
#endif
	float& operator() (const int &column, const int &row);	// reference & non-const -> modifiable by subscript
	float elementAt(const int &column, const int &row) const;
	mat4 operator* (const mat4 &R);	// the other matrix needs to be transposed, hence no const specifier
	vec4 operator* (const vec4 &R);
	
	vec4 row(const int& i);	// these two should be const, but optimization is a higher priority :D
	vec4 column(const int& i);

	void assignToRow(const int& row, const vec4& v);
	void assignToColumn(const int& column, const vec4& v);

	mat4(const float *data);
	mat4();
	mat4(const int main_diagonal_val);

	void zero();
	void identity();	// "in place identity"
	void T();			// "in place transpose"
	
	// T(): benchmarking results for 100000000 iterations:
	//
	// SSE (microsoft macro _MM_TRANSPOSE4_PS):
	//		memcpy/_mm_store_ps:		8.3 s.
	//		_mm_set_ps/_mm_store_ps:	5.6 s.
	//		_mm_set_ps/_mm_storeu_ps:	5.6 s.	identical (?)
	//		_mm_loadu_ps/_mm_store_ps:	4.8 s.	Nice!
	//	
	// NEW bare edition:				2.2 s. :P
	//
	// non-SSE:
	//		the elif linux version:		150.2 s. ! :D


	void *rawdata() const;	// returns a column-major float[16]
	void printRaw() const;	// prints elements in actual memory order.

	void print();
	void make_proj_orthographic(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	void make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	
};

vec4 ftransform_test(vec4 &);


#endif
