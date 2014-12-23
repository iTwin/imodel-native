#ifndef POINTOOLS_VARIANT_DEFINITION
#define POINTOOLS_VARIANT_DEFINITION

#include <ttl\var\variant.hpp>
#include <pt\ptstring.h>
#include <pt\geomtypes.h>
#include <string>

namespace pt
{
	typedef ttl::var::variant<int, bool, float, double, unsigned int, __int64, vector3i, vector3, vector3d, String, unsigned __int64> Variant;
	typedef std::string ParamIdType;

	struct ValueToString
	{
		ValueToString(String &str) : s(str){};

		void operator()(const int &v)			{	s.format("%i", v);	}	
		void operator()(const bool &v)			{	s = v ? "true" : "false";	}
		void operator()(const double &v)		{	s.format("%0.4f", v);	}
		void operator()(const float &v)			{	s.format("%0.4f", v);	}
		void operator()(const unsigned int &v)	{	s.format("%i", (int)v);	}
		void operator()(const __int64 &v)		{	s.format("%i", (int)v);	}
		void operator()(const vector3i &v)		{	s.format("%i, %i, %i", v.x, v.y, v.z);	}
		void operator()(const vector3 &v)		{	s.format("%f, %f, %f", v.x, v.y, v.z);	}
		void operator()(const vector3d &v)		{	s.format("%f, %f, %f", v.x, v.y, v.z);	}
		void operator()(const pt::String &v)		{	s = v;	}	
		void operator()(const unsigned __int64 &v)	{	s.format("%i", (int)v);	}
		
		pt::String &s;
	};

	struct VariantUtils
	{
		static void toString( const Variant &v, pt::String &s )
		{
			ValueToString vu(s);
			ttl::var::apply_visitor(vu, v);
		}
	};
}

#endif