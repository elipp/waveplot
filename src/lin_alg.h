#ifndef LIN_ALG_H
#define LIN_ALG_H

#include <iostream>
#include <iomanip>
#include <cstring>

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

	mat4(float *data);
	mat4() { memset(this->data, 0, sizeof(this->data)); }

	void identity();	// "in place identity"

	float *rawdata();	// returns a column-major float[16]

	void print();

};


vec4 ftransform_test(vec4 &);

#endif
