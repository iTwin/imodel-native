#pragma once

#include <ptcloud2/voxel.h>
#include <ptcloud2/scene.h>

namespace pointsengine
{
	enum FilterResult
	{
		FilterOut = 0,
		FilterIn = 1,
		FilterPartial = 2
	};

#define IMPL_FILTER_IN	\
	inline static FilterResult node( const pcloud::Node *n )			{	return FilterIn;	} \
	inline static FilterResult cloud( const pcloud::PointCloud *pc )	{	return FilterIn;	} \
	inline static FilterResult scene( const pcloud::Scene *sc )			{	return FilterIn;	} 

#define IMPL_FILTER_OUT	\
	inline static FilterResult node( const pcloud::Node *n )			{	return FilterOut;	} \
	inline static FilterResult cloud( const pcloud::PointCloud *pc )	{	return FilterOut;	} \
	inline static FilterResult scene( const pcloud::Scene *sc )			{	return FilterOut;	} 

		
	inline static FilterResult combinedResult( const FilterResult &first, const FilterResult &second )
	{
		if (first == FilterOut || second == FilterOut) 
			return FilterOut;

		if (second==FilterPartial) return FilterPartial;
		return first;
	}

	struct NullFilter
	{
		IMPL_FILTER_OUT
	};
}