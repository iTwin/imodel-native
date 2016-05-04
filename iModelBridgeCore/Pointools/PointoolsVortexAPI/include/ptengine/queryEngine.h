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
	static int64_t run( Traversal &traversal, FilterChain &filters )
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
	static int64_t run( Traversal &traversal, FilterChain &filters, DensityPolicy &d )
	{
		traversal.traverse( filters, d );
		return 0;
	}

};

}