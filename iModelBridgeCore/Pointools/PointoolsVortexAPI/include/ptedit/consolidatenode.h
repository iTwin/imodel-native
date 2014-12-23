#pragma once

#include "editNodeDef.h"
#include <ptcloud2/voxel.h>

namespace ptedit
{
	struct ConsolidationData
	{
		std::map<__int64, uint> nodeSpecs;
		std::map<__int64, ubyte*> nodeFilters;
	};

	class ConsolidateNode : public EditNodeDef
	{
	public:
		ConsolidateNode();

		const pt::String &name() const  { static pt::String n("ConsolidateNode"); return n; }
		const pt::String &desc() const  { static pt::String n("Snapshot Node"); return n; }
		int icon() const { return -1; }

		/* apply the filter, return true if anything was done */ 
		bool apply();
		bool writeState( pt::datatree::Branch *b) const;
		bool readState(const pt::datatree::Branch *);
		uint flags();

		bool applyNodeDef( const pt::datatree::Branch *b );
	};
};
