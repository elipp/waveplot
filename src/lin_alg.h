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

struct vec4 {	// for debugging purposes 

	float data[4];
	vec4(float _x, float _y, float _z, float _w);
	vec4() {}
	float& operator() (unsigned short row);

	void print();

};

struct mat4 {	// column major :D


	float data[4][4];
	float& operator() (unsigned short column, unsigned short row);	// reference & non-const -> modifiable by subscript

	mat4 operator* (mat4 &R);
	vec4 operator* (vec4 &R);

	mat4(const float *data);
	mat4() { memset(this->data, 0, sizeof(this->data)); }
	mat4(const int main_diagonal_val);

	void identity();	// "in place identity"
	void T();			// "in place transpose"

	const float *rawdata() const;	// returns a column-major float[16]
	void printRaw();	// prints elements in actual memory order.

	void print();
	void make_proj_orthographic(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	void make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar);
	
};

vec4 ftransform_test(vec4 &);


#endif
