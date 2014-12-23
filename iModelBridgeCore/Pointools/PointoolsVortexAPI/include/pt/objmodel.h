namespace pt
{

#define UID ParamIdType

	class Params
	{
	/*methods for class using this adaptor*/ 
	public:
		template <class T>
		bool add(const ParamIdType &id, T &v)
		{
			/*adds parameter into generic map as pointer*/ 
		}
		bool addCallback()
		{
			/* callback for parameter set */ 
		}
		template <class T>
		bool add(ParamMap &map)
		{
			//adds parameter into generic map
		}
		template <class T>
		bool add(ParamMap &map)
		{
			//adds parameter into generic map
		}
	public:
		template <class T>
		bool set(const ParamIdType &id, const T &v)
		{
			/*iterate of submaps and pointer map*/ 
			PARAMETER_MAP::iterator i = _data.find(id);
			if (i == _data.end()) return false;

			if (i->second.which() == Variant(v).which())
			{
				i->second = Variant(v);
				return true;
			}
			return false;
		}
	};

	class _ObjectI
	{
		const UID &id();
		bool get(UID,  
		
	
	template <class T> bool set(const ParamIdType &id, const T &v) { return _params.set(id, v); }
	template <class T> bool get(const ParamIdType &id, T &v) const { return _params.get(id, v);	}
//! Set parameter, returns success  

	
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
	};


};