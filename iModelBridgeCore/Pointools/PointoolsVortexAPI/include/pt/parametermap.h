/*--------------------------------------------------------------------------*/ 
/*  ParameterMap.h															*/ 
/*	Parameter Map definition and implementation								*/ 
/*  Requires no linking														*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 27 December 2004 Faraz Ravi								*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef COMMONCLASSES_PARAMETERMAP_DEFINITION
#define COMMONCLASSES_PARAMETERMAP_DEFINITION

#include <pt/variant.h>
#include <Loki/AssocVector.h>
#include <vector>
#include <set>

#pragma warning (disable : 4800)
#pragma warning (disable : 4311) //pointer truncation from 'void *' to 'unsigned int'
#pragma warning (disable : 4312) //conversion from 'unsigned int' to 'void *' of greater size

namespace pt
{
#define PARAMETER_MAP_TYPE Loki::AssocVector
#define PARAM_NULL_VALUE -2147483648

//------------------------------------------------------------------------------
// ParameterMap
//------------------------------------------------------------------------------
//! Stores unordered variant parameters keyed by strings  
//------------------------------------------------------------------------------
class ParameterMap
{
public:
	ParameterMap(){}; //! Default Constructor, does nothing
	ParameterMap(const ParameterMap &m) { *this = m; } //! Copy constructor
	~ParameterMap(){}; //! Destructor, does nothing, no heap allocations to destroy.
	
	template <class T> 
	bool insert(const ParamIdType &id, const T &v)	{
		return _data.insert(PARAMETER_MAP::value_type(id,Variant(v))).second;
	}//! Add a parameter  

	bool insertV(const ParamIdType &id, const Variant &v)
	{
		return _data.insert(PARAMETER_MAP::value_type(id,v)).second;	
	}
	void clear()		{ _data.clear(); }
	int size() const	{ return (int)_data.size(); }

	
	bool has(const ParamIdType &id)	{
		return _data.find(id) != _data.end();
	}//! Check parameter exists, warning: no type checking done  

	
	template <class T>
	bool set(const ParamIdType &id, const T &v)
	{
		PARAMETER_MAP::iterator i = _data.find(id);
		if (i == _data.end()) return false;

		if (i->second.which() == Variant(v).which())
		{
			i->second = Variant(v);
			return true;
		}
		return false;
	}//! Set parameter, returns success  

	bool setV(const ParamIdType &id, const Variant &v)
	{
		PARAMETER_MAP::iterator i = _data.find(id);
		if (i == _data.end()) return false;

		if (i->second.which() == v.which())
		{
			i->second = v;
			return true;
		}
		return false;
	}//! Set parameter, 

	template <class T>
	bool get(const ParamIdType &id, T &v) const 
	{	
		PARAMETER_MAP::iterator i = _data.find(id);
		if (i == _data.end()) return false;

		if (i->second.which() == Variant(v).which())
		{
			v = ttl::var::get<T>(i->second);
			return true;
		}
		return false;
	}//! Access parameter, returns success
	template <class T>
	bool typesMatch(const ParamIdType &id, const T &v) const
	{
		PARAMETER_MAP::iterator i = _data.find(id);
		if (i == _data.end()) return false;
		return (i->second.which() == Variant(v).which());				
	}

	const Variant* get(const ParamIdType &id) const
	{
		PARAMETER_MAP::const_iterator i = _data.find(id);
		if (i == _data.end()) return 0;
		return &i->second;		
	}//! Access parameter pointer 

	template<class Visitor> 
	void applyVisitor(Visitor &v)
	{
		PARAMETER_MAP::iterator i=_data.begin();	
		while (i!= _data.end())
		{
			v.identifier = i->first;
			ttl::var::apply_visitor(v, i->second);
			++i;
		};
	}//! Apply visitor to whole parameter map  
	
	void operator = (const ParameterMap &pm)
	{
		clear();

		PARAMETER_MAP::const_iterator i= pm._data.begin();
		while (i != pm._data.end())
		{
			_data.insert(PARAMETER_MAP::value_type(i->first, i->second));
			++i;
		};
	}//! Copy parameter map 
	void copyValues(const ParameterMap &pm, bool merge=false)
	{
		PARAMETER_MAP::const_iterator i= pm._data.begin();
		while (i != pm._data.end())
		{
			PARAMETER_MAP::iterator j = _data.find(i->first);
			if (j!=_data.end() && j->second.which() == i->second.which())
				j->second = i->second;
			else if (merge) insert(i->first, i->second);
			++i;
		};		
	}
	struct VariantCompare
	{
		bool equal;
		template<class TA, class TB> void operator()(const TA &a, const TB &b) { equal = false; };
		template <class T> void operator()(const T &a, const T &b) { equal = (a == b); }
	};
	int compare(const ParameterMap &pm, ParameterMap &differences) const
	{
		int diffs = 0;

		PARAMETER_MAP::const_iterator i= pm._data.begin();
		while (i != pm._data.end())
		{
			PARAMETER_MAP::const_iterator j = _data.find(i->first);
			if (j!=_data.end() && j->second.which() == i->second.which())
			{
				VariantCompare v;
				ttl::var::apply_visitor(v, i->second, j->second);

				if (!v.equal)
				{
					differences.insertV(i->first, i->second);
					++diffs;
				}
			}
			else 
			{
				differences.insertV(i->first, i->second);
				++diffs;
			}
			++i;
		}
		return diffs;
	}
	void getParameterIds(std::set<ParamIdType> &ids)
	{
		PARAMETER_MAP::const_iterator i= _data.begin();
		while (i != _data.end()) { ids.insert(i->first); ++i; }
	}
	void getParameterIds(std::vector<ParamIdType> &ids)
	{
		PARAMETER_MAP::const_iterator i= _data.begin();
		while (i != _data.end()) { ids.push_back(i->first); ++i; }
	}
private:
	typedef PARAMETER_MAP_TYPE<ParamIdType, Variant> PARAMETER_MAP; 
	PARAMETER_MAP _data; //! The parameter map
};

//------------------------------------------------------------------------------
// ParameterList
//------------------------------------------------------------------------------
//! Stores ordered parameters 
//------------------------------------------------------------------------------
struct ParameterList
{
	ParameterList(){};
	~ParameterList(){};

	/* Parameter Insertion */ 
	template <class T> 
		void insert(const T &v) { _data.push_back(MakeVariant()(v)); }
	template <class t0, class t1>	
		void insert(const t0 &p0, const t1 &p1) { insert(p0); insert(p1); }
	template <class t0, class t1, class t2>	
		void insert(const t0 &p0, const t1 &p1, const t2 &p2) { insert(p0); insert(p1); insert(p2); }
	template <class t0, class t1, class t2, class t3>	
		void insert(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3) { insert(p0); insert(p1); insert(p2); insert(p3); }
	template <class t0, class t1, class t2, class t3, class t4>	
		void insert(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3, const t4 &p4) { insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); }
	template <class t0, class t1, class t2, class t3, class t4, class t5>	
		void insert(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3, const t4 &p4, const t5 &p5) { insert(p0); insert(p1); insert(p2); insert(p3); insert(p4); insert(p5); }

	/* Parameter List creation (less efficient but easier) */ 
	template <class t0>	
		static ParameterList create(const t0 &p0) { ParameterList pl; pl.insert(p0); return pl; }
	template <class t0, class t1>	
		static ParameterList create(const t0 &p0, const t1 &p1) { ParameterList pl; pl.insert(p0); pl.insert(p1); return pl; }
	template <class t0, class t1, class t2>	
		static ParameterList create(const t0 &p0, const t1 &p1, const t2 &p2) { ParameterList pl; pl.insert(p0); pl.insert(p1); pl.insert(p2); return pl; }
	template <class t0, class t1, class t2, class t3>	
		static ParameterList create(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3) { ParameterList pl; pl.insert(p0); pl.insert(p1); pl.insert(p2); pl.insert(p3); return pl; }
	template <class t0, class t1, class t2, class t3, class t4>	
		static ParameterList create(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3, const t4 &p4) { ParameterList pl; pl.insert(p0); pl.insert(p1); pl.insert(p2); pl.insert(p3); pl.insert(p4); return pl; }
	template <class t0, class t1, class t2, class t3, class t4, class t5>	
		static ParameterList create(const t0 &p0, const t1 &p1, const t2 &p2, const t3 &p3, const t4 &p4, const t5 &p5) { ParameterList pl; pl.insert(p0); pl.insert(p1); pl.insert(p2); pl.insert(p3); pl.insert(p4); pl.insert(p5); return pl; }

	void clear()			{ _data.clear(); }					//! Clear parameters
	int size() const		{ return (int)_data.size(); }			//! Number of parameters

	template <class T>
	bool setAt(int index, const T &v) 
	{ 
		if (match(index, v))
		{
			_data[index] = Variant(v);
			return true;
		}
		return false;
	}//! Set parameter, returns success 
	struct MakeVariant
	{
		Variant operator()(void *v) const {	return Variant((size_t)v);	}
		template <class T>	Variant operator()(const T &v) const	{ return Variant(v); }
	};
	
	template <class T>
	bool getAt(int index, T &v) const
	{
		return (*this)(index, v);
	}//! Access type matched parameter, returns success 
	
	template <class T>
	bool matchAt(int index, T &v) const 
	{ 
		return MakeVariant()(v).which() == _data[index].which(); 
	}

	template <class t0>	int match(t0 &p0) const 
	{ 
		return matchAt(0, p0) ? -1 : 0; 
	}//! match 1 ordered parameter, return -1 for success otherwise failed parameter index
	
	template <class t0, class t1>	
		int match(t0 p0, t1 p1) const 
	{ 
		if (!matchAt(0, p0)) return 0;
		if (!matchAt(1, p1)) return 1;
		return -1;
	}//! match 2 ordered parameters
		 
	template <class t0, class t1, class t2>	
		int match(t0 p0, t1 p1, t2 p2) const 
	{ 
		if (!matchAt(0, p0)) return 0;
		if (!matchAt(1, p1)) return 1;	
		if (!matchAt(2, p2)) return 2;	
		return -1;
	}//! match 3 ordered parameters

	template <class t0, class t1, class t2, class t3>	
		int match(t0 p0, t1 p1, t2 p2, t3 p3) const 
	{ 
		if (!matchAt(0, p0)) return 0;
		if (!matchAt(1, p1)) return 1;	
		if (!matchAt(2, p2)) return 2;	
		if (!matchAt(3, p3)) return 3;	
		return -1;
	}//! match 4 ordered parameters

	template <class t0, class t1, class t2, class t3, class t4>	
		int match(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4) const 
	{ 
		if (!matchAt(0, p0)) return 0;
		if (!matchAt(1, p1)) return 1;	
		if (!matchAt(2, p2)) return 2;	
		if (!matchAt(3, p3)) return 3;	
		if (!matchAt(4, p4)) return 4;	
		return -1;
	}//! match 5 ordered parameters

	template <class t0, class t1, class t2, class t3, class t4, class t5>	
		int match(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) const
	{ 
		if (!matchAt(0, p0)) return 0;
		if (!matchAt(1, p1)) return 1;	
		if (!matchAt(2, p2)) return 2;	
		if (!matchAt(3, p3)) return 3;	
		if (!matchAt(4, p4)) return 4;	
		if (!matchAt(5, p5)) return 5;	
		return -1;
	}//! match 6 ordered parameters

	
	template <class t0>	int get(t0 &p0) const
	{ 
		return getAt(0, p0) ? -1 : 0; 
	}//! get 1 ordered parameter, return -1 for success otherwise failed parameter index
	
	template <class t0, class t1>	
		int get(t0 &p0, t1 &p1) const 
	{ 
		if (!getAt(0, p0)) return 0;
		if (!getAt(1, p1)) return 1;
		return -1;
	}//! get 2 ordered parameters
		 
	template <class t0, class t1, class t2>	
		int get(t0 &p0, t1 &p1, t2 &p2) const
	{ 
		if (!getAt(0, p0)) return 0;
		if (!getAt(1, p1)) return 1;	
		if (!getAt(2, p2)) return 2;	
		return -1;
	}//! get 3 ordered parameters

	template <class t0, class t1, class t2, class t3>	
		int get(t0 &p0, t1 &p1, t2 &p2, t3 &p3) const 
	{ 
		if (!getAt(0, p0)) return 0;
		if (!getAt(1, p1)) return 1;	
		if (!getAt(2, p2)) return 2;	
		if (!getAt(3, p3)) return 3;	
		return -1;
	}//! get 4 ordered parameters

	template <class t0, class t1, class t2, class t3, class t4>	
		int get(t0 &p0, t1 &p1, t2 &p2, t3 &p3, t4 &p4) const 
	{ 
		if (!getAt(0, p0)) return 0;
		if (!getAt(1, p1)) return 1;	
		if (!getAt(2, p2)) return 2;	
		if (!getAt(3, p3)) return 3;	
		if (!getAt(4, p4)) return 4;	
		return -1;
	}//! get 5 ordered parameters

	template <class t0, class t1, class t2, class t3, class t4, class t5>	
		int get(t0 &p0, t1 &p1, t2 &p2, t3 &p3, t4 &p4, t5 &p5) const
	{ 
		if (!getAt(0, p0)) return 0;
		if (!getAt(1, p1)) return 1;	
		if (!getAt(2, p2)) return 2;	
		if (!getAt(3, p3)) return 3;	
		if (!getAt(4, p4)) return 4;	
		if (!getAt(5, p5)) return 5;	
		return -1;
	}//! get 6 ordered parameters

	template<class Visitor> 
	void applyVisitor(Visitor &v)
	{	
		for (int i=0; i<_data.size(); i++)
			ttl::var::apply_visitor(v, _data[i]);
	}//! Apply visitor to whole parameter list 
	
	void operator = (const ParameterList &pm)
	{
		clear();
		std::copy(pm._data.begin(), pm._data.end(), _data.begin());
	}//! Copy parameter list 

private:
	bool operator()(int index, void *&v) const
	{	
		unsigned int p;

		if ((*this)(index, p))
		{
			v = (void *)p;
			return true;
		}
		return false;			
	}
	template <class T>
		bool operator()(int index, T &v) const	
	{ 
		if (matchAt(index, v))
		{
			v = ttl::var::get<T>(_data[index]);
			return true;
		}
		return false; 
	}
	typedef std::vector <Variant> PARAMETER_LIST; 
	PARAMETER_LIST _data;
};

}
#endif
