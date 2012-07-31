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


mat4::mat4(const float *data) {

	memcpy(this->data, data, (4*4)*sizeof(float));

}

mat4::mat4(const int main_diagonal_val) {

	mat4 &a = (*this);
	memset(a.data, 0, 16);

	a(0,0)=main_diagonal_val;
	a(1,1)=main_diagonal_val;
	a(2,2)=main_diagonal_val;
	a(3,3)=main_diagonal_val;
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

const float *mat4::rawdata() const {

	return &data[0][0];

}



void mat4::printRaw() {

	
	float * const ptr = &this->data[0][0];
	
	
#ifdef _WIN32
	static const char* fmt = "%4.3f %4.3f %4.3f %4.3f\n";

	for (int i = 0; i < 16; i += 4)
		printf(fmt, *(ptr + i), *(ptr + i + 1), *(ptr + i + 2), *(ptr + i + 3));
	printf ("\n");

#elif __linux__
	for (int i = 0; i < 16, i += 4)
		std::cout << std::setprecision(3) << *(ptr + i) << " " << *(ptr + i + 1) << " " << *(ptr + i + 2) << " " << *(ptr + i + 3) << "\n";
	std::cout << "\n";
#endif

	
}


void mat4::make_proj_orthographic(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar) {

	// any previous data is simply discarded.
	// We could just assume here that the matrix is "clean",
	// i.e. that any matrix elements other than the ones used in
	// a pure orthographic projection matrix are zero.

	// also, the subscript operators could be used here.
	
	this->data[0][0] = 2.0/(right - left);
	this->data[1][1] = 2.0/(top - bottom);
	this->data[2][2] = -2.0/(zFar - zNear);

	// this can be simplified further, if the viewing volume is symmetric,
	// i.e. right - left == 0 && top-bottom == 0 && zFar-zNear == 0.
	// But it isn't. =)

	this->data[3][0] = - (right + left) / (right - left);
	this->data[3][1] = - (top + bottom) / (top - bottom);
	this->data[3][2] = - (zFar + zNear) / (zFar - zNear);

	this->data[3][3] = 1.0;	// Regardless of how the projection works, opengl itself seems to
	// be settings this element to 1.0 (tested with glOrtho(...) -> glGetFloatv(GL_PROJECTION_MATRIX) -> print)
}

void mat4::make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar) {
		
	// STUB(B)! (probably never going to be used)
	// nop
	return;

}

// performs an "in-place transposition" of the matrix

void mat4::T() {

#ifdef _WIN32

	__m128 r1, r2, r3, r4;

#endif

	mat4 &m = (*this);
	
	float tmp;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp = m(i, j);
			m(i, j) = m(j, i);
			m(j, i) = tmp;
		}
	}

}



//ftransform_test. NOTE: Requires a valid OpenGL context.

/*vec4 ftransform_test(vec4 &vec) {

	float dataholder[16];
	glGetFloatv(GL_PROJECTION_MATRIX, dataholder);
	mat4 projection(dataholder);

	glGetFloatv(GL_MODELVIEW_MATRIX, dataholder);
	mat4 modelview(dataholder);

	vec4 ret = (projection*modelview)*vec;
	return ret;

}*/
