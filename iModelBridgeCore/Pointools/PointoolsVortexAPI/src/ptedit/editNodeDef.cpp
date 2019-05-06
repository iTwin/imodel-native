#include "PointoolsVortexAPIInternal.h"
#include <ptedit/editNodeDef.h>

using namespace pt;
using namespace ptedit;

namespace detail
{
	typedef std::map<pt::String, EditNodeDef*> EditNodeDefs;
	EditNodeDefs _editNodeDefs;
}
using namespace detail;

/* constructor self registers */ 
EditNodeDef::EditNodeDef( const char *_name ) : _lastState("edit")
{
	_editNodeDefs.insert( EditNodeDefs::value_type( pt::String(_name), this ) );
}

bool EditNodeDef::applyNodeDef( const datatree::Branch *b )
{
	pt::String n;
	if (b->getNode("name", n ))
	{

		EditNodeDef *en = findNodeDef( String(n) );
		if (en)
		{
			if (en->readState(b)) 
				return en->apply();
		}
		else return false;
	}
	return false;
}

EditNodeDef *EditNodeDef::findNodeDef( const char *n )
{
	detail::EditNodeDefs::iterator i = detail::_editNodeDefs.find( String(n) );
	return (i==detail::_editNodeDefs.end()) ? 0 : i->second;
}
bool EditNodeDef::applyByName( const char *nm )
{
	EditNodeDef *nd = findNodeDef( nm );
	if (nd) return nd->apply();
	return false;
}
