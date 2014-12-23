/*--------------------------------------------------------------------------*/ 
/*  Bounds.h																*/ 
/*	Linear bounds definition												*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef CCOMMONCLASSESBOUNDS_INTERFACE
#define CCOMMONCLASSESBOUNDS_INTERFACE

namespace pt
{
template <int B, class T> class Bounds
{
public:
	Bounds<B,T>()
	{
		for (int i=0; i<B*2; i++) _usebound[i] = true;
		_empty = true;
	}
	~Bounds() {};

	void makeEmpty() { _empty = true; }

	typedef T value_type;

	void set(const T *lower, const T*upper)
	{
		for (int i=0; i<B; i++)
		{
			upper_bounds[i] = upper[i];
			lower_bounds[i] = lower[i];
		}
		_empty = false;
	}
	bool valid() const
	{
		bool sz = false;
		for (int i=0; i<B; i++)
		{
			if (upper_bounds[i] < lower_bounds[i]) return false;
			if (!sz && lower_bounds[i] != upper_bounds[i]) sz = true;
		}
		return sz;
	}
	void makeValid()
	{
		T temp;
		for (int i=0; i<B; i++)
		{
			if (lower_bounds[i] > upper_bounds[i]) 
			{
				temp = upper_bounds[i];
				upper_bounds[i] = lower_bounds[i];
				lower_bounds[i] = temp;
			}
		}
	}

	void centralize(void)
	{
											// Translates bounds so that center is the origin
											// while maintaining its existing size
		float center[3];
		getCenter(center);

		upper_bounds[0] -= center[0];
		lower_bounds[0] -= center[0];

		upper_bounds[1] -= center[1];
		lower_bounds[1] -= center[1];

		upper_bounds[2] -= center[2];
		lower_bounds[2] -= center[2];
	}

	void normalizedValue(const T v[B], double n[B]) const
	{
		for (int i=0; i<B; i++)
		{
			if(size(i) > 0)
			{
				n[i] = (v[i] - lower_bounds[i]) / size(i);
			}
			else
			{
				n[i] = 0;
			}
		}
	}
	void normalizedValue(const T v[B], double n[B], double rangemin, double rangemax) const
	{
		for (int i=0; i<B; i++)
		{
			if(size(i) > 0)
			{
				n[i] = v[i];
				n[i] -= lower_bounds[i];
				n[i] /= size(i);
				n[i] *= rangemax - rangemin;
				n[i] += rangemin;
			}
			else
			{
				n[i] = rangemax;
			}
		}
	}
	void expand(const T v[B])
	{
		if (!_empty)
		{
			for (int i=0; i<B; i++)
			{
				if (v[i] > upper_bounds[i]) upper_bounds[i] = v[i];
				if (v[i] < lower_bounds[i]) lower_bounds[i] = v[i];
			}
		}
		else
		{
			for (int i=0; i<B; i++)
			{
				upper_bounds[i] = v[i]; 
				lower_bounds[i] = v[i];
			}
			_empty = false;
		}
	}
	inline int getNumBounds() const { return B; };
	inline bool inBounds(const T *v) const
	{
		for (int i=0; i<B; i++)
		{
			if (_usebound[i] && v[i] > upper_bounds[i]) return false;
			if (_usebound[i] && v[i] < lower_bounds[i]) return false;
		}
		return true;
	}
	inline const T &lower(int i) const { return lower_bounds[i]; }
	inline const T &upper(int i) const { return upper_bounds[i]; }
	inline bool intersects(const Bounds<B,T> *b) const
	{
		for (int i=0; i<B; i++)
		{
			if (lower_bounds[i] < b->lower(i) && upper_bounds[i] < b->lower(i)
				|| lower_bounds[i] > b->upper(i) && upper_bounds[i] > b->upper(i))
				return false;
		}
		return true;
	}
	inline bool contains(const Bounds<B,T> *b) const
	{
		for (int i=0; i<B; i++)
		{
			if (lower_bounds[i] > b->lower(i) || upper_bounds[i] < b->upper(i))
				return false;
		}
		return true;
	}
	inline T size(int dim) const { return upper_bounds[dim] - lower_bounds[dim]; }
	inline void size(T* dim) const 
	{ 
		for (int i=0; i<B; i++)	dim[i] = upper_bounds[i] - lower_bounds[i]; 
	}
	bool isEmpty() const { return _empty; }

	inline T mid(int i) const 
	{ 
		return lower_bounds[i] + (upper_bounds[i] - lower_bounds[i]) / (T)2.0; 
	};

	void getCenter(T *v) const
	{
		for (int i=0; i<B; i++)
			v[i] = (upper_bounds[i] + lower_bounds[i]) / (T)2;
	}
	void translateBy(const T* v)
	{
		for (int i=0; i<B; i++)
		{
			upper_bounds[i] += v[i];
			lower_bounds[i] += v[i];
		}
	}
	void invtranslateBy(const T* v)
	{
		for (int i=0; i<B; i++)
		{
			upper_bounds[i] -= v[i];
			lower_bounds[i] -= v[i];
		}
	}
	void centerAt(const T* v)
	{
		T c[B];
		memcpy(c, lower_bounds, sizeof(T)*B);

		for (int i=0; i<B; i++)
		{
			c[i] += upper_bounds[i];
			c[i] /= (T)2;
			c[i] = v[i] - c[i];
		}
		translateBy(c);
	}
	void setUseBound(bool use, bool upper, int idx)
	{
		_usebound[idx + B * (upper ? 1 : 0)] = use;
	}
    inline void getExtrema(unsigned int pos, T*v) const
    {
		for (int i=0; i<B; i++)
			v[i] = (unsigned char)pos & (1 << i) ? upper_bounds[i] : lower_bounds[i];
    }
protected:
	bool _empty;
	bool _usebound[B*2];
	T upper_bounds[B];
	T lower_bounds[B];
};
}
#endif

