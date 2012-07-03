#include "lin_alg.h"

vec4::vec4(float _x, float _y, float _z, float _w) {

	data[0] = _x;
	data[1] = _y;
	data[2] = _z;
	data[3] = _w;

}

void vec4::print(){

	std::cout.precision(4);

	std::cout << std::fixed << this->data[0]<< ", " <<  this->data[1]<< ", " <<  this->data[2]<< ", " <<  this->data[3] << "\n";
}

void mat4::print() {

	mat4 &mat = (*this);

	std::cout.precision(4);
	for (int i = 0; i < 4; i++) {

		std::cout << std::fixed << mat(0, i) << " " << mat(1, i) << " " << mat(2, i) << " " << mat(3, i) << "\n";

	}


}


float& vec4::operator() (unsigned short row) {	

	return data[row];

}


mat4::mat4(float *data) {

	memcpy(this->data, data, (4*4)*sizeof(float));

}


mat4 mat4::operator* (mat4& R) {

	mat4 ret;
	mat4 &L = (*this);	// for easier syntax

//	ret(0, 0) = l(0, 0)*r(0, 0) + l(1,0)*l(0,1) + l(2,0)*r(0,2) + l(3,0)*r(0,3);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret(i,j) = L(0, j)*R(i, 0) 
				 + L(1, j)*R(i, 1) 
				 + L(2, j)*R(i, 2) 
				 + L(3, j)*R(i, 3);

	return ret;

}

vec4 mat4::operator* (vec4& R) {

	vec4 ret;
	mat4 &L = (*this);

	for (int i = 0; i < 4; i++)
		ret(i) = L(0, i)*R(0)
		       + L(1, i)*R(1)
		       + L(2, i)*R(2)
		       + L(3, i)*R(3);

	return ret;
}


float& mat4::operator() ( unsigned short column, unsigned short row ) {

	// no bounds checking! 
	return data[column][row];
}

void mat4::identity() {

	memset(this->data, 0, sizeof(this->data));

	this->data[0][0] = 1.0;
	this->data[1][1] = 1.0;
	this->data[2][2] = 1.0;
	this->data[3][3] = 1.0;

}

float *mat4::rawdata() {

	return &data[0][0];

}

vec4 ftransform_test(vec4 &vec) {

	float dataholder[16];
	glGetFloatv(GL_PROJECTION_MATRIX, dataholder);
	mat4 projection(dataholder);

	glGetFloatv(GL_MODELVIEW_MATRIX, dataholder);
	mat4 modelview(dataholder);

	vec4 ret = (projection*modelview)*vec;
	return ret;

}
