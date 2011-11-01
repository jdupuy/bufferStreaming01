////////////////////////////////////////////////////////////////////////////////
// \file    gmath.hpp
// \author  Jonathan Dupuy
// \brief   Basic math library extension for computer graphics.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef GMATH_EXT_HPP
#define GMATH_EXT_HPP

//#include "gmath.hpp" // basic types

////////////////////////////////////////////////////////////////////////////////
// Type declarations
template <typename T>    class plane;
typedef plane<float>     planef;
typedef plane<double>    planed;
typedef plane<int>       planei;
typedef plane<unsigned>  planeu;

template <typename T>   class aabb;   // TODO!
template <typename T>   class ray;    // TODO!
template <typename T>   class sphere; // TODO!


////////////////////////////////////////////////////////////////////////////////
// Function declarations
// plane
template <typename T>
	T distance(const plane<T>& p, const vec3<T>& v); // distance point/plane

template <typename T>
	plane<T> normalize(const plane<T>& p); // normalize a plane


////////////////////////////////////////////////////////////////////////////////
// Plane definition
template <typename T>
class plane
{
public:
	// Constructors
	plane();
	plane(const T& a, const T& b, const T& c, const T& d);            // explicit
	plane(const vec3<T>& v0, const vec3<T>& v1, const vec3<T>& v2);   // 3 points
	plane(const vec3<T>& v, const vec3<T>& n);                        // point and normalized normal
	template <typename U>
	explicit plane(const plane<U>& p);

	// Member access
	const T& operator[](size_t i)   const;
	T& operator[](size_t i);

	// Members
	T a, b, c, d; // Components of the plane equation
};

template <typename T>
inline
plane<T>::plane()
{

}

template <typename T>
inline
plane<T>::plane(const T& a, const T& b, const T& c, const T& d) : 
	a(a), b(b), c(c), d(d)
{
}

template <typename T>
inline 
plane<T>::plane(const vec3<T>& v0, const vec3<T>& v1, const vec3<T>& v2)
{
	vec3<T> v1v0 = v0-v1;
	vec3<T> v1v2 = v2-v1;
	vec3<T> n = normalize(cross(v1v0, v1v2));

	a = n.x;
	b = n.y;
	c = n.z;
	d = -dot(n, v0);
}

template <typename T>
inline 
plane<T>::plane(const vec3<T>& v, const vec3<T>& n)
{
	a = n.x;
	b = n.y;
	c = n.z;
	d = -dot(n, v);
}

template <typename T>
template <typename U>
inline
plane<T>::plane(const plane<U>& p) : 
	a(T(p[0])), b(T(p[1])), c(T(p[2])), d(T(p[3]))
{

}

template <typename T>
inline
T& plane<T>::operator[] (size_t i)
{
	assert(i < 4);
	return (&a)[i];
}

template <typename T>
inline
const T& plane<T>::operator[] (size_t i)   const
{
	assert(i < 4);
	return (&a)[i];
}


////////////////////////////////////////////////////////////////////////////////
// Function implementations
template <typename T>
inline
T distance(const plane<T>& p, const vec3<T>& v)
{
	return p[0]*v[0] + p[1]*v[1] + p[2]*v[2] + p[3];
}

template <typename T>
inline
plane<T> normalize(const plane<T>& p)
{
	T mag = sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] + p[3]*p[3]);
	plane<T> result;
	result.a = p[0]/mag;
	result.b = p[1]/mag;
	result.c = p[2]/mag;
	result.d = p[3]/mag;

	return result;
}

////////////////////////////////////////////////////////////////////////////////

#endif // __G_MATH_EXT__

