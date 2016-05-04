#pragma once

#include <pt/classes.h>

// simple guid class
namespace pt
{
struct Guid
{
	typedef int64_t GuidPartType;

	Guid() 
	{
		guid[0] = 0;
		guid[1] = 0;
	}
	Guid(const Guid &g) 
	{
		guid[0] = g.guid[0];
		guid[1] = g.guid[1];
	}
	Guid(const GuidPartType &g) 
	{
		(*this)=g;
	}
	Guid(const GuidPartType &g1, const GuidPartType &g2 ) 
	{
		guid[0] = g1;
		guid[1] = g2;
	}
	void generate();

	bool isValid() const
	{
		return guid[0] != 0 || guid[1] != 0
			? true : false;
	}
	void operator = (GuidPartType v)	// point cloud uses single int64 guid
	{
		guid[0] = v;
		guid[1] = 0;
	}	
	bool operator == (Guid v) const
	{
		return (guid[0]==v.guid[0]
			&& guid[1]==v.guid[1]) ? true : false;
	}
	void operator = (Guid v)	// point cloud uses single int64 guid
	{
		guid[0] = v.guid[0];
		guid[1] = v.guid[1];
	}

	operator bool() const
	{
		return isValid();
	}

	void setPart1( GuidPartType g1 )
	{
		guid[0] = g1;
	}
	void setPart2( GuidPartType g2 )
	{
		guid[1] = g2;
	}
	GuidPartType getPart1() const	
	{ 
		return guid[0]; 
	}
	GuidPartType getPart2() const	
	{ 
		return guid[1]; 
	}

private:
	GuidPartType	guid[2];
};
}