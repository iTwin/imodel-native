#pragma once

namespace pt
{
namespace datatree
{

//! MultiTypeArray
//! Array of single values of potentially differing types
class Array
{
public:

	friend Mem;

	template <typename Vector>
	static Array *createSingleType( Vector &values, int num_elems )
	{
		Array *arr= Mem::newArray();
		if (arr)
		{
			arr->allocateSingleType( sizeof(values[0]), num_elems );
			for (int i=0; i<num_elems; i++) arr->setValue(i, values[i]);
		}
		return arr;
	}

	template <typename Vector>
	static Array *createMultiType( Vector &values, int num_elems )
	{
		Array *arr= Mem::newArray();
		if (arr)
		{
			arr->allocateMultiType( sizeof(values[0]), num_elems );
			for (int i=0; i<num_elems; i++) arr->setValue(i, values[i]);
		}
		return arr;
	}

	int		numValues() const		{ return _numElems; }

	// set single value (values can be of different types but same size)
	template <class T> bool setValue(int index, const T &v) 
	{
		assert(! _isVariantArray );
		if (_isVariantArray) return false;

		assert ( sizeof(v) == _typesize );
		if (sizeof(v) != _typesize || index < 0 || index >=_numElems) 
			return false;

		memcpy( _values[index], &v, _typesize );	// not exactly type safe!, should not be used for variants
		return true;
	}
	// set single value of variant type
	template <>
	bool setValue(int index, const Variant &v )
	{
		assert( _isVariantArray );
		if (!_isVariantArray || index >=_numElems || index <0) return false;

		try {
			getVariant( index ) = v;
			return true;
		}
		catch(...) // type error
		{
			return false;
		}
	}

	// get single value
	template <class T> bool getValue(int index, T &v) const
	{
		if (_isVariantType)
		{
			if (index > 0 && index < _numElems)
			{
				try {
					v = ttl::var::get<T>( getVariant(index) );
				}
				catch(...) // wrong type
				{
					return false;
				}
			}
			else return false;
		}
		else
		{
			v = memcpy( &v, _values[index], _typesize );
			return true;
		}
	}
private:
	Array() : _values(0) {}
	~Array()
	{
		if (_values) Mem::release( (uint8*)_values );
	}
	bool	allocateSingleType( int typesize, int num_elems )
	{
		if (_values) return false;

		try 
		{
			_values = Mem::alloc( typesize * num_elems );
			_typesize = typesize;		
			_numElems = num_elems;
			memset(_values, 0, typesize * num_elems );
			_isVariantArray = false;
		}
		catch( std::bad_alloc )	{ return false; }
	}

	bool	allocateMultiType( int typesize, int num_elems )
	{
		if (_values) return false;

		try 
		{
			Variant *variantArray = Mem::newVariant(num_elems);
			_typesize = typesize;		
			_values = (void*)variantArray;
			_numElems = num_elems;
			_isVariantArray = true;
		}
		catch( std::bad_alloc )	{ return false; }
	}

	Variant		&getVariant( int index )	// not safe, calling function must check isVariant array + bounds of in
	{
		return reinterpret_cast<Variant*>(_values)[index];
	}
	const Variant	&getVariant( int index ) const	// not safe, calling function must check isVariant array + bounds of in
	{
		return reinterpret_cast<const Variant*>(_values)[index];
	}

	friend		Mem;

	bool		_isVariantArray;
	int			_numElems;
	int			_typesize;
	void*		_values;
};
} // namespace datatree
} // namespace pt