
#pragma once

#include <PTRMI/Status.h>
#include <PTRMI/DataBuffer.h>

#include <map>
#include <string.h>
#include <sstream>

#define VARIANT_TYPE_NAME_GROUP				L"Group"
#define VARIANT_TYPE_NAME_CHAR				L"Char"
#define VARIANT_TYPE_NAME_UNSIGNED_CHAR		L"UChar"
#define VARIANT_TYPE_NAME_SHORT				L"Short"
#define VARIANT_TYPE_NAME_UNSIGNED_SHORT	L"UShort"
#define VARIANT_TYPE_NAME_LONG				L"Long"
#define VARIANT_TYPE_NAME_UNSIGNED_LONG		L"ULong"
#define VARIANT_TYPE_NAME_INT				L"Int"
#define VARIANT_TYPE_NAME_UNSIGNED_INT		L"UInt"
#define VARIANT_TYPE_NAME_FLOAT				L"Float"
#define VARIANT_TYPE_NAME_DOUBLE			L"Double"
#define VARIANT_TYPE_NAME_BOOL				L"Bool"
#define VARIANT_TYPE_NAME_WSTRING			L"String"

#define VARIANT_BOOL_TRUE					L"true"
#define VARIANT_BOOL_FALSE					L"false"

#define VARIANT_GRAPH_READ_BUFFER_SIZE		8192

#define VARIANT_READ_COMMENT_BLOCK_START	L"/*"
#define VARIANT_READ_COMMENT_BLOCK_END		L"*/"
#define VARIANT_READ_COMMENT_LINE_START		L"//"


namespace PTRMI
{

template<typename VT> class Variant
{

public:

	typedef size_t		DefaultIntType;

	typedef std::wstring		VariantKeyName;
	typedef std::wstring		VariantTypeName;

	typedef VT					VariantType;

	typedef	Variant<VT>			thisType;

	typedef size_t		DependencyCounter;

public:

	const static DefaultIntType	variantDefaultIntTypeGroup				= 1;
	const static DefaultIntType	variantDefaultIntTypeChar				= 2;
	const static DefaultIntType	variantDefaultIntTypeUnsignedChar		= 3;
	const static DefaultIntType	variantDefaultIntTypeShort				= 4;
	const static DefaultIntType	variantDefaultIntTypeUnsignedShort		= 5;
	const static DefaultIntType	variantDefaultIntTypeLong				= 6;
	const static DefaultIntType	variantDefaultIntTypeUnsignedLong		= 7;
	const static DefaultIntType	variantDefaultIntTypeInt				= 8;
	const static DefaultIntType	variantDefaultIntTypeUnsignedInt		= 9;
	const static DefaultIntType	variantDefaultIntTypeFloat				= 10;
	const static DefaultIntType	variantDefaultIntTypeDouble				= 11;
	const static DefaultIntType	variantDefaultIntTypeBool				= 12;
	const static DefaultIntType	variantDefaultIntTypeWString			= 13;

protected:

	DependencyCounter			dependencyCounter;

public:

								Variant						(void)						{setDependencyCounter(0);}
	virtual					   ~Variant						(void)						{}

	virtual		bool			copy						(thisType *variant)			= 0;

	void						setDependencyCounter		(DependencyCounter count)	{dependencyCounter = count;}
	DependencyCounter			getDependencyCounter		(void)						{return dependencyCounter;}
	DependencyCounter			incrementDependencyCounter	(void)						{setDependencyCounter(getDependencyCounter() + 1); return getDependencyCounter();}
	DependencyCounter			decrementDependencyCounter	(void)						{setDependencyCounter(getDependencyCounter() - 1); return getDependencyCounter();}

	virtual		VariantType	&	getType						(void) const				= 0;

	virtual		void			read						(DataBuffer &buffer)		= 0;
	virtual		void			write						(DataBuffer &buffer) const	= 0;

	virtual		VariantType		readType					(DataBuffer &buffer)		= 0;
	virtual		void			writeType					(DataBuffer &buffer) const	= 0;

				void			getIndents					(std::wostringstream &string, unsigned int indent = 0) const;

	virtual		void			getString					(std::wostringstream &string, unsigned int indent = 0)		= 0;
	virtual		void			getTypeString				(std::wostringstream &string)	= 0;
	virtual		void			readString					(std::wifstream &stream)		= 0;

	template<typename V> static thisType *	newVariant		(void) {return new V;}

	template<typename V> static void readVariant(std::wifstream &in, Variant<VT> *v)
	{
		V *variant;

		if(variant = dynamic_cast<V *>(v))
		{
			variant->readString(in);
		}
	}

	template<typename V> bool copyTyped(Variant<VT> *variant)
	{
		V *	v		= dynamic_cast<V *>(variant);
		V *	vThis	= dynamic_cast<V *>(this);

		if(v && vThis)
		{
			*vThis = *v;

			setDependencyCounter(0);

			return true;
		}

		return false;
	}


};


template<typename T, typename VT> class VariantT : public Variant<VT>
{

public:

    typedef Variant<VT>         Base;

	typedef T					Type;

	typedef VariantT<T, VT>		thisType;


protected:

	Type						value;

protected:

	static typename Base::VariantType			type;
	static typename Base::VariantTypeName		typeName;

public:
								VariantT	(void)							{}
								VariantT	(Type &v)						{set(v);}

 	void						set			(const Type &v)					{value = v;}
 	Type					&	get			(void)							{return value;} const

	static void					setType		(typename Base::VariantType initType)			{type = initType;}
    typename Base::VariantType&	getType		(void) const					{return type;}

	static void					setTypeName	(const Base::VariantTypeName &initTypeName)	{typeName = initTypeName;}
	const Base::VariantTypeName& getTypeName	(void)						{return typeName;}

	static typename Base::VariantType& getTypeS	(void)						{return type;}

 	void						read		(DataBuffer &buffer)			{buffer >> value;}
 	void						write		(DataBuffer &buffer) const		{buffer << value;}

	VT							readType	(DataBuffer &buffer)			{VT t; buffer >> t; return t;}
	void						writeType	(DataBuffer &buffer) const		{buffer << getType();}


	void getTypeString(std::wostringstream &string)
	{
		string << getTypeName().c_str();
	}

	void getString(std::wostringstream &string, unsigned int indent)
	{
		string << get();
	}

};


template<typename T, typename VT> class VariantInteger : public VariantT<T, VT>
{
public:

	VariantInteger(void)	: VariantT<T, VT>() {}
	VariantInteger(T v)		: VariantT<T, VT>(v) {}

	void readString(std::wifstream &stream)
	{
		int64_t	v;

		stream >> v;

		value = static_cast<T>(v);
	}
};


template<typename T, typename VT> class VariantReal : public VariantT<T, VT>
{
public:

	VariantReal(void)	: VariantT<T, VT>() {}
	VariantReal(T v)	: VariantT<T, VT>(v) {}

	void readString(std::wifstream &stream)
	{
		double v;

		stream >> v;

		value = static_cast<T>(v);
	}
};


template<typename VT> class VariantChar : public VariantInteger<char, VT>
{
public:

	typedef VariantChar		thisType;

public:

	VariantChar		(void) : VariantInteger<char, VT>()			{}
	VariantChar		(char c) : VariantInteger<char, VT>(v)		{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}
};


template<typename VT> class VariantUnsignedChar : public VariantInteger<unsigned char, VT>
{
public:

	typedef VariantUnsignedChar		thisType;

public:
	VariantUnsignedChar(void)				: VariantInteger<unsigned char, VT>()		{}
	VariantUnsignedChar(unsigned char v)	: VariantInteger<unsigned char, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};

template<typename VT> class VariantInt : public VariantInteger<int, VT>
{
public:

	typedef VariantInt	thisType;

public:
	VariantInt(void)	: VariantInteger<int, VT>()	{}
	VariantInt(int v)	: VariantInteger<int, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};

template<typename VT> class VariantUnsignedInt : public VariantInteger<unsigned int, VT>
{
public:

	typedef VariantUnsignedInt	thisType;

public:
	VariantUnsignedInt(void)			: VariantInteger<unsigned int, VT>()	{}
	VariantUnsignedInt(unsigned int v)	: VariantInteger<unsigned int, VT>(v) {}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};



template<typename VT> class VariantShort : public VariantInteger<short, VT>
{
public:

	typedef VariantShort	thisType;

public:
	VariantShort(void)		: VariantInteger<short, VT>()		{}
	VariantShort(short v)	: VariantInteger<short, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};

template<typename VT> class VariantUnsignedShort : public VariantInteger<unsigned short, VT>
{
public:

	typedef VariantUnsignedShort	thisType;

public:
	VariantUnsignedShort(void)				: VariantInteger<unsigned short, VT>()	 {}
	VariantUnsignedShort(unsigned short v)	: VariantInteger<unsigned short, VT>(v) {}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};


template<typename VT> class VariantLong : public VariantInteger<long, VT>
{
public:

	typedef VariantLong		thisType;

public:
	VariantLong(void)	: VariantInteger<long, VT>()	{}
	VariantLong(long v) : VariantInteger<long, VT>(v) {}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};

template<typename VT> class VariantUnsignedLong : public VariantInteger<unsigned long, VT>
{
public:

	typedef VariantUnsignedLong		thisType;

public:
	VariantUnsignedLong(void)				: VariantInteger<unsigned long, VT>()		{}
	VariantUnsignedLong(unsigned long v)	: VariantInteger<unsigned long, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};



template<typename VT> class VariantFloat : public VariantReal<float, VT>
{
public:

	typedef VariantFloat		thisType;

public:
	VariantFloat(void)		: VariantReal<float, VT>()		{}
	VariantFloat(float v)	: VariantReal<float, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};


template<typename VT> class VariantDouble : public VariantReal<double, VT>
{
public:

	typedef VariantDouble	thisType;

public:
	VariantDouble(void)		: VariantReal<double, VT>()	{}
	VariantDouble(double v) : VariantReal<double, VT>(v)	{}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

};


template<typename VT> class VariantBool : public VariantT<bool, VT>
{
public:

	typedef VariantBool		thisType;

public:
	VariantBool(void)	: VariantT<bool, VT>()	{}
	VariantBool(bool v) : VariantT<bool, VT>(v) {}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

	void getString(std::wostringstream &string, unsigned int indent)
	{
		if(this->get())
		{
			string << VARIANT_BOOL_TRUE;
		}
		else
		{
			string << VARIANT_BOOL_FALSE;
		}
	}

	void readString(std::wifstream &in)
	{
		std::wstring	v;
		bool			val = false;

		in >> v;

		if(v == VARIANT_BOOL_TRUE)
		{
			val = true;
		}
		else
		if(v == VARIANT_BOOL_FALSE)
		{
			val = false;
		}

        this->set(val);
	}
};


template<typename VT> class VariantWString : public VariantT<std::wstring, VT>
{
public:

	typedef VariantWString	thisType;

public:
	VariantWString(void)			: VariantT<std::wstring, VT>()	{}
	VariantWString(std::wstring &v) : VariantT<std::wstring, VT>(v) {}

	bool copy(Variant<VT> *variant)
	{
		return this->copyTyped<thisType>(variant);
	}

	void getString(std::wostringstream &string, unsigned int indent)
	{
		string << "\"";

		string << get();

		string <<"\"";
	}

	void readString(std::wifstream &stream)
	{
		stream >> value;

		unsigned int	length	= value.size();
		unsigned int	start	= 0;
		unsigned int	end		= length - 1;
															// Strip off start and end quotes if present
		if(length >= 2)
		{
			if(value[start] == L'\"')
			{
				++start;
			}

			if(value[end] == L'\"')
			{
				--end;
			}

			value = value.substr(start, end - start + 1);
		}
	}


};


template<typename VT> class VariantMeta
{
public:

	typedef	VT										VariantType;
	typedef typename Variant<VT>::VariantTypeName	VariantTypeName;

	typedef	PTRMI::Variant<VT> *(CDECL_ATTRIBUTE *VariantMethodNew)(void);

protected:

	VariantType				type;
	VariantTypeName			typeName;

	VariantMethodNew		variantMethodNew;

public:

	void					setType			(VariantType &initType)				{type = initType;}
	VariantType			&	getType			(void)								{return type;}

	void					setTypeName		(const VariantTypeName &initTypeName)		{typeName = initTypeName;}
	const VariantTypeName &	getTypeName		(void)								{return typeName;}

	void					setMethodNew	(VariantMethodNew method)			{variantMethodNew = method;}
	VariantMethodNew		getMethodNew	(void)								{return variantMethodNew;}
};


template<typename K, typename VT = std::wstring> class VariantGraph
{

public:

	typedef VariantChar<VT>										VGVariantChar;
	typedef VariantUnsignedChar<VT>								VGVariantUnsignedChar;
	typedef VariantInt<VT>										VGVariantInt;
	typedef VariantUnsignedInt<VT>								VGVariantUnsignedInt;
	typedef VariantShort<VT>									VGVariantShort;
	typedef VariantUnsignedShort<VT>							VGVariantUnsignedShort;
	typedef VariantLong<VT>										VGVariantLong;
	typedef VariantUnsignedLong<VT>								VGVariantUnsignedLong;
	typedef VariantFloat<VT>									VGVariantFloat;
	typedef VariantDouble<VT>									VGVariantDouble;
	typedef VariantBool<VT>										VGVariantBool;
	typedef VariantWString<VT>									VGVariantWString;

public:

	typedef		K												Key;
	typedef		std::vector<Key>								KeyPath;
	typedef		VT												VariantType;
	typedef		std::vector<Variant<VT> *>						VariantSet;

	typedef		VariantGraph<K, VT>								thisType;

	typedef		typename Variant<VT>							VariantTyped;

	typedef		typename VariantTyped::VariantKeyName			VariantKeyName;
	typedef		typename VariantTyped::VariantTypeName			VariantTypeName;

protected:

	typedef		std::map<Key, Variant<VT> *>					KeyVariantMap;


	typedef		typename VariantSet::iterator					Iterator;
	typedef		typename VariantSet::const_iterator				ConstIterator;

	typedef		typename VariantMeta<VT>::VariantMethodNew		VariantMethodNew;

	typedef		std::map<VT, VariantMeta<VT>>					VariantTypeMetaMap;
	typedef		std::map<VariantTypeName, VariantMeta<VT> *>	VariantTypeNameMetaMap;

	class Group : public VariantT<thisType, VT>
	{
	public:

		typedef Group	thisType;

	public:

		bool copy(Variant<VT> *variant)
		{
			Group *v = dynamic_cast<Group *>(variant);

			if(v)
			{
				return get().deepCopy(v->get());
			}

			return false;
		}

		void getString(std::wostringstream &string, unsigned int indent)
		{
			get().getString(string, indent);
		}

		void readString(std::wifstream &in)
		{
			get().readGroup(in);
		}
	};

protected:

	static	bool					variantGraphInitialized;

			VariantSet				variants;
			KeyVariantMap			keyVariants;
	static	VariantTypeMetaMap		variantTypeMeta;
	static	VariantTypeNameMetaMap	variantTypeNameMeta;

protected:

	VariantMeta<VT>	*	newVariantMeta			(VT type);
	VariantMeta<VT> *	getVariantMeta			(VT type);
	VariantMeta<VT>	*	getVariantMeta			(VariantTypeName &typeName);

	bool				addVariantNameMeta		(VT type, const VariantTypeName &typeName);

	bool				clear					(VT type);

	Variant<VT> 	*	newVariant				(VT type);
	Variant<VT> 	*	newVariant				(VariantTypeName &typeName);
	Variant<VT> 	*	createAndReadVariant	(DataBuffer &buffer, VT type);
	Variant<VT> 	*	createAndReadVariant	(std::wifstream &stream, VariantKeyName &keyName, VariantTypeName &typeName);

	bool				readGroup				(std::wifstream &in);
	bool				readSymbol				(std::wifstream &in, std::wstring &symbol);

	virtual void		setInitialized			(bool init)		{variantGraphInitialized = init;}
	virtual bool		getInitialized			(void)			{return variantGraphInitialized;}


public:

						VariantGraph				(void);
					   ~VariantGraph				(void);

	template<typename V> VariantMeta<VT> *addType(VT type, typename const Variant<VT>::VariantTypeName &typeName)
	{
		VariantMeta<VT> *	meta;

		V::setType(type);
		V::setTypeName(typeName);

		if((meta = getVariantMeta(type)) == NULL)
		{
			newVariantMeta(type);

			addVariantNameMeta(type, typeName);

			if((meta = getVariantMeta(type)) == NULL)
			{
				return meta;
			}
		}

		meta->setType(type);
		meta->setTypeName(typeName);

		meta->setMethodNew(&V::newVariant<V>);

		return meta;
	}

	void addExisting(Key &key, Variant<VT> *variant)
	{
		if(variant)
		{
			variants.push_back(variant);
			keyVariants[key] = variant;

			variant->incrementDependencyCounter();
		}
	}

	template<typename V> Variant<VT> *add(Key &key, V *variant)
	{
		Variant<VT> *v;

		if(v = new V(*variant))
		{
			addExisting(key, v);

			return v;
		}

		return NULL;
	}


	template<typename V> Variant<VT> *add(Key &key, V &variant)
	{
		return add(key, &variant);
	}

	template<typename V> Variant<VT> *add(KeyPath &keyPathLocation, Key &key, V &variant, bool create = true)
	{
		KeyPath		p;

		p = keyPathLocation;

		p.push_back(key);

		return add(p, &variant);
	}


	template<typename V> Variant<VT> *add(KeyPath &keyPath, V &variant, bool create = true)
	{
		return add(keyPath, &variant, create);
	}


	template<typename V> Variant<VT> *add(KeyPath &keyPath, V *variant, bool create = true)
	{
		Status		status;
		Group	*	group;

		if(keyPath.size() == 0)
		{
			return NULL;
		}

		if(keyPath.size() == 1)
		{
			return add(keyPath.back(), *variant);
		}

		if(create)
		{
			group = addLocation(keyPath);
		}
		else
		{
			group = findLocation(keyPath);
		}

		if(group)
		{
			return group->get().add(keyPath.back(), *variant);
		}

		return NULL;
	}

	template<typename T> bool get(KeyPath &keyPath, T &value) const
	{
		VariantT<T, VT> *variant;
															// Find variant and dynamic cast to specified Variant class to see if it's the correct type
		if((variant = dynamic_cast<VariantT<T, VT> *>(find(keyPath))) != NULL)
		{
															// If it is, get the value
			value = variant->get();
															// Return OK
			return true;
		}
															// Return not found
		return false;
	}

	unsigned int					getNumItems			(void) const;

	Group						*	addGroup			(Key &key);
	Group						*	addGroup			(KeyPath &keyPath);
	Group						*	addLocation			(KeyPath &keyPath);
	Group						*	addGroup			(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd);

	bool							remove				(Key &key);

	Variant<VT> 				*	find				(Key &key) const;
	Variant<VT> 				*	find				(KeyPath &keyPath) const;
	Variant<VT> 				*	find				(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const;
	Group						*	findGroup			(Key &key) const;
	Group						*	findGroup			(KeyPath &keyPath) const;
	Group						*	findGroup			(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const;
	Group						*	findLocation		(KeyPath &keyPath) const;
	Group						*	findLocation		(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const;

	bool							getKey				(Variant<VT> *variant, Key &key) const;

	bool							deleteAll			(void);

	void							read				(DataBuffer &buffer);
	void							write				(DataBuffer &buffer) const;

	bool							readFile			(const wchar_t *file, std::wistringstream *string = NULL);
	bool							writeFile			(const wchar_t *file, std::wostringstream *string = NULL) const;

	void							getIndents			(std::wostringstream &string, unsigned int indent = 0) const;
	void							getString			(std::wostringstream &string, unsigned int indent = 0) const;

	virtual VariantGraph<K, VT>	&	operator=			(const VariantGraph<K, VT> &variantGraph);
	virtual	VariantGraph<K, VT> 	operator+			(const VariantGraph<K, VT> &variantGraph);
	bool							deepCopy			(const VariantGraph<K, VT> &variantGraph);

	void							getVariantIterators	(ConstIterator &begin, ConstIterator &end) const {begin = variants.begin(); end = variants.end();}
};


template<typename K, typename VT>
inline VariantGraph<K, VT> PTRMI::VariantGraph<K, VT>::operator+(const VariantGraph<K, VT> &variantGraph)
{
	VariantGraph<K, VT>	temp;

	temp.deepCopy(*this);

	temp.deepCopy(variantGraph);

	return temp;
}


template<typename K, typename VT>
inline VariantGraph<K, VT> &PTRMI::VariantGraph<K, VT>::operator=(const VariantGraph<K, VT> &variantGraph)
{
	deleteAll();

	deepCopy(variantGraph);

	return *this;
}

template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::deepCopy(const VariantGraph<K, VT> &variantGraph)
{
	ConstIterator	begin, end;
	ConstIterator	it;
	Variant<VT>	*	variant;
	Variant<VT>	*	newV;
	Key				key;
															// Get given VariantGraph's iterators
	variantGraph.getVariantIterators(begin, end);
															// For all Variants in the given VariantGraph
	for(it = begin; it != end; it++)
	{
															// Get variant
		if(variant = *it)
		{
															// Get variant's key
			if(variantGraph.getKey(variant, key))
			{
															// Create a new variant of the same type
				if(newV = newVariant(variant->getType()))
				{
					newV->copy(variant);
															// Add the new variant copy to this VariantGraph
					addExisting(key, newV);
				}
			}
		}
	}
															// Return whether all items were copied								
	return (getNumItems() == variantGraph.getNumItems());
}

template<typename K, typename VT>
inline VariantMeta<VT>	*PTRMI::VariantGraph<K, VT>::newVariantMeta(VT type)
{
	VariantMeta<VT>	meta;

	meta.setType(type);

	variantTypeMeta[type] = meta;

	return getVariantMeta(type);
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::addVariantNameMeta(VT type, const VariantTypeName &typeName)
{
	VariantMeta<VT>	*meta;

	if((meta = getVariantMeta(type)) != NULL)
	{
		variantTypeNameMeta[typeName] = meta;

		return true;
	}

	return false;
}


template<typename K, typename VT>
inline VariantMeta<VT>	*PTRMI::VariantGraph<K, VT>::getVariantMeta(VariantTypeName &typeName)
{
    typename VariantTypeNameMetaMap::iterator it;

	if((it = variantTypeNameMeta.find(typeName)) != variantTypeNameMeta.end())
	{
		return it->second;
	}

	return NULL;
}

template<typename K, typename VT>
inline VariantMeta<VT>	*PTRMI::VariantGraph<K, VT>::getVariantMeta(VT type)
{
	typename VariantTypeMetaMap::iterator it;

	if((it = variantTypeMeta.find(type)) != variantTypeMeta.end())
	{
		return &(it->second);
	}

	return NULL;
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::clear(VT type)
{
	variantTypeMeta.clear();
	variantTypeNameMeta.clear();

	variants.clear();

	return true;
}


template<typename K, typename VT>
inline PTRMI::VariantGraph<K, VT>::VariantGraph(void)
{
	if(getInitialized() == false)
	{
															// Register standard variant types using default integral type values and type names
		addType<Group>(VariantChar<VariantType>::variantDefaultIntTypeGroup, std::wstring(VARIANT_TYPE_NAME_GROUP));

		addType<VGVariantChar>(Variant<VariantType>::variantDefaultIntTypeChar, std::wstring(VARIANT_TYPE_NAME_CHAR));
		addType<VGVariantUnsignedChar>(Variant<VariantType>::variantDefaultIntTypeUnsignedChar, std::wstring(VARIANT_TYPE_NAME_UNSIGNED_CHAR));

		addType<VGVariantShort>(Variant<VariantType>::variantDefaultIntTypeShort, std::wstring(VARIANT_TYPE_NAME_SHORT));
		addType<VGVariantUnsignedShort>(Variant<VariantType>::variantDefaultIntTypeUnsignedShort, std::wstring(VARIANT_TYPE_NAME_UNSIGNED_SHORT));

		addType<VGVariantLong>(Variant<VariantType>::variantDefaultIntTypeLong, std::wstring(VARIANT_TYPE_NAME_LONG));
		addType<VGVariantUnsignedLong>(Variant<VariantType>::variantDefaultIntTypeUnsignedLong, std::wstring(VARIANT_TYPE_NAME_UNSIGNED_LONG));

		addType<VGVariantInt>(Variant<VariantType>::variantDefaultIntTypeInt, std::wstring(VARIANT_TYPE_NAME_INT));
		addType<VGVariantUnsignedInt>(Variant<VariantType>::variantDefaultIntTypeUnsignedInt, std::wstring(VARIANT_TYPE_NAME_UNSIGNED_INT));

		addType<VGVariantFloat>(Variant<VariantType>::variantDefaultIntTypeFloat, std::wstring(VARIANT_TYPE_NAME_FLOAT));
		addType<VGVariantDouble>(Variant<VariantType>::variantDefaultIntTypeDouble, std::wstring(VARIANT_TYPE_NAME_DOUBLE));

		addType<VGVariantBool>(Variant<VariantType>::variantDefaultIntTypeBool, std::wstring(VARIANT_TYPE_NAME_BOOL));

		addType<VGVariantWString>(Variant<VariantType>::variantDefaultIntTypeWString, std::wstring(VARIANT_TYPE_NAME_WSTRING));

		setInitialized(true);
	}
}


template<typename K, typename VT>
inline PTRMI::VariantGraph<K, VT>::~VariantGraph(void)
{
	deleteAll();
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::deleteAll(void)
{
	//Iterator		it;
	Key				key;
	Variant<VT> *	v;
	bool			ret = true;
															// For each variant
	while(variants.size() > 0)
	{
		if(v = variants[0])
		{
															// Delete variant and all indexing
			if(getKey(v, key))
			{
				remove(key);
			}
			else
			{
				ret = false;
			}
		}
		else
		{
			ret = false;
		}
	}
															// Return result
	return ret;
}


template<typename T, typename VT>
inline std::wostringstream& operator << (std::wostringstream &out, VariantGraph<T, VT> &variantMap)
{
	variantMap.getString(out);

	return out;
}


template<typename K, typename VT>
inline Variant<VT> * PTRMI::VariantGraph<K, VT>::newVariant(VT type)
{
	//Variant<VT>						*	v;
	VariantMeta<VT>					*	meta;
															// Get Variant type's Meta class
	if(meta = getVariantMeta(type))
	{
															// Get New method
		if(meta->getMethodNew())
		{
															// Execute New method
			return meta->getMethodNew()();
		}
	}
															// Error, return NULL
	return NULL;
}


template<typename K, typename VT>
inline Variant<VT> * PTRMI::VariantGraph<K, VT>::createAndReadVariant(DataBuffer &buffer, VT type)
{
	Variant<VT>	*	v;

	if((v = newVariant(type)) != NULL)
	{
		v->read(buffer);
	}

	return v;
}


template<typename K, typename VT>
inline Variant<VT> * PTRMI::VariantGraph<K, VT>::createAndReadVariant(std::wifstream &in, VariantKeyName &keyName, VariantTypeName &typeName)
{
	VT					type;
	VariantMeta<VT> *	meta;
	Variant<VT>		*	v = NULL;

	if(meta = getVariantMeta(typeName))
	{
		type = meta->getType();

		if((v = newVariant(type)) != NULL)
		{
			v->readString(in);
		}
	}

	return v;
}


template<typename K, typename VT>
inline unsigned int PTRMI::VariantGraph<K, VT>::getNumItems(void) const
{
	return variants.size();
}
									

#define VariantTypeStatics(VG)		VG::VariantType 	VariantChar<VG::VariantType>::type;					\
									VG::VariantType 	VariantUnsignedChar<VG::VariantType>::type;			\
									VG::VariantType 	VariantInt<VG::VariantType>::type;					\
									VG::VariantType 	VariantUnsignedInt<VG::VariantType>::type;			\
									VG::VariantType 	VariantShort<VG::VariantType>::type;				\
									VG::VariantType 	VariantUnsignedShort<VG::VariantType>::type;		\
									VG::VariantType 	VariantLong<VG::VariantType>::type;					\
 									VG::VariantType 	VariantUnsignedLong<VG::VariantType>::type;			\
 									VG::VariantType 	VariantFloat<VG::VariantType>::type;				\
 									VG::VariantType 	VariantDouble<VG::VariantType>::type;				\
									VG::VariantType 	VariantBool<VG::VariantType>::type;					\
 									VG::VariantType 	VariantWString<VG::VariantType>::type;				\
									VG::VariantType 	VG::Group::type;									\
									VG::VariantTypeName VariantChar<VG::VariantType>::typeName;				\
									VG::VariantTypeName VariantUnsignedChar<VG::VariantType>::typeName;		\
									VG::VariantTypeName VariantInt<VG::VariantType>::typeName;				\
									VG::VariantTypeName VariantUnsignedInt<VG::VariantType>::typeName;		\
									VG::VariantTypeName VariantShort<VG::VariantType>::typeName;			\
									VG::VariantTypeName VariantUnsignedShort<VG::VariantType>::typeName;	\
									VG::VariantTypeName VariantLong<VG::VariantType>::typeName;				\
									VG::VariantTypeName VariantUnsignedLong<VG::VariantType>::typeName;		\
									VG::VariantTypeName VariantFloat<VG::VariantType>::typeName;			\
									VG::VariantTypeName VariantDouble<VG::VariantType>::typeName;			\
									VG::VariantTypeName VariantBool<VG::VariantType>::typeName;				\
									VG::VariantTypeName VariantWString<VG::VariantType>::typeName;			\
									VG::VariantTypeName VG::Group::typeName;								


#define VariantGraphStatics(VG)		VG::VariantTypeMetaMap VG::variantTypeMeta;								\
									VG::VariantTypeNameMetaMap VG::variantTypeNameMeta;						\
									bool VG::variantGraphInitialized = false;											\
									VariantTypeStatics(VG)

typedef VariantGraph<std::wstring, unsigned char>	StringVariantGraph;


template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::addLocation(KeyPath &keyPath)
{
    typename KeyPath::iterator	lastItem = keyPath.end();
	lastItem--;

	return addGroup(keyPath, keyPath.begin(), lastItem);
}

template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::addGroup(Key &key)
{
	add(key, Group());

	return findGroup(key);
}

template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::addGroup(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd)
{
	Group *group;
															// If path position illegal, return NULL
	if(keyPathIterator == keyPath.end())
	{
		return NULL;
	}

	if((group = findGroup(*keyPathIterator)) == NULL)
	{
															// If another variant exists with same name, return NULL
		if(find(*keyPathIterator))
		{
			return NULL;
		}
															// Add a group node
		add(*keyPathIterator, Group());
															// Get group just created
		if((group = findGroup(*keyPathIterator)) == NULL)
		{
			return NULL;
		}
	}

	keyPathIterator++;

	if(keyPathIterator != keyPathEnd && keyPathIterator != keyPath.end())
	{
		return group->get().addGroup(keyPath, keyPathIterator, keyPathEnd);
	}

	return group;
}


template<typename K, typename VT>
inline Variant<VT> *PTRMI::VariantGraph<K, VT>::find(KeyPath &keyPath) const
{
	return find(keyPath, keyPath.begin(), keyPath.end());
}


template<typename K, typename VT>
inline Variant<VT> *PTRMI::VariantGraph<K, VT>::find(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const
{
															// If path position illegal, return NULL
	if(keyPathIterator == keyPath.end() || keyPathIterator == keyPathEnd)
	{
		return NULL;
	}

	KeyPath::iterator lastItem = keyPathEnd;
	lastItem--;
															// If path position not last item
	if(keyPathIterator != lastItem)
	{
		Group	*group;
															// Find given keyPath position entry in 'this' variant map (equivalent to looking for a folder at 'this' path)
		if(group = findGroup(*keyPathIterator))
		{
															// Found, so move to next keyPath entry
			keyPathIterator++;
															// Continue traversing hierarchy looking for key at next keyPath entry
			return group->get().find(keyPath, keyPathIterator, keyPathEnd);
		}
		else
		{
			return NULL;
		}
	}
															// Find item in 'this' map
	return find(*lastItem);
}


template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::findGroup(Key &key) const
{
	return dynamic_cast<Group *>(find(key));
}



template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group * PTRMI::VariantGraph<K, VT>::findGroup(KeyPath &keyPath) const
{
	return dynamic_cast<Group *>(find(keyPath));
}


template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::findGroup(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const
{
	return dynamic_cast<Group *>(find(keyPath, keyPathIterator, keyPathEnd));
}


template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::findLocation(KeyPath &keyPath) const
{
	return findLocation(keyPath, keyPath.begin(), keyPath.end());
}


template<typename K, typename VT>
inline typename PTRMI::VariantGraph<K, VT>::Group *PTRMI::VariantGraph<K, VT>::findLocation(KeyPath &keyPath, typename KeyPath::iterator keyPathIterator, typename KeyPath::iterator keyPathEnd) const
{
	KeyPath::iterator	containerEnd = keyPathEnd;
															// Finding containing location of given full path, so step back one level
	containerEnd--;
															// Get container
	return findGroup(keyPath, keyPathIterator, containerEnd);
}


template<typename K, typename VT>
inline Variant<VT> *PTRMI::VariantGraph<K, VT>::find(Key &key) const
{
    typename KeyVariantMap::const_iterator	it;
															// Look for given key at 'this' level
	if((it = keyVariants.find(key)) != keyVariants.end())
	{
		return it->second;
	}
															// Not found, so return NULL
	return NULL;
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::remove(Key &key)
{
	Variant<VT>					*	v;
    typename KeyVariantMap::iterator			it;
	bool							ret = true;
	Iterator						vit;
															// Look for given key at 'this' level
	if((it = keyVariants.find(key)) != keyVariants.end())
	{
		v = it->second;
															// Erase Key to Variant mapping
		keyVariants.erase(it);
	}
	else
	{
		ret = false;
	}

	if(v)
	{
															// Get variant
		if((vit = std::find(variants.begin(), variants.end(), v)) != variants.end())
		{
															// Erase variant entry
			variants.erase(vit);
		}
															// Delete variant if no dependencies exist
		if(v->decrementDependencyCounter() == 0)
		{
			delete v;
		}
	}
	else
	{
		ret = false;
	}
															// Return OK
	return ret;
}


template<typename K, typename VT>
inline void PTRMI::VariantGraph<K, VT>::read(DataBuffer &buffer)
{
	unsigned int		numItems;
	unsigned int		t;
	Key					key;
	VT					type;
	Variant<VT>		*	v;

	buffer >> numItems;

	if(numItems == 0)
		return;

	for(t = 0; t < numItems; t++)
	{
															// Read Type
		buffer >> type;
															// Read Key
		buffer >> key;
															// Read Variant
		if((v = createAndReadVariant(buffer, type)) != NULL)
		{
			addExisting(key, v);
		}
	}
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::getKey(Variant<VT> *variant, Key &key) const
{
	typename KeyVariantMap::const_iterator		it;
	//Variant<VT>						*	v;

	for(it = keyVariants.begin(); it != keyVariants.end(); it++)
	{
		if(it->second == variant)
		{
			key = it->first;
			return true;
		}
	}

	return false;
}

template<typename K, typename VT>
inline void PTRMI::VariantGraph<K, VT>::write(DataBuffer &buffer) const
{
	ConstIterator		it;
	Variant<VT>		*	v;
	Key					key;
															// Get number of items in the Map
	unsigned int numItems = getNumItems();
															// Write number of items
	buffer << numItems;
															// Write all variants to given buffer
	for(it = variants.begin(); it != variants.end(); it++)
	{
		if(v = *it)
		{
			if(getKey(v, key))
			{
															// Write variant type identifier
				buffer << v->getType();
															// Write Key
				buffer << key;
															// Write variant value
				v->write(buffer);
			}
		}
	}
}


template<typename VT>
inline void PTRMI::Variant<VT>::getIndents(std::wostringstream &string, unsigned int indent) const
{
	unsigned int t;

	for(t = 0; t < indent; t++)
	{
		string << std::wstring(L"\t");
	}
}


template<typename K, typename VT>
inline void PTRMI::VariantGraph<K, VT>::getIndents(std::wostringstream &string, unsigned int indent) const
{
	unsigned int t;

	for(t = 0; t < indent; t++)
	{
		string << std::wstring(L"\t");
	}
}


template<typename K, typename VT>
inline void PTRMI::VariantGraph<K, VT>::getString(std::wostringstream &string, unsigned int indent) const
{
	Key				key;
	Variant<VT> *	v;

	string << L"\n";

//	getIndents(string, indent);
//	string << VARIANT_TYPE_NAME_GROUP << "\n";

	getIndents(string, indent);
	string << std::wstring(L"{\n");

	ConstIterator it;

	for(it = variants.begin(); it != variants.end(); it++)
	{
		if(v = *it)
		{
			if(getKey(*it, key))
			{
				getIndents(string, indent + 1);
															// Write type
				v->getTypeString(string);

				string << " ";
															// Write Name
				string << key << " ";
															// Write Type and Value
				v->getString(string, indent + 1);

				string << L"\n";
			}
		}
	}

	getIndents(string, indent);
	string << L"}\n";
}

template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::readGroup(std::wifstream &in)
{
	//Group *childGroup;

	VariantKeyName		keyName;
	VariantTypeName		typeName;
	std::wstring		brace;

	readSymbol(in, brace);
	if(brace != L"{")
	{
		return false;
	}

	do 
	{
		if(readSymbol(in, typeName) == false)
		{
			return false;
		}

		if(typeName == L"}")
		{
			return true;
		}

		if(readSymbol(in, keyName) == false)
		{
			return false;
		}

		addExisting(keyName, createAndReadVariant(in, keyName, typeName));

/*
		if(typeName == VARIANT_TYPE_NAME_GROUP)
		{
			readSymbol(in, brace);

			if(brace == L"{")
			{
				if((childGroup = addGroup(keyName)) != NULL)
				{
					childGroup->get().readGroup(in);
				}
			}
			else
			{
				addExisting(keyName, createAndReadVariant(in, keyName, typeName));
			}
		}
*/

	} while (in.good());

	return in.good();
}

template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::readSymbol(std::wifstream &in, std::wstring &symbol)
{

	wchar_t	buffer[VARIANT_GRAPH_READ_BUFFER_SIZE];
	bool	found = false;

	do 
	{
		in >> symbol;

		if(symbol == VARIANT_READ_COMMENT_BLOCK_START)
		{
			do 
			{
				in >> symbol;

			} while(in.good() && symbol != VARIANT_READ_COMMENT_BLOCK_END);
		}
		else
		if(symbol == VARIANT_READ_COMMENT_LINE_START)
		{
			in.getline(buffer, VARIANT_GRAPH_READ_BUFFER_SIZE);
		}
		else
		{
			found = true;
		}

	} while (in.good() && found == false);


	return found;
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::readFile(const wchar_t *file, std::wistringstream *string)
{
	std::wifstream	in;
	bool			ret;

	if(file == NULL)
	{
		return false;
	}

	in.open(file);

	ret = readGroup(in);

	in.close();

	return ret;
}


template<typename K, typename VT>
inline bool PTRMI::VariantGraph<K, VT>::writeFile(const wchar_t *file, std::wostringstream *string) const
{
	std::wostringstream	str;

	if(file == NULL)
		return false;

	if(string == NULL)
	{
		getString(str, 0);

		string = &str;
	}

	std::wofstream	out;

	out.open(file);

	out << string->str();

	bool ret = out.good();

	out.close();

	return ret;
}


} // End PTRMI namespace
