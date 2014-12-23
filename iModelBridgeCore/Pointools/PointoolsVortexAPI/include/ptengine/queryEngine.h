#pragma once

#include <ptengine/queryTraversal.h>

namespace pointsengine
{

class QueryEngine
{
public:

	template
	< 
		class Traversal, 
		class FilterChain
	>
	static __int64 run( Traversal &traversal, FilterChain &filters )
	{
		traversal.traverse( filters );
		return 0;
	}

	template
	< 
		class Traversal, 
		class FilterChain,
		class DensityPolicy
	>
	static __int64 run( Traversal &traversal, FilterChain &filters, DensityPolicy &d )
	{
		traversal.traverse( filters, d );
		return 0;
	}

};

}