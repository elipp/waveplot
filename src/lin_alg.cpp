#include "lin_alg.h"

#ifdef _WIN32
static const __m128 ZERO = _mm_setzero_ps();
static const __m128 ZERO_BUT_W1 = _mm_set_ps(1.0, 0.0, 0.0, 0.0);
static const __m128 QUAT_CONJUGATE = _mm_set_ps(1.0, -1.0, -1.0, -1.0);	// in reverse order!
static const __m128 QUAT_NO_ROTATION = _mm_set_ps(1.0, 0.0, 0.0, 0.0);
static const int mask3021 = 0xC9, // 11 00 10 01_2
				 mask3102 = 0xD2, // 11 01 00 10_2
				 xyz_dot_mask = 0x71,
				 xyzw_dot_mask = 0xF1;
#endif

const char* checkCPUCapabilities() {
#ifdef _WIN32
	int a[4];
	__cpuid(a, 1);

	// SSE
	if (!(a[3] & (0x1 << 25))) {
		return "SSE not supported by host processor!";
	}
	// SSE2
	if (!(a[3] & (0x1 << 26))) {
		return "SSE2 not supported by host processor!"; 
	}
	
	if (!(a[2] & (0x1 << 19))) {
		return "SSE4.1 not supported by host processor!";
	}
	
	return "OK";

#elif __linux__

	// nyi

#endif
}

vec4::vec4(float _x, float _y, float _z, float _w) {

#ifdef _WIN32
	// must use _mm_setr_ps for this constructor. Note 'r' for reversed.
	//data = _mm_setr_ps(_x, _y, _z, _w);
	data = _mm_set_ps(_w, _z, _y, _x);	// could be faster, haven't tested
#elif __linux__

	data[0] = _x;
	data[1] = _y;
	data[2] = _z;
	data[3] = _w;
#endif
}


vec4::vec4() {
#ifdef _WIN32
	data = ZERO;
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
	data = _mm_mul_ps(data, _mm_load1_ps(&scalar));

#elif __linux__

	vec4 &a = (*this);
	a(0) *= scalar;
	a(1) *= scalar;
	a(2) *= scalar;
	a(3) *= scalar;

#endif


}

vec4 operator*(const float& scalar, vec4& v) {

	vec4 r(v.data);
	r *= scalar;
	return r;

}	

vec4 vec4::operator*(const float& scalar) {

	vec4 v(this->data);
	v *= scalar;
	return v;

}

void vec4::operator+=(const vec4 &b) {
	
	this->data=_mm_add_ps(data, b.data);

}

vec4 vec4::operator+(const vec4 &b) {
#ifdef _WIN32

	vec4 v = (*this);
	v += b; 
	return v;
	
#elif __linux__

	return vec4(data[0]+b.data[0],data[1]+b.data[1],data[2]+b.data[2],data[3]+b.data[3]);

#endif
}

float vec4::length3() const {
#ifdef _WIN32

	return sqrt(_mm_dp_ps(this->data, this->data, 0x71).m128_f32[0]);

#elif __linux__
	const vec4 &v = (*this);
	return sqrt(v.data[0]*v.data[0] + v.data[1]*v.data[1] + v.data[2]*v.data[2]);

#endif

}

float vec4::length4() const {

#ifdef _WIN32

	return sqrt(_mm_dp_ps(this->data, this->data, 0xF1).m128_f32[0]);	// includes x,y,z,w in the computation

#elif __linux__
	const vec4 &v = (*this);
	return sqrt(v.data[0]*v.data[0] + v.data[1]*v.data[1] + v.data[2]*v.data[2] + v.data[3]*v.data[3]);

#endif

}

// this should actually include all components, but given the application, this won't :P

void vec4::normalize() {
#ifdef _WIN32
	const float l_recip = 1.0/sqrt(_mm_dp_ps(this->data, this->data, xyz_dot_mask).m128_f32[0]); // only x,y,z components
	this->data = _mm_mul_ps(this->data, _mm_set1_ps(l_recip));
#elif __linux__

	const float l_recip = 1.0/sqrt(length3());

	this->data[0] *= l_recip;
	this->data[1] *= l_recip;
	this->data[2] *= l_recip;

#endif

}

void vec4::print(){

#ifdef _WIN32
	printf("(%4.3f, %4.3f, %4.3f, %4.3f)\n", data.m128_f32[0], data.m128_f32[1], data.m128_f32[2], data.m128_f32[3]);
#elif __linux__
	std::cout.precision(4);
	std::cout << std::fixed << this->data[0]<< ", " <<  this->data[1]<< ", " <<  this->data[2]<< ", " <<  this->data[3] << "\n";
#endif
}

void mat4::print() {
#ifdef _WIN32
	mat4 &M = (*this);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			printf("%4.3f ", M.elementAt(j,i));
		printf("\n");
	}
	printf("\n");
#elif __linux__
	mat4 &M = (*this);

	std::cout.precision(4);
	for (int i = 0; i < 4; i++) {
		std::cout << std::fixed << M(0, i) << " " << M(1, i) << " " << M(2, i) << " " << M(3, i) << "\n";
	}

#endif
}


float dot(const vec4 &a, const vec4 &b) {
#ifdef _WIN32

	__m128 dot = _mm_dp_ps(a.data, b.data, xyz_dot_mask);	// direct computation of dot product (SSE4)
	return dot.m128_f32[0];	 
	
	/*// yet another solution	
	__m128 mul = _mm_mul_ps(a.data, b.data);
	return mul.m128_f32[0]+mul.m128_f32[1]+mul.m128_f32[2]+mul.m128_f32[3];*/

#elif __linux__

	return 0;
	//return a.data[0]*b.data[0] + a.data[1]*b.data[1] + a.data[2]*b.data[2] +a.data[3]*b.data[3];
	// NAIVE: return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
#endif

}

vec4 cross(const vec4 &a, const vec4 &b) {

	// See: http://fastcpp.blogspot.fi/2011/04/vector-cross-product-using-sse-code.html.
	// Absolutely beautiful! (the exact same recipe can be found at
	// http://neilkemp.us/src/sse_tutorial/sse_tutorial.html#E, albeit in assembly.)
	
#ifdef _WIN32
		
	return vec4(
	_mm_sub_ps(
    _mm_mul_ps(_mm_shuffle_ps(a.data, a.data, mask3021), _mm_shuffle_ps(b.data, b.data, mask3102)), 
    _mm_mul_ps(_mm_shuffle_ps(a.data, a.data, mask3102), _mm_shuffle_ps(b.data, b.data, mask3021))
  )
  );

#elif __linux__

	//NYI!
#endif
}


mat4::mat4() {
#ifdef _WIN32

	data[0] = data[1] = data[2] = data[3] = ZERO;

#elif __linux__

	memset(this->data, 0, sizeof(this->data));
#endif

}
mat4::mat4(const float *arr) {
#ifdef _WIN32
	data[0] = _mm_loadu_ps(arr);
	data[1] = _mm_loadu_ps(arr + 4);	
	data[2] = _mm_loadu_ps(arr + 8);	
	data[3] = _mm_loadu_ps(arr + 12);	
#elif __linux__
	memcpy(this->data, data, (4*4)*sizeof(float));
#endif

}

mat4::mat4(const int main_diagonal_val) {
#ifdef _WIN32
	mat4 &a = (*this);
	a.zero();
	a(0,0)=main_diagonal_val;
	a(1,1)=main_diagonal_val;
	a(2,2)=main_diagonal_val;
	a(3,3)=main_diagonal_val;
#elif __linux__
	memset(a.data, 0, sizeof(a.data));
	a(0,0)=main_diagonal_val;
	a(1,1)=main_diagonal_val;
	a(2,2)=main_diagonal_val;
	a(3,3)=main_diagonal_val;
#endif

}


mat4::mat4(const vec4& c1, const vec4& c2, const vec4& c3, const vec4& c4) {

	data[0] = c1.data;
	data[1] = c2.data;
	data[2] = c3.data;
	data[3] = c4.data;

}

mat4 mat4::operator* (const mat4& R) const {

#ifdef _WIN32

	mat4 L = (*this).transposed();
	mat4 ret;
	
	// we'll choose to transpose the other matrix, and instead of calling the perhaps
	// more intuitive row(), we'll be calling column(), which is a LOT faster in comparison.
	
	// calculate using dot products (_mm_dp_ps)
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret.data[j].m128_f32[i] = _mm_dp_ps(R.data[j], L.data[i], xyzw_dot_mask).m128_f32[0];
		}
	}
	
	return ret;
#elif __linux__

	mat4 ret;
	const mat4 &L = (*this);	// for easier syntax

//	ret(0, 0) = l(0, 0)*r(0, 0) + l(1,0)*l(0,1) + l(2,0)*r(0,2) + l(3,0)*r(0,3);

for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret(j,i) = L.elementAt(j, 0)*R.elementAt(0, i) 
				 + L.elementAt(j, 1)*R.elementAt(1, i) 
				 + L.elementAt(j, 2)*R.elementAt(2, i) 
				 + L.elementAt(j, 3)*R.elementAt(3, i);

	return ret;

#endif
}

void mat4::zero() {
#ifdef _WIN32
	data[0] = data[1] = data[2] = data[3] = ZERO;
#elif __linux__
	memset(data, 0, sizeof(data));
#endif
}

vec4 mat4::operator* (const vec4& R) const {

#ifdef _WIN32
	
	// try with temporary mat4? :P
	// result: performs better!
	const mat4 M = (*this).transposed();
	vec4 v;
	for (int i = 0; i < 4; i++) 
		v.data.m128_f32[i] = _mm_dp_ps(M.data[i], R.data, xyzw_dot_mask).m128_f32[0];	

	return v;

#elif __linux
	vec4 v;
	const mat4 &L = (*this);

	for (int i = 0; i < 4; i++)
		v(i) = L.elementAt(0, i)*R.elementAt(0)
		       + L.elementAt(1, i)*R.elementAt(1)
		       + L.elementAt(2, i)*R.elementAt(2)
		       + L.elementAt(3, i)*R.elementAt(3);

	return v;
#endif


}


void mat4::identity() {
#ifdef _WIN32

	mat4 &a = (*this);
	a.zero();
	// dislike using operator(), but cba to expand it :D
	a(0,0) = 1.0;
	a(1,1) = 1.0;
	a(2,2) = 1.0;
	a(3,3) = 1.0;

#elif __linux__
	memset(this->data, 0, sizeof(this->data));

	this->data[0][0] = 1.0;
	this->data[1][1] = 1.0;
	this->data[2][2] = 1.0;
	this->data[3][3] = 1.0;
#endif
}

vec4 mat4::row(const int &i) const {
#ifdef _WIN32		
	return vec4((*this).transposed().data[i]);

#elif __linux__
	return vec4(data[0][i], data[1][i], data[2][i], data[3][i]);
#endif

	// benchmarks for 100000000 iterations
	// Two transpositions, no redirection to mat4::column:	15.896s
	// Two transpositions, redirection to mat4::column:		18.332s
	// Naive implementation:								14.140s. !
	// copy, transpose:										11.354s
	
	// for comparison: column, 100000000 iterations:		4.7s

}

vec4 mat4::column(const int &i) const {
#ifdef _WIN32
	return vec4(this->data[i]);
#elif __linux__
	return vec4(data[0][i], data[1][i], data[2][i], data[3][i]);
#endif
}

void mat4::assignToColumn(const int &column, const vec4& v) {
#ifdef _WIN32
	this->data[column] = v.data;
#elif __linux__
	// NYI
#endif

}

void mat4::assignToRow(const int &row, const vec4& v) {
#ifdef _WIN32
	// here, since 
	// 1. row operations are inherently slower than column operations, and
	// 2. transposition is blazing fast (:D)

	// we could just transpose the matrix and do some fancy sse shit with it.
	
	// this could (and probably should) be done with a reference, like this:
	// mat4.row(i) = vec4(...). However, this is only possible if the mat4 is 
	// constructed of actual vec4s (which is something one should consider anyway)

	this->T();
	this->data[row] = v.data;
	this->T();
	
#elif __linux__
		mat4& M = (*this);
	M(0, row) = v.elementAt(0);
	M(1, row) = v.elementAt(1);
	M(2, row) = v.elementAt(2);
	M(3, row) = v.elementAt(3);
#endif

}
// return by void pointer? :P
void *mat4::rawdata() const {
#ifdef _WIN32
	return (void*)&data[0];	// seems to work just fine :D
#elif __linux__

	// nyi

#endif


}

void mat4::printRaw() const {

	const float * const ptr = (float*)&this->data[0];
		
#ifdef _WIN32
	const char* fmt = "%4.3f %4.3f %4.3f %4.3f\n";

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
		

	mat4 &M = (*this);
	
	M.identity();

	M(0,0) = 2.0/(right - left);
	M(1,1) = 2.0/(top - bottom);
	M(2,2) = -2.0/(zFar - zNear);

	M(3,0) = - (right + left) / (right - left);
	M(3,1) = - (top + bottom) / (top - bottom);
	M(3,2) = - (zFar + zNear) / (zFar - zNear);
	// the element at [3][3] is already 1 (identity() was called)
}

void mat4::make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar) {
		

	mat4 &M = (*this);
	M.identity();

	M(0,0) = 2*zNear/(right-left);
	M(1,1) = 2*zNear/(top-bottom);
	M(2,0) = (right+left)/(right-left);
	M(2,1) = (top+bottom)/(top-bottom);
	M(2,2) = -(zFar + zNear)/(zFar - zNear);
	M(2,3) = -1;
	M(3,2) = -(2*zFar*zNear)/(zFar - zNear);
	

}

// performs an "in-place transposition" of the matrix

void mat4::T() {

#ifdef _WIN32

	_MM_TRANSPOSE4_PS(data[0], data[1], data[2], data[3]);	// microsoft special in-place transpose macro :P

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

mat4 mat4::transposed() const {

	mat4 ret = (*this);	// copying can't be avoided
	_MM_TRANSPOSE4_PS(ret.data[0], ret.data[1], ret.data[2], ret.data[3]);
	return ret;

}

// i'm not quite sure why anybody would ever want to construct a quaternion like this :-XD
Quaternion::Quaternion(float x, float y, float z, float w) { 
	data = _mm_set_ps(w, z, y, x);	// in reverse order
}

Quaternion::Quaternion() { data=QUAT_NO_ROTATION; }

Quaternion Quaternion::conjugate() const {
	return Quaternion(_mm_mul_ps(this->data, QUAT_CONJUGATE));	
}

void Quaternion::print() const { printf("[(%4.3f, %4.3f, %4.3f), %4.3f]\n\n", element(Q::x), element(Q::y), element(Q::z), element(Q::w)); }


void Quaternion::normalize() { 

	Quaternion &Q = (*this);
	const float mag_squared = _mm_dp_ps(Q.data, Q.data, xyzw_dot_mask).m128_f32[0];
	if (fabs(mag_squared-1.0) > 0.001) {	// to prevent calculations from exploding
		Q.data = _mm_mul_ps(Q.data, _mm_set1_ps(1.0/sqrt(mag_squared)));	
	}

}

Quaternion Quaternion::operator*(const Quaternion &b) const {
#ifdef _WIN32

	// q1*q2 = w1w2 + dot(v1,v2) + cross(v1,v2)


	const Quaternion &a = (*this);
	Quaternion ret(
	_mm_sub_ps(
	_mm_mul_ps(_mm_shuffle_ps(a.data, a.data, mask3021), _mm_shuffle_ps(b.data, b.data, mask3102)),
	_mm_mul_ps(_mm_shuffle_ps(a.data, a.data, mask3102), _mm_shuffle_ps(b.data, b.data, mask3021))));

	ret.data.m128_f32[Q::w] = a.data.m128_f32[Q::w]*b.data.m128_f32[Q::w] + _mm_dp_ps(a.data, b.data, xyz_dot_mask).m128_f32[0];

	return ret;

#elif __linux__
	//nyi
#endif 

}

Quaternion Quaternion::operator+(const Quaternion& b) const {
	return Quaternion(_mm_add_ps(this->data, b.data));
}

vec4 Quaternion::operator*(const vec4& b) const {

	vec4 v(b);
	v.normalize();
	Quaternion vec_q, res_q;
	vec_q.data = b.data;
	vec_q(Q::w) = 0.0;

	res_q = vec_q * (*this).conjugate();
	res_q = (*this)*res_q;
	
	return vec4(res_q.data);	// the w component probably contains some garbage, but it's not used anyway
}

mat4 Quaternion::toRotationMatrix() const {
	
	// using SSE, the initial combinatorics could be done with
	// 2 multiplications, two shuffles, and one regular
	// multiplication. not sure it's quite worth it though :P

	// ASSUMING NORMALIZED QUATERNION!

	using namespace Q;	// to use x, y etc. instead of Q::x, Q::y

	const float x2 = element(x)*element(x);
	const float y2 = element(y)*element(y);
	const float z2 = element(z)*element(z);
	const float xy = element(x)*element(y);
	const float xz = element(x)*element(z);
	const float xw = element(x)*element(w);
	const float yz = element(y)*element(z);
	const float yw = element(y)*element(w);
	const float zw = element(z)*element(w);

	return mat4(vec4(1.0 - 2.0*(y2 + z2), 2.0*(xy-zw), 2.0*(xz + yw), 0.0f),
				vec4(2.0 * (xy + zw), 1.0 - 2.0*(x2 + z2), 2.0*(yz - xw), 0.0),
				vec4(2.0 * (xz - yw), 2.0 * (yz + xw), 1.0 - 2.0 * (x2 + y2), 0.0),
				vec4(0.0, 0.0, 0.0, 1.0));

}