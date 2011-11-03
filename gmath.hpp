////////////////////////////////////////////////////////////////////////////////
// \file    gmath.hpp
// \author  Jonathan Dupuy
// \brief   Basic math library for computer graphics.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef GMATH_HPP
#define GMATH_HPP

#include <cstddef>  // size_t
#include <cassert>  // assert
#include <cmath>    // sqrt
#ifdef _WIN32
#   define M_PI 3.14159265358979323846
#endif


////////////////////////////////////////////////////////////////////////////////
// Type declarations
template <typename T>   class vec3;
typedef vec3<float>     vec3f;
typedef vec3<double>    vec3d;
typedef vec3<int>       vec3i;
typedef vec3<unsigned>  vec3u;

template <typename T>   class vec4;
typedef vec4<float>     vec4f;
typedef vec4<double>    vec4d;
typedef vec4<int>       vec4i;
typedef vec4<unsigned>  vec4u;

template <typename T>   class mat3;
typedef mat3<float>     mat3f;
typedef mat3<double>    mat3d;
typedef mat3<int>       mat3i;
typedef mat3<unsigned>  mat3u;

template <typename T>   class mat4;
typedef mat4<float>     mat4f;
typedef mat4<double>    mat4d;
typedef mat4<int>       mat4i;
typedef mat4<unsigned>  mat4u;


////////////////////////////////////////////////////////////////////////////////
// Vector function declarations
// Dot product
template <typename T>
	T dot(const vec3<T>& u, const vec3<T>& v);
template <typename T>
	T dot(const vec4<T>& u, const vec4<T>& v);

// Cross product
template <typename T>
	vec3<T> cross(const vec3<T>& u, const vec3<T>& v);

// Length
template <typename T>
	T length(const vec3<T>& v);
template <typename T>
	T length(const vec4<T>& v);

// Normalization
template <typename T>
	vec3<T> normalize(const vec3<T>& v);
template <typename T>
	vec4<T> normalize(const vec4<T>& v);

// Distance
template <typename T>
	T distance(const vec3<T>& u, const vec3<T>& v);

// Reflect
template <typename T>
	vec3<T> reflect(const vec3<T>& i, const vec3<T>& n);

// Refract
template <typename T>
	vec3<T> refract(const vec3<T>& i, const vec3<T>& n, const T& eta);


////////////////////////////////////////////////////////////////////////////////
// Matrix function declarations
// Idendity
template <typename T>
	void identity(mat3<T>& m);
template <typename T>
	void identity(mat4<T>& m);

// Transposition
template <typename T>
	mat3<T> transpose(const mat3<T>& m);
template <typename T>
	mat4<T> transpose(const mat4<T>& m);

// Inverse
template <typename T>
	mat3<T> inverse(const mat3<T>& m);
template <typename T>
	mat4<T> inverse(const mat4<T>& m);

// Determinant
template <typename T>
	T determinant(const mat3<T>& m);
template <typename T>
	T determinant(const mat4<T>& m);

// Affine transformations
template <typename T>
	mat3<T> rotatex(const T& degrees);
//template <typename T>
//    mat4<T> rotatex(const T& degrees);
template <typename T>
	mat3<T> rotatey(const T& degrees);
//template <typename T>
//    mat4<T> rotatey(const T& degrees);
template <typename T>
	mat3<T> rotatez(const T& degrees);
//template <typename T>
//    mat4<T> rotatez(const T& degrees);

// Projections
template <typename T>
	mat4<T> perspective(const T& fovy, const T& aspect, const T& zNear, const T& zFar);
template <typename T>
	mat4<T> frustum(const T& left, const T& right, const T& bottom, const T& top, const T& nearVal, const T& farVal);
template <typename T>
	mat4<T> ortho(const T& left, const T& right, const T& bottom, const T& top, const T& nearVal, const T& farVal);


////////////////////////////////////////////////////////////////////////////////
// vec3 definition
template <typename T>
class vec3
{
public:
	// Constructors
	vec3();
	vec3(const T& x, const T& y, const T& z);
	template <typename U>
	explicit vec3(const vec3<U>& v);

	// Arithmetic operators
	const vec3 operator+(const vec3& v)   const;
	const vec3 operator-(const vec3& v)   const;
	const vec3 operator*(const T& s)      const;
	const vec3 operator*(const vec3& v)   const;
	const vec3 operator/(const vec3& v)   const;

	// Assignement operators
	vec3& operator+=(const vec3& v);
	vec3& operator-=(const vec3& v);
	vec3& operator*=(const T& s);
	vec3& operator*=(const vec3& v);
	vec3& operator/=(const vec3& v);

	// Comparison operators
	bool operator==(const vec3& v)  const;
	bool operator!=(const vec3& v)  const;

	// Member access
	const T& operator[](size_t i)    const;
	T& operator[](size_t i);

	// Members
	T x;
	T y;
	T z;
};

template <typename T>
inline
vec3<T>::vec3()
{

}

template <typename T>
inline
vec3<T>::vec3(const T& x, const T& y, const T& z) :
	x(x), y(y), z(z)
{

}

template <typename T>
template <typename U>
inline
vec3<T>::vec3(const vec3<U>& v) :
	x(T(v[0])), y(T(v[1])), z(T(v[2]))
{

}

template <typename T>
inline
const vec3<T> vec3<T>::operator+(const vec3<T>& v)  const
{
	return vec3<T>(x+v[0], y+v[1], z+v[2]);
}

template <typename T>
inline
const vec3<T> vec3<T>::operator-(const vec3<T>& v)  const
{
	return vec3<T>(x-v[0], y-v[1], z-v[2]);
}

template <typename T>
inline
const vec3<T> vec3<T>::operator*(const T& s)  const
{
	return vec3<T>(x*s, y*s, z*s);
}

// left sided multiplication
template <typename T, typename U>
inline
const vec3<U> operator*(const U& s, const vec3<T>& v)
{
	return vec3<U>(v[0]*s, v[1]*s, v[2]*s);
}

template <typename T>
inline
const vec3<T> vec3<T>::operator*(const vec3<T>& v)  const
{
	return vec3<T>(x*v[0], y*v[1], z*v[2]);
}

template <typename T>
inline
const vec3<T> vec3<T>::operator/(const vec3<T>& v)  const
{
	return vec3<T>(x/v[0], y/v[1], z/v[2]);
}

template <typename T>
inline
vec3<T>& vec3<T>::operator+=(const vec3<T>& v)
{
	x+=v[0];
	y+=v[1];
	z+=v[2];
	return (*this);
}

template <typename T>
inline
vec3<T>& vec3<T>::operator-=(const vec3<T>& v)
{
	x-=v[0];
	y-=v[1];
	z-=v[2];
	return (*this);
}

template <typename T>
inline
vec3<T>& vec3<T>::operator*=(const T& s)
{
	x*=s;
	y*=s;
	z*=s;
	return (*this);
}

template <typename T>
inline
vec3<T>& vec3<T>::operator*=(const vec3<T>& v)
{
	x*=v[0];
	y*=v[1];
	z*=v[2];
	return (*this);
}

template <typename T>
inline
vec3<T>& vec3<T>::operator/=(const vec3<T>& v)
{
	x/=v[0];
	y/=v[1];
	z/=v[2];
	return (*this);
}

template <typename T>
inline
bool vec3<T>::operator==(const vec3<T>& v)  const
{
	return (x==v[0] && y==v[1] && z==v[2]);
}

template <typename T>
inline
bool vec3<T>::operator!=(const vec3<T>& v)  const
{
	return (x!=v[0] || y!=v[1] || z!=v[2]);
}

template <typename T>
inline
const T& vec3<T>::operator[](size_t i)  const
{
	assert(i<3);
	return (&x)[i];
}

template <typename T>
inline
T& vec3<T>::operator[](size_t i)
{
	assert(i<3);
	return (&x)[i];
//	return const_cast<T&>((static_cast< const vec3<T> >(*this))[i]);
}

////////////////////////////////////////////////////////////////////////////////
// vec4
template <typename T>
class vec4
{
public:
	// Constructors
	vec4();
	vec4(const T& x, const T& y, const T& z, const T& w);
	template <typename U>
	explicit vec4(const vec4<U>& v);

	// Arithmetic operators
	const vec4 operator+(const vec4& v)   const;
	const vec4 operator-(const vec4& v)   const;
	const vec4 operator*(const T& s)      const;
	const vec4 operator*(const vec4& v)   const;
	const vec4 operator/(const vec4& v)   const;

	// Assignement operators
	vec4& operator+=(const vec4& v);
	vec4& operator-=(const vec4& v);
	vec4& operator*=(const T& s);
	vec4& operator*=(const vec4& v);
	vec4& operator/=(const vec4& v);

	// Comparison operators
	bool operator==(const vec4& v)  const;
	bool operator!=(const vec4& v)  const;

	// Member access
	const T& operator[](size_t i)    const;
	T& operator[](size_t i);

	// Members
	T x;
	T y;
	T z;
	T w;
};

template <typename T>
inline
vec4<T>::vec4()
{

}

template <typename T>
inline
vec4<T>::vec4(const T& x, const T& y, const T& z, const T& w) :
	x(x), y(y), z(z), w(w)
{

}

template <typename T>
template <typename U>
inline
vec4<T>::vec4(const vec4<U>& v) :
	x(T(v[0])), y(T(v[1])), z(T(v[2])), w(T(v[3]))
{

}

template <typename T>
inline
const vec4<T> vec4<T>::operator+(const vec4<T>& v)  const
{
	return vec4<T>(x+v[0], y+v[1], z+v[2], w+v[3]);
}

template <typename T>
inline
const vec4<T> vec4<T>::operator-(const vec4<T>& v)  const
{
	return vec4<T>(x-v[0], y-v[1], z-v[2], w-v[3]);
}

template <typename T>
inline
const vec4<T> vec4<T>::operator*(const T& s)  const
{
	return vec4<T>(x*s, y*s, z*s, w*s);
}

// left sided multiplication
template <typename T, typename U>
inline
const vec4<U> operator*(const U& s, const vec4<T>& v)
{
	return vec4<U>(v[0]*s, v[1]*s, v[2]*s, v[3]*s);
}

template <typename T>
inline
const vec4<T> vec4<T>::operator*(const vec4<T>& v)  const
{
	return vec4<T>(x*v[0], y*v[1], z*v[2], w*v[3]);
}

template <typename T>
inline
const vec4<T> vec4<T>::operator/(const vec4<T>& v)  const
{
	return vec4<T>(x/v[0], y/v[1], z/v[2], w/v[3]);
}

template <typename T>
inline
vec4<T>& vec4<T>::operator+=(const vec4<T>& v)
{
	x+=v[0];
	y+=v[1];
	z+=v[2];
	w+=v[3];
	return (*this);
}

template <typename T>
inline
vec4<T>& vec4<T>::operator-=(const vec4<T>& v)
{
	x-=v[0];
	y-=v[1];
	z-=v[2];
	w-=v[3];
	return (*this);
}

template <typename T>
inline
vec4<T>& vec4<T>::operator*=(const T& s)
{
	x*=s;
	y*=s;
	z*=s;
	w*=s;
	return (*this);
}

template <typename T>
inline
vec4<T>& vec4<T>::operator*=(const vec4<T>& v)
{
	x*=v[0];
	y*=v[1];
	z*=v[2];
	w*=v[3];
	return (*this);
}

template <typename T>
inline
vec4<T>& vec4<T>::operator/=(const vec4<T>& v)
{
	x/=v[0];
	y/=v[1];
	z/=v[2];
	w/=v[3];
	return (*this);
}

template <typename T>
inline
bool vec4<T>::operator==(const vec4<T>& v)  const
{
	return (x==v[0] && y==v[1] && z==v[2] && w==v[3]);
}

template <typename T>
inline
bool vec4<T>::operator!=(const vec4<T>& v)  const
{
	return (x!=v[0] || y!=v[1] || z!=v[2] || w!=v[3]);
}

template <typename T>
inline
const T& vec4<T>::operator[](size_t i)  const
{
	assert(i<4);
	return (&x)[i];
}

template <typename T>
inline
T& vec4<T>::operator[](size_t i)
{
	assert(i<4);
	return (&x)[i];
//	return const_cast<T&>((static_cast< const vec4<T> >(*this))[i]);
}


////////////////////////////////////////////////////////////////////////////////
// mat3
template <typename T>
class mat3
{
public:
	// Constructors
	mat3();
	mat3(const vec3<T>& c0, const vec3<T>& c1, const vec3<T>& c2);
	mat3(const T& m00, const T& m10, const T& m20,
		 const T& m01, const T& m11, const T& m21,
		 const T& m02, const T& m12, const T& m22  );
	template <typename U>
	explicit mat3(const mat3<U>& m);

	// Arithmetic operators
	const mat3 operator+(const mat3& m)       const;
	const mat3 operator-(const mat3& m)       const;
	const mat3 operator*(const T& s)          const;
	const vec3<T> operator*(const vec3<T>& v) const;
	const mat3 operator*(const mat3& m)       const;

	// Assignement operators
	mat3& operator+=(const mat3& m);
	mat3& operator-=(const mat3& m);
	mat3& operator*=(const mat3& m);
	mat3& operator*=(const T& s);

	// Comparison operators
	bool operator==(const mat3& m)      const;
	bool operator!=(const mat3& m)      const;

	// Member access
	const vec3<T>& operator[](size_t i) const;
	vec3<T>& operator[](size_t i);

private:
	// Members
	vec3<T> _data[3];
};

template <typename T>
inline
mat3<T>::mat3()
{

}

template <typename T>
inline
mat3<T>::mat3(const vec3<T>& c0, const vec3<T>& c1, const vec3<T>& c2)
{
	_data[0] = c0;
	_data[1] = c1;
	_data[2] = c2;
}

template <typename T>
inline
mat3<T>::mat3(  const T& m00, const T& m10, const T& m20,
                const T& m01, const T& m11, const T& m21,
                const T& m02, const T& m12, const T& m22)
{
	_data[0][0] = m00;
	_data[0][1] = m01;
	_data[0][2] = m02;
	_data[1][0] = m10;
	_data[1][1] = m11;
	_data[1][2] = m12;
	_data[2][0] = m20;
	_data[2][1] = m21;
	_data[2][2] = m22;
}

template <typename T>
template <typename U>
mat3<T>::mat3(const mat3<U>& m)
{
	_data[0] = vec3<T>(m[0]);
	_data[1] = vec3<T>(m[1]);
	_data[2] = vec3<T>(m[2]);
}

template <typename T>
inline
const mat3<T> mat3<T>::operator+(const mat3<T>& m)    const
{
	return mat3<T>(_data[0]+m[0], _data[1]+m[1], _data[2]+m[2]);
}

template <typename T>
inline
const mat3<T> mat3<T>::operator-(const mat3<T>& m)    const
{
	return mat3<T>(_data[0]-m[0], _data[1]-m[1], _data[2]-m[2]);
}

template <typename T>
inline
const mat3<T> mat3<T>::operator*(const T& s)    const
{
	return mat3<T>(_data[0]*s, _data[1]*s, _data[2]*s);
}

// left sided multiplication
template <typename T, typename U>
inline
const mat3<U> operator*(const U& s, const mat3<T>& m)
{
	return mat3<U>(m[0]*s, m[1]*s, m[2]*s);
}

template <typename T>
inline
const vec3<T> mat3<T>::operator*(const vec3<T>& v)    const
{
	return vec3<T>(_data[0][0]*v[0]+_data[1][0]*v[1]+_data[2][0]*v[2],
	               _data[0][1]*v[0]+_data[1][1]*v[1]+_data[2][1]*v[2],
	               _data[0][2]*v[0]+_data[1][2]*v[1]+_data[2][2]*v[2]);
}

template <typename T>
inline
const mat3<T> mat3<T>::operator*(const mat3<T>& m)    const
{
	return mat3<T>(_data[0][0]*m[0][0]+_data[1][0]*m[0][1]+_data[2][0]*m[0][2],
	               _data[0][0]*m[1][0]+_data[1][0]*m[1][1]+_data[2][0]*m[1][2],
	               _data[0][0]*m[2][0]+_data[1][0]*m[2][1]+_data[2][0]*m[2][2],
	               _data[0][1]*m[0][0]+_data[1][1]*m[0][1]+_data[2][1]*m[0][2],
	               _data[0][1]*m[1][0]+_data[1][1]*m[1][1]+_data[2][1]*m[1][2],
	               _data[0][1]*m[2][0]+_data[1][1]*m[2][1]+_data[2][1]*m[2][2],
	               _data[0][2]*m[0][0]+_data[1][2]*m[0][1]+_data[2][2]*m[0][2],
	               _data[0][2]*m[1][0]+_data[1][2]*m[1][1]+_data[2][2]*m[1][2],
	               _data[0][2]*m[2][0]+_data[1][2]*m[2][1]+_data[2][2]*m[2][2] );
}

template <typename T>
inline
mat3<T>& mat3<T>::operator+=(const mat3<T>& m)
{
	_data[0]+=m[0];
	_data[1]+=m[1];
	_data[2]+=m[2];
	return (*this);
}

template <typename T>
inline
mat3<T>& mat3<T>::operator-=(const mat3<T>& m)
{
	_data[0]-=m[0];
	_data[1]-=m[1];
	_data[2]-=m[2];
	return (*this);
}

template <typename T>
inline
mat3<T>& mat3<T>::operator*=(const T& s)
{
	_data[0]*=s;
	_data[1]*=s;
	_data[2]*=s;
	return (*this);
}

template <typename T>
inline
mat3<T>& mat3<T>::operator*=(const mat3<T>& m)
{
	return (*this = *this * m);
}

template <typename T>
inline
bool mat3<T>::operator==(const mat3<T>& m)  const
{
	return (_data[0]==m[0] && _data[1]==m[1] && _data[2]==m[2]);
}

template <typename T>
inline
bool mat3<T>::operator!=(const mat3<T>& m)  const
{
	return (_data[0]!=m[0] || _data[1]!=m[1] || _data[2]!=m[2]);
}

template <typename T>
inline
const vec3<T>& mat3<T>::operator[](size_t i)    const
{
	assert(i<3);
	return _data[i];
}

template <typename T>
inline
vec3<T>& mat3<T>::operator[](size_t i)
{
	assert(i<3);
	return _data[i];
//	return const_cast<vec3<T>&>((static_cast< const mat3<T> >(*this))[i]);
}


////////////////////////////////////////////////////////////////////////////////
// mat4
template <typename T>
class mat4
{
public:
	// Constructors
	mat4();
	mat4(const vec4<T>& c0, const vec4<T>& c1, const vec4<T>& c2, const vec4<T>& c3);
	mat4(const T& m00, const T& m10, const T& m20, const T& m30,
		 const T& m01, const T& m11, const T& m21, const T& m31,
		 const T& m02, const T& m12, const T& m22, const T& m32,
		 const T& m03, const T& m13, const T& m23, const T& m33 );
	template <typename U>
	explicit mat4(const mat4<U>& m);

	// Arithmetic operators
	const mat4 operator+(const mat4& m)       const;
	const mat4 operator-(const mat4& m)       const;
	const mat4 operator*(const T& s)          const;
	const vec4<T> operator*(const vec4<T>& v) const;
	const mat4 operator*(const mat4& m)       const;

	// Assignement operators
	mat4& operator+=(const mat4& m);
	mat4& operator-=(const mat4& m);
	mat4& operator*=(const mat4& m);
	mat4& operator*=(const T& s);

	// Comparison operators
	bool operator==(const mat4& m)      const;
	bool operator!=(const mat4& m)      const;

	// Member access
	const vec4<T>& operator[](size_t i) const;
	vec4<T>& operator[](size_t i);

private:
	// Members
	vec4<T> _data[4];
};

template <typename T>
inline
mat4<T>::mat4()
{

}

template <typename T>
inline
mat4<T>::mat4(const vec4<T>& c0, const vec4<T>& c1, const vec4<T>& c2, const vec4<T>& c3)
{
	_data[0] = c0;
	_data[1] = c1;
	_data[2] = c2;
	_data[3] = c3;
}

template <typename T>
inline
mat4<T>::mat4(  const T& m00, const T& m10, const T& m20, const T& m30,
                const T& m01, const T& m11, const T& m21, const T& m31,
                const T& m02, const T& m12, const T& m22, const T& m32,
                const T& m03, const T& m13, const T& m23, const T& m33  )
{
	_data[0][0] = m00;
	_data[0][1] = m01;
	_data[0][2] = m02;
	_data[0][3] = m03;
	_data[1][0] = m10;
	_data[1][1] = m11;
	_data[1][2] = m12;
	_data[1][3] = m13;
	_data[2][0] = m20;
	_data[2][1] = m21;
	_data[2][2] = m22;
	_data[2][3] = m23;
	_data[3][0] = m30;
	_data[3][1] = m31;
	_data[3][2] = m32;
	_data[3][3] = m33;
}

template <typename T>
template <typename U>
mat4<T>::mat4(const mat4<U>& m)
{
	_data[0] = vec4<T>(m[0]);
	_data[1] = vec4<T>(m[1]);
	_data[2] = vec4<T>(m[2]);
	_data[3] = vec4<T>(m[3]);
}

template <typename T>
inline
const mat4<T> mat4<T>::operator+(const mat4<T>& m)    const
{
	return mat4<T>(_data[0]+m[0], _data[1]+m[1], _data[2]+m[2], _data[3]+m[3]);
}

template <typename T>
inline
const mat4<T> mat4<T>::operator-(const mat4<T>& m)    const
{
	return mat4<T>(_data[0]-m[0], _data[1]-m[1], _data[2]-m[2], _data[3]-m[3]);
}

template <typename T>
inline
const mat4<T> mat4<T>::operator*(const T& s)    const
{
	return mat4<T>(_data[0]*s, _data[1]*s, _data[2]*s, _data[3]*s);
}

// left sided multiplication
template <typename T, typename U>
inline
const mat4<U> operator*(const U& s, const mat4<T>& m)
{
	return mat4<T>(m[0]*s, m[1]*s, m[2]*s, m[3]*s);
}

template <typename T>
inline
const vec4<T> mat4<T>::operator*(const vec4<T>& v)    const
{
	return vec4<T>(_data[0][0]*v[0]+_data[1][0]*v[1]+_data[2][0]*v[2]+_data[3][0]*v[3],
	               _data[0][1]*v[0]+_data[1][1]*v[1]+_data[2][1]*v[2]+_data[3][1]*v[3],
	               _data[0][2]*v[0]+_data[1][2]*v[1]+_data[2][2]*v[2]+_data[3][2]*v[3],
	               _data[0][3]*v[0]+_data[1][3]*v[1]+_data[2][3]*v[2]+_data[3][3]*v[3]);
}

template <typename T>
inline
const mat4<T> mat4<T>::operator*(const mat4<T>& m)    const
{
	return mat4<T>(_data[0][0]*m[0][0]+_data[1][0]*m[0][1]+_data[2][0]*m[0][2]+_data[3][0]*m[0][3],
	               _data[0][0]*m[1][0]+_data[1][0]*m[1][1]+_data[2][0]*m[1][2]+_data[3][0]*m[1][3],
	               _data[0][0]*m[2][0]+_data[1][0]*m[2][1]+_data[2][0]*m[2][2]+_data[3][0]*m[2][3],
	               _data[0][0]*m[3][0]+_data[1][0]*m[3][1]+_data[2][0]*m[3][2]+_data[3][0]*m[3][3],
	               _data[0][1]*m[0][0]+_data[1][1]*m[0][1]+_data[2][1]*m[0][2]+_data[3][1]*m[0][3],
	               _data[0][1]*m[1][0]+_data[1][1]*m[1][1]+_data[2][1]*m[1][2]+_data[3][1]*m[1][3],
	               _data[0][1]*m[2][0]+_data[1][1]*m[2][1]+_data[2][1]*m[2][2]+_data[3][1]*m[2][3],
	               _data[0][1]*m[3][0]+_data[1][1]*m[3][1]+_data[2][1]*m[3][2]+_data[3][1]*m[3][3],
	               _data[0][2]*m[0][0]+_data[1][2]*m[0][1]+_data[2][2]*m[0][2]+_data[3][2]*m[0][3],
	               _data[0][2]*m[1][0]+_data[1][2]*m[1][1]+_data[2][2]*m[1][2]+_data[3][2]*m[1][3],
	               _data[0][2]*m[2][0]+_data[1][2]*m[2][1]+_data[2][2]*m[2][2]+_data[3][2]*m[2][3],
	               _data[0][2]*m[3][0]+_data[1][2]*m[3][1]+_data[2][2]*m[3][2]+_data[3][2]*m[3][3],
	               _data[0][3]*m[0][0]+_data[1][3]*m[0][1]+_data[2][3]*m[0][2]+_data[3][3]*m[0][3],
	               _data[0][3]*m[1][0]+_data[1][3]*m[1][1]+_data[2][3]*m[1][2]+_data[3][3]*m[1][3],
	               _data[0][3]*m[2][0]+_data[1][3]*m[2][1]+_data[2][3]*m[2][2]+_data[3][3]*m[2][3],
	               _data[0][3]*m[3][0]+_data[1][3]*m[2][1]+_data[2][3]*m[3][2]+_data[3][3]*m[3][3] );
}

template <typename T>
inline
mat4<T>& mat4<T>::operator+=(const mat4<T>& m)
{
	_data[0]+=m[0];
	_data[1]+=m[1];
	_data[2]+=m[2];
	_data[3]+=m[3];
	return (*this);
}

template <typename T>
inline
mat4<T>& mat4<T>::operator-=(const mat4<T>& m)
{
	_data[0]-=m[0];
	_data[1]-=m[1];
	_data[2]-=m[2];
	_data[3]-=m[3];
	return (*this);
}

template <typename T>
inline
mat4<T>& mat4<T>::operator*=(const T& s)
{
	_data[0]*=s;
	_data[1]*=s;
	_data[2]*=s;
	_data[3]*=s;
	return (*this);
}

template <typename T>
inline
mat4<T>& mat4<T>::operator*=(const mat4<T>& m)
{
	return (*this = *this * m);
}

template <typename T>
inline
bool mat4<T>::operator==(const mat4<T>& m)  const
{
	return (_data[0]==m[0] && _data[1]==m[1] && _data[2]==m[2] && _data[3]==m[3]);
}

template <typename T>
inline
bool mat4<T>::operator!=(const mat4<T>& m)  const
{
	return (_data[0]!=m[0] || _data[1]!=m[1] || _data[2]!=m[2] || _data[3]!=m[3]);
}

template <typename T>
inline
const vec4<T>& mat4<T>::operator[](size_t i)    const
{
	assert(i<4);
	return _data[i];
}

template <typename T>
inline
vec4<T>& mat4<T>::operator[](size_t i)
{
//	return const_cast<vec4<T>&>((static_cast< const mat4<T> >(*this))[i]);
	assert(i<4);
	return _data[i];
}


////////////////////////////////////////////////////////////////////////////////
// Vector function implementation
template <typename T>
inline
T dot(const vec3<T>& u, const vec3<T>& v)
{
	return u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
}

template <typename T>
inline
T dot(const vec4<T>& u, const vec4<T>& v)
{
	return u[0]*v[0]+u[1]*v[1]+u[2]*v[2]+u[3]*v[3];
}

template <typename T>
inline
vec3<T> cross(const vec3<T>& u, const vec3<T>& v)
{
    return vec3<T>(u[1]*v[2]-v[1]*u[2], u[2]*v[0]-v[2]*u[0], u[0]*v[1]-v[0]*u[1]);
}

template <typename T>
inline
T length(const vec3<T>& v)
{
    return sqrt(dot(v,v));
}

template <typename T>
inline
T length(const vec4<T>& v)
{
    return sqrt(dot(v,v));
}

template <typename T>
inline
vec3<T> normalize(const vec3<T>& v)
{
    return 1.0f/length(v) * v;
}

template <typename T>
inline
vec4<T> normalize(const vec4<T>& v)
{
    return 1.0f/length(v) * v;
}

template <typename T>
inline
T distance(const vec3<T>& u, const vec3<T>& v)
{
    return length(u-v);
}

template <typename T>
inline
vec3<T> reflect(const vec3<T>& i, const vec3<T>& n)
{
    return i - T(2)*dot(n, i)*n;
}

template <typename T>
inline
vec3<T> refract(const vec3<T>& i, const vec3<T>& n, const T& eta)
{
    const T k = T(1) - eta*eta * ( T(1) - dot(n,i)*dot(n,i) );
    assert(k >= T(0));
    return eta * i - (eta * dot(n,i) + sqrt(k)) * n;
}


////////////////////////////////////////////////////////////////////////////////
// Matrix function implementation
template <typename T>
inline
void identity(mat3<T>& m)
{
    m = mat3<T>(1,0,0,
                0,1,0,
                0,0,1);
}

template <typename T>
inline
void identity(mat4<T>& m)
{
    m = mat4<T>(1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1);
}

template <typename T>
inline
mat3<T> transpose(const mat3<T>& m)
{
    return mat3<T>( m[0][0], m[0][1], m[0][2],
                    m[1][0], m[1][1], m[1][2],
                    m[2][0], m[2][1], m[2][2]   );
}

template <typename T>
inline
mat4<T> transpose(const mat4<T>& m)
{
    return mat3<T>( m[0][0], m[0][1], m[0][2], m[0][3],
                    m[1][0], m[1][1], m[1][2], m[1][3],
                    m[2][0], m[2][1], m[2][2], m[2][3],
                    m[3][0], m[3][1], m[3][2], m[3][3]   );
}

template <typename T>
inline
mat3<T> inverse(const mat3<T>& m)
{
    const T det = determinant(m);
    assert(det != T(0));

    const T invdet = T(1)/det;

    mat3<T> tcm; // transposed comatrix
    tcm[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]);
    tcm[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]);
    tcm[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]);
    tcm[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]);
    tcm[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]);
    tcm[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]);
    tcm[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
    tcm[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]);
    tcm[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]);

    return invdet*tcm;
}

template <typename T>
inline
mat4<T> inverse(const mat4<T>& m)
{
    T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

    T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

    T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

    T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
    T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

    T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
    T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

    T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
    T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

    const vec4<T> SignA(+1, -1, +1, -1);
    const vec4<T> SignB(-1, +1, -1, +1);

    vec4<T> Fac0(Coef00, Coef00, Coef02, Coef03);
    vec4<T> Fac1(Coef04, Coef04, Coef06, Coef07);
    vec4<T> Fac2(Coef08, Coef08, Coef10, Coef11);
    vec4<T> Fac3(Coef12, Coef12, Coef14, Coef15);
    vec4<T> Fac4(Coef16, Coef16, Coef18, Coef19);
    vec4<T> Fac5(Coef20, Coef20, Coef22, Coef23);

    vec4<T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
    vec4<T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
    vec4<T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
    vec4<T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

    vec4<T> Inv0 = SignA * (Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    vec4<T> Inv1 = SignB * (Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    vec4<T> Inv2 = SignA * (Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    vec4<T> Inv3 = SignB * (Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

    mat4<T> im(Inv0, Inv1, Inv2, Inv3);

    vec4<T> Row0(im[0][0], im[1][0], im[2][0], im[3][0]);

    T det = dot(m[0], Row0);
    assert(det != T(0));

    return (T(1)/det)*im;
}

template <typename T>
inline
T determinant(const mat3<T>& m)
{
    return  (m[0][0]*m[1][1]*m[2][2]
            +m[0][1]*m[1][2]*m[2][0]
            +m[0][2]*m[1][0]*m[2][1])
            -
            (m[2][0]*m[1][1]*m[0][2]
            +m[0][0]*m[2][1]*m[1][2]
            +m[0][1]*m[1][0]*m[2][2]);
}

template <typename T>
inline
T determinant(const mat4<T>& m)
{
    return  (m[0][0]*m[1][1]*m[2][2]*m[3][3]
            +m[0][1]*m[1][2]*m[2][3]*m[3][0]
            +m[0][2]*m[1][3]*m[2][0]*m[3][1]
            +m[0][3]*m[1][0]*m[2][1]*m[3][2])
            -
            (m[3][0]*m[2][1]*m[1][2]*m[0][3]
            +m[2][0]*m[1][1]*m[0][2]*m[3][3]
            +m[1][0]*m[0][1]*m[3][2]*m[2][3]
            +m[0][0]*m[3][1]*m[2][2]*m[1][3]);
}

template <typename T>
inline mat3<T> rotatex(const T& degrees)
{
    const T rad = degrees * M_PI / 180.0;
    T ca = cos(rad);
    T sa = sin(rad);
    return mat3<T>(1, 0, 0,
                   0, ca, -sa,
                   0, sa, ca );
}

//template <typename T>
//inline mat4<T> rotatex(const T& degrees)
//{
//    const T rad = degrees * M_PI / 180.0;
//    T ca = cos(rad);
//    T sa = sin(rad);
//    return mat4<T>(1, 0, 0, 0,
//                   0, ca, -sa, 0,
//                   0, sa, ca, 0,
//                   0, 0, 0, 1);
//}

template <typename T>
inline mat3<T> rotatey(const T& degrees)
{
    const T rad = degrees * M_PI / 180.0;
    T ca = cos(rad);
    T sa = sin(rad);
    return mat3<T>(ca, 0, sa,
                   0, 1, 0,
                   -sa, 0, ca);
}

//template <typename T>
//inline mat4<T> rotatey(const T& degrees)
//{
//    const T rad = degrees * M_PI / 180.0;
//    T ca = cos(rad);
//    T sa = sin(rad);
//    return mat4<T>(ca, 0, sa, 0,
//                   0, 1, 0, 0,
//                   -sa, 0, ca, 0,
//                   0, 0, 0, 1);
//}

template <typename T>
inline mat3<T> rotatez(const T& degrees)
{
    const T rad = degrees * M_PI / 180.0;
    T ca = cos(rad);
    T sa = sin(rad);
    return mat3<T>(ca, -sa, 0,
                   sa, ca, 0,
                   0, 0, 1  );
}

//template <typename T>
//inline mat4<T> rotatez(const T& degrees)
//{
//    const T rad = degrees * M_PI / 180.0;
//    T ca = cos(rad);
//    T sa = sin(rad);
//    return mat4<T>(ca, -sa, 0, 0,
//                   sa, ca, 0, 0,
//                   0, 0, 1, 0,
//                   0, 0, 0, 1);
//}

template <typename T>
inline
mat4<T> perspective(const T& fovy, const T& aspect, const T& zNear, const T& zFar)
{
    T f = T(1) / tan(fovy * M_PI / 360.0);
    return mat4<T>(f / aspect,  0, 0,                         0,
                      0,        f, 0,                         0,
                      0,        0, (zFar + zNear) / (zNear - zFar), (2.0*zFar*zNear) / (zNear - zFar),
                      0,        0, -1.0,                        0);
}

template <typename T>
inline
mat4<T> frustum(const T& left, const T& right, const T& bottom, const T& top, const T& nearVal, const T& farVal)
{
    assert(nearVal > 0 && farVal > 0);
    assert(left != right && bottom != top && nearVal != farVal);

    const T A = (right + left)/(right - left);
    const T B = (top + bottom)/(top - bottom);
    const T C = -(farVal + nearVal)/(farVal - nearVal);
    const T D = -(T(2)*farVal*nearVal)/(farVal - nearVal);

    const T m00 = T(2)*nearVal / (right-left);
    const T m11 = T(2)*nearVal / (top-bottom);

    return mat4<T>(m00, 0, A, 0,
                   0, m11, B, 0,
                   0, 0,   C, D,
                   0, 0,  -1, 0);
}

template <typename T>
inline
mat4<T> ortho(const T& left, const T& right, const T& bottom, const T& top, const T& nearVal, const T& farVal)
{
    assert(left != right && bottom != top && nearVal != farVal);

    const T tmp1 = T(1)/(right-left);
    const T tmp2 = T(1)/(top-bottom);
    const T tmp3 = T(1)/(farVal-nearVal);

    const T tx = -(right+left)*tmp1;
    const T ty = -(top+bottom)*tmp2;
    const T tz = -(farVal+nearVal)*tmp3;

    const T m00 = T(2)*tmp1;
    const T m11 = T(2)*tmp2;
    const T m22 = T(-2)*tmp3;

    return mat4<T>(m00, 0, 0, tx,
                   0, m11, 0, ty,
                   0, 0, m22, tz,
                   0, 0,   0,  1 );
}

#endif // __G_MATH__
