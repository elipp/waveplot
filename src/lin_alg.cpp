#include "lin_alg.h"

vec4::vec4(float _x, float _y, float _z, float _w) {

#ifdef _WIN32
	// must use _mm_set_ps for this function
	data = _mm_set_ps(_x, _y, _z, _w);
#elif __linux__

	data[0] = _x;
	data[1] = _y;
	data[2] = _z;
	data[3] = _w;
#endif
}


vec4::vec4() {
#ifdef _WIN32
	data = _mm_setzero_ps();
#elif __linux__
	memset(data, 0, sizeof(data)); 
#endif

}

// obviously enough, this requires a 4-float array as argument
vec4::vec4(const float* const a) {
#ifdef _WIN32
	data = _mm_loadu_ps(a);		// not assuming 16-byte alignment for a.
#elif __linux__
	memcpy(data, a, sizeof(data));
#endif

}

void vec4::operator*=(const float& scalar) {

#ifdef _WIN32
	// use xmmintrin
	__m128 s = _mm_load1_ps(&scalar);	// set the whole register to value scalar
	data = _mm_mul_ps(data, s);

#elif __linux__

	vec4 &a = (*this);
	a(0) *= scalar;
	a(1) *= scalar;
	a(2) *= scalar;
	a(3) *= scalar;

#endif


}

vec4 vec4::operator+(const vec4 &b) {
#ifdef _WIN32

	return vec4(_mm_add_ps(data, b.data));
	
#elif __linux__

	return vec4(data[0]+b.data[0],data[1]+b.data[1],data[2]+b.data[2],data[3]+b.data[3]);

#endif
}

void vec4::print(){

#ifdef _WIN32
	float data_arr[4];
	_mm_storeu_ps(data_arr, data);	// storeu for now
	printf("%f %f %f %f\n", data_arr[0], data_arr[1], data_arr[2], data_arr[3]);
#elif __linux__
	std::cout.precision(4);
	std::cout << std::fixed << this->data[0]<< ", " <<  this->data[1]<< ", " <<  this->data[2]<< ", " <<  this->data[3] << "\n";
#endif
}

void mat4::print() {

	mat4 &mat = (*this);

	std::cout.precision(4);
	for (int i = 0; i < 4; i++) {

		std::cout << std::fixed << mat(0, i) << " " << mat(1, i) << " " << mat(2, i) << " " << mat(3, i) << "\n";

	}


}


float& vec4::operator() (const int & row) {	

#ifdef _WIN32
	// TODO: look for a better solution :D
	float a[4];
	_mm_storeu_ps(a, data);	// unaligned just in case (storeu)
	return a[row];
#elif __linux__
	return data[row];
#endif


}


mat4::mat4(const float *data) {

	memcpy(this->data, data, (4*4)*sizeof(float));

}

mat4::mat4(const int main_diagonal_val) {

	mat4 &a = (*this);
	memset(a.data, 0, sizeof(a.data));

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

vec4 mat4::row(const int &i) {

	return vec4(data[0][i], data[1][i], data[2][i], data[3][i]);

}

vec4 mat4::column(const int &i) {

	return vec4(data[i][0], data[i][1],data[i][2],data[i][3]);

}


const float *mat4::rawdata() const {

	return &data[0][0];

}

void mat4::printRaw() const {

	const float * const ptr = &this->data[0][0];
		
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

	// We could just assume here that the matrix is "clean",
	// i.e. that any matrix elements other than the ones used in
	// a pure orthographic projection matrix are zero.

	// also, the subscript operators could be used here.
	
	// FOR DEBUG, and just to be on the safe side:
	this->identity();

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

	__m128	r1, r2, r3, r4;
	 //let's still keep the memcpy version just for comparison
	/*
	memcpy(&r1, &(data[0][0]), 4*sizeof(float));
	memcpy(&r2, &(data[1][0]), 4*sizeof(float));
	memcpy(&r3, &(data[2][0]), 4*sizeof(float));
	memcpy(&r4, &(data[3][0]), 4*sizeof(float));
	*/
	
	r1 = _mm_set_ps(data[0][0], data[0][1], data[0][2], data[0][3]);
	r2 = _mm_set_ps(data[1][0], data[1][1], data[1][2], data[1][3]);
	r3 = _mm_set_ps(data[2][0], data[2][1], data[2][2], data[2][3]);
	r4 = _mm_set_ps(data[3][0], data[3][1], data[3][2], data[3][3]);
	
	_MM_TRANSPOSE4_PS(r1, r2, r3, r4);	// microsoft special transpose macro :P

	// as of now, the mat4 (and vec4) structures are
	// 16-byte aligned, so _mm_store_ps can be used
	// safely instead of the less efficient _mm_storeu_ps
	// intrinsics

	_mm_store_ps(&(data[0][0]), r1);
	_mm_store_ps(&(data[1][0]), r2);
	_mm_store_ps(&(data[2][0]), r3);
	_mm_store_ps(&(data[3][0]), r4);
	
	


#elif __linux__

	mat4 &m = (*this);
	
	float tmp;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp = m(i, j);
			m(i, j) = m(j, i);
			m(j, i) = tmp;
		}
	}

#endif
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
