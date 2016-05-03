#ifndef POINTOOLS_MATRIX4_DEF
#define POINTOOLS_MATRIX4_DEF
#include <pt/classes.h>
#include <pt/geomtypes.h>
namespace pt
{
/*--------------------------------------------------------------------------*/ 
/* matrix4																	*/ 
/*--------------------------------------------------------------------------*/ 
struct CCLASSES_API matrix4
{
	float _data[16];
	/*transformation*/ 
	static matrix4 xRotationM(const float &theta);
	static matrix4 yRotationM(const float &theta);
	static matrix4 zRotationM(const float &theta);
	static matrix4 zyxRotationM(const vector3 &v);
	static matrix4 scaleM(const vector3 &v);
	static matrix4 translationM(const vector3 &v);
	static matrix4 transformM(const vector3 &sc, const vector3 &rt, const vector3 &tr);

	inline void transform(const vector3 &v, vector3 &t) const
	{
		t.x = (*this)(0,0) * v[0] + (*this)(1,0) * v[1] + (*this)(2,0) * v[2] + (*this)(3,0);
		t.y = (*this)(0,1) * v[0] + (*this)(1,1) * v[1] + (*this)(2,1) * v[2] + (*this)(3,1);
		t.z = (*this)(0,2) * v[0] + (*this)(1,2) * v[1] + (*this)(2,2) * v[2] + (*this)(3,2);
	}
	/*data filling*/ 
	void zero() { for (int i=0; i<16; i++) _data[i] = 0; };
	void fill(float v) { for (int i=0; i<16; i++) _data[i] = v; };
	void identity()
	{
		for (unsigned int j = 0; j < 4; j++)
			for (unsigned int i = 0; i < 4; i++)
				(*this)(i,j) = (i==j) ? 1.0f : 0;
	}
	/* Indexing: format = i down, j across*/ 
	inline	const	float &operator()(unsigned int i, unsigned int j) const { return _data[j*4+i]; }
	inline  float &operator()(const unsigned int i, const unsigned int j) { return _data[j*4+i]; }

	/*matrix concatenation*/ 
	matrix4	operator >>(const matrix4 &m) const	{ return concat(m); }
	void	operator >>=(const matrix4 &m)		{ *this = concat(m); }
	matrix4	concat(const matrix4 &m) const;

	/*functions*/ 
	void invert();
	float determinant() const;

	/*assignment*/ 
	const matrix4 &operator = (const matrix4 &m)
	{
		memcpy(_data, m._data, sizeof(float)*16);
		return (*this);
	}
};
}
#endif