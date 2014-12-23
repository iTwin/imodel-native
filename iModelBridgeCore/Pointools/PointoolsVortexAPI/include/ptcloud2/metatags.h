// Metatags 
#pragma once
#include <ptfs/iohelper.h>
#include <pt/ptstring.h>
namespace pcloud
{

class MetaTags
{
public:
	MetaTags()
	{
		addGroup( pt::String("Default") );
	}
	~MetaTags()
	{
		clear();
	}	
	void clear();
	int numGroups() const					
	{ 
		return (int)m_mtgroups.size(); 
	}
	bool groupName(int index, pt::String &name) const;
	int numTagsInGroup( const pt::String &group ) const;
	void addGroup(const pt::String &name);

	void addMetaTag( const pt::String &group, const pt::String &name, const pt::String &val );
	void setMetaTag(const pt::String &group, const pt::String &name, const pt::String &val);

	bool getMetaTag( int group, int index, pt::String &name, pt::String &val) const;
	bool getMetaTag(const pt::String &group, const pt::String &name, pt::String &val) const;
	
	void operator = ( const MetaTags &mt );
	int writeToBlock( ptds::WriteBlock &wb ) const;
	int readFromBlock( ptds::ReadBlock &rb );

private:

	typedef std::map<pt::String, pt::String> 	MetaTagMap;
	typedef std::map<pt::String, MetaTagMap*> 	MetaTagGroups;
	
	MetaTagMap *getGroup( const pt::String &name, bool addIfNeeded );
	const MetaTagMap *getGroup( const pt::String &name ) const;

	const MetaTagMap *getGroup(int index, pt::String &name) const;

	MetaTagMap *getGroup(int index, pt::String &name);

	MetaTagGroups m_mtgroups;
};


}