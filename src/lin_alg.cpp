#include "lin_alg.h"

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
	const float l_recip = 1.0/sqrt(_mm_dp_ps(this->data, this->data, 0x71).m128_f32[0]); // only x,y,z components
	const __m128 d = _mm_set1_ps(l_recip);

	this->data = _mm_mul_ps(this->data, d);
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

float& vec4::operator() (const int & row) {	

#ifdef _WIN32
	return data.m128_f32[row];
#elif __linux__
	return data[row];
#endif
}

float vec4::elementAt(const int& row) const {
#ifdef _WIN32
	return data.m128_f32[row];
#elif __linux__
	return data[row];
#endif
}

float dot(const vec4 &a, const vec4 &b) {
#ifdef _WIN32

	static const int mask = 0x71; // == 0111 0001_2, which means "use x,y,z components of the vectors, store result in the lowest word only"
	__m128 dot = _mm_dp_ps(a.data, b.data, mask);	// direct computation of dot product (SSE4)
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
	// Absolutely beautiful (the exact same recipe can be found at
	// http://neilkemp.us/src/sse_tutorial/sse_tutorial.html#E, albeit in assembly.)
	
#ifdef _WIN32

	return vec4(
	_mm_sub_ps(
    _mm_mul_ps(_mm_shuffle_ps(a.data, a.data, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b.data, b.data, _MM_SHUFFLE(3, 1, 0, 2))), 
    _mm_mul_ps(_mm_shuffle_ps(a.data, a.data, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b.data, b.data, _MM_SHUFFLE(3, 0, 2, 1)))
  )
  );

#elif __linux__

	//NYI!
#endif
}


mat4::mat4() {
#ifdef _WIN32

#ifdef NDEBUG
	const __m128 zero;	// no need to initialize; only being used for _mm_xor_ps(zero,zero), which always returns a 0-vector
#else
	const __m128 zero = _mm_setzero_ps();
#endif
	data[0] = data[1] = data[2] = data[3] = _mm_xor_ps(zero,zero);

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

mat4 mat4::operator* (const mat4& R) {

#ifdef _WIN32

	mat4 &L = (*this);
	mat4 ret;
	
	// we'll choose to transpose the other matrix, and instead of calling the perhaps
	// more intuitive row(), we'll be calling column(), which is a LOT faster in comparison.
	L.T();
	
	static const int mask = 0xF1;	// a different mask than what's used in dot().
									// this time, we want to include the 4th component as well
	
	// calculate using dot products (_mm_dp_ps)
	

	for (int i = 0; i < 4; i++) {
		//vec4 b(L.column(i));		
		//const __m128 b = _mm_load_ps(&L.data[i][0]); //vec4 b(L.column(i));
		for (int j = 0; j < 4; j++) {
			//__m128 r = _mm_dp_ps(R.data[j], L.data[i], mask);
			//ret.data[j].m128_f32[i] = r.m128_f32[0];		//ret(j, i) = r.m128_f32[0];
			ret.data[j].m128_f32[i] = _mm_dp_ps(R.data[j], L.data[i], mask).m128_f32[0];
		}
	}
	L.T();
	
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

#ifdef NDEBUG
	
	const __m128 zero;
		// deliberately uninitialized, since it doesn't matter what you XOR	
#else
	const __m128 zero = _mm_setzero_ps();
#endif
	data[0] = data[1] = data[2] = data[3] = _mm_xor_ps(zero,zero);
}

vec4 mat4::operator* (const vec4& R) {

#ifdef _WIN32
	/*static const int mask = 0xF1; // == 1111 0001_2, which means "use x,y,z,w components of the vectors, store result in the lowest word only"
	mat4 &M = (*this);
	vec4 v;
	// dot() can't be used here, since it discards the w component!
	for (int i = 0; i < 4; i++) 
		v.data.m128_f32[i] = _mm_dp_ps(M.row(i).data, R.data, mask).m128_f32[0]; 
	
	return v;*/
	
	// with transposition :D
	mat4 &M = (*this);
	vec4 v;
	static const int mask = 0xF1;
	M.T();
	for (int i = 0; i < 4; i++) 
		//v.data.m128_f32[i] = _mm_dp_ps(M.column(i).data, R.data, mask).m128_f32[0];	
		v.data.m128_f32[i] = _mm_dp_ps(M.data[i], R.data, mask).m128_f32[0];	
	
	M.T();
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


float& mat4::operator() ( const int &column, const int &row ) {

	// no bounds checking! 
	return data[column].m128_f32[row];
}

float mat4::elementAt(const int &column, const int &row) const {

	return data[column].m128_f32[row];

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

vec4 mat4::row(const int &i) {
#ifdef _WIN32
		
	this->T();
	const __m128 row = this->data[i];
	this->T();
	return vec4(row);
	
	//return vec4(_mm_setr_ps(M.elementAt(0,i), M.elementAt(1,i), M.elementAt(2,i), M.elementAt(3,i)));
	//return vec4(_mm_set_ps(M.elementAt(3,i), M.elementAt(2,i), M.elementAt(1,i), M.elementAt(0,i)));
	
	// with transposition 

#elif __linux__
	return vec4(data[0][i], data[1][i], data[2][i], data[3][i]);
#endif

	// benchmarks for 100000000 iterations
	// Two transpositions, no redirection to mat4::column:	15.896s
	// Two transpositions, redirection to mat4::column:		18.332s
	// Naive implementation:								14.140s. !	

}

vec4 mat4::column(const int &i) {
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
	mat4 &M = (*this);
	
	M.identity();

	M(0,0) = 2.0/(right - left);
	M(1,1) = 2.0/(top - bottom);
	M(2,2) = -2.0/(zFar - zNear);

	// this can be simplified further, if the viewing volume is symmetric,
	// i.e. right - left == 0 && top-bottom == 0 && zFar-zNear == 0.
	// But it isn't. =)

	M(3,0) = - (right + left) / (right - left);
	M(3,1) = - (top + bottom) / (top - bottom);
	M(3,2) = - (zFar + zNear) / (zFar - zNear);
	// the element at [3][3] is already 1 (identity() was called)
}

void mat4::make_proj_perspective(float const & left, float const & right, float const & bottom, float const & top, float const & zNear, float const & zFar) {
		
	// STUB(B)! (probably never going to be used)
	// nop
	return;

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
