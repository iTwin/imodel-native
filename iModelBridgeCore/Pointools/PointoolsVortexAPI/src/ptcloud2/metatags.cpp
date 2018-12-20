#include "PointoolsVortexAPIInternal.h"
#include <ptcloud2/metatags.h>

using namespace pcloud;

	
void MetaTags::clear()
{
	MetaTagGroups::iterator i = m_mtgroups.begin();
	while (i != m_mtgroups.end())
	{
		if(i->second != NULL)
		{
			delete i->second;
			++i;
		}
	}
	m_mtgroups.clear();
}

bool MetaTags::groupName(int index, pt::String &name) const
{
	const MetaTagMap *tags = getGroup(index, name);
	return (tags ? true : false);
}
int MetaTags::numTagsInGroup( const pt::String &group ) const
{
	const MetaTagMap *tags = getGroup( group );
	return (tags ? static_cast<int>(tags->size()) : 0 );
}
void MetaTags::addGroup(const pt::String &name)
{
	m_mtgroups.insert( MetaTagGroups::value_type(name, new MetaTagMap ) );
}
void MetaTags::addMetaTag( const pt::String &group, const pt::String &name, const pt::String &val )
{
	getGroup( group, true )->insert( MetaTagMap::value_type( name, val ) );
}
void MetaTags::setMetaTag( const pt::String &group, const pt::String &name, const pt::String &val )
{
	MetaTagMap * mg = getGroup( group, true );
	MetaTagMap::iterator i  = mg->find( name );

	if (i != mg->end())
		i->second = val;
	else
		mg->insert( MetaTagMap::value_type( name, val ) );
}
bool MetaTags::getMetaTag( int group, int index, pt::String &name, pt::String &val) const
{
	pt::String gname;
	const MetaTagMap* tags = getGroup( group, gname );
	if (!tags) return false;
	
	MetaTagMap::const_iterator i = tags->begin();
	
	while (index-- > 0 && i != tags->end())
	{
		++i;
	}
	if ( i!= tags->end())
	{
		name = i->first;
		val = i->second;
		return true;
	}
	val = "";
	return false;
}
bool MetaTags::getMetaTag(const pt::String &group, const pt::String &name, pt::String &val) const
{
	const MetaTagMap *tags = getGroup( group );	
	if (!tags) return false;
	MetaTagMap::const_iterator i = tags->find( name );
	if (i != tags->end())
	{
		val = i->second;
		return true;
	}
	val = "";
	return false;
}
void MetaTags::operator = ( const MetaTags &mt )
{
	clear();

	/* copy tags only, groups will be created as we go */ 
	for (int g=0;g<mt.numGroups();g++)
	{
		pt::String gname, mtname, mtval;
		mt.groupName(g, gname);
		int numTags = mt.numTagsInGroup(gname);
		for (int i=0; i<numTags; i++)
		{
			mt.getMetaTag(g, i, mtname, mtval);
			if (mtname.length())
			{
				if (!mtval.length()) mtval = pt::String("");
				this->addMetaTag(gname, mtname, mtval);
			}
		}
	}	
}
int MetaTags::writeToBlock( ptds::WriteBlock &wb ) const
{
	int version = 1;
	wb.write( version );
	wb.write( (int)numGroups() );
	
	for (int g=0;g<numGroups();g++)
	{
		pt::String gname, mtname, mtval;
		groupName(g, gname);

		wb.write( gname );

		int numTags = numTagsInGroup(gname);
		wb.write(numTags);

		for (int i=0; i<numTags; i++)
		{
			getMetaTag(g, i, mtname, mtval);
			wb.write( mtname );
			wb.write( mtval );
		}
	}	
	return 1;
}
int MetaTags::readFromBlock( ptds::ReadBlock &rb )
{
	clear();

	int version = 1;
	int num;
	rb.read( version );
	rb.read( num );
	
	for (int g=0;g<num;g++)
	{
		pt::String gname, mtname, mtval;
		rb.read( gname );

		int numTags = 0;
		rb.read(numTags);

		for (int i=0; i<numTags; i++)
		{
			rb.read( mtname );
			rb.read( mtval );
			addMetaTag( gname, mtname, mtval );
		}
	}	
	return 1;
}

	
MetaTags::MetaTagMap *MetaTags::getGroup( const pt::String &name, bool addIfNeeded )
{
	MetaTagGroups::iterator i = m_mtgroups.find( name );
	if (i== m_mtgroups.end() && addIfNeeded )
	{
		MetaTagMap *tags = new MetaTagMap;
		m_mtgroups.insert( MetaTagGroups::value_type( name, tags ) );
		return tags;
	}
	return (i == m_mtgroups.end() ? 0 : i->second );
}
const MetaTags::MetaTagMap *MetaTags::getGroup( const pt::String &name ) const
{
	MetaTagGroups::const_iterator i = m_mtgroups.find( name );
	return (i == m_mtgroups.end() ? 0 : i->second );
}

const MetaTags::MetaTagMap *MetaTags::getGroup(int index, pt::String &name) const
{
	MetaTagGroups::const_iterator i = m_mtgroups.begin();

	while (index && i!= m_mtgroups.end())
	{
		--index;
		++i;
	}
	if ( i != m_mtgroups.end() ) 
	{
		name = i->first;
		return i->second;
	}
	return 0;
}

MetaTags::MetaTagMap *MetaTags::getGroup(int index, pt::String &name)
{
	MetaTagGroups::iterator i = m_mtgroups.begin();

	while (index && i!= m_mtgroups.end())
	{
		--index;
		++i;
	}
	if ( i != m_mtgroups.end() ) 
	{
		name = i->first;
		return i->second;
	}
	return 0;
}


