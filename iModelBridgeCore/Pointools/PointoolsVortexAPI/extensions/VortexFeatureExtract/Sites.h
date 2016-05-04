#pragma once

#include "GeomTypes.h"
#include <map>
#include <vector>
#include <set>

namespace vortex
{

#define INVALID_BIT 0x80000000

typedef uint64_t	SKeyType;
typedef unsigned int		SIndexType;

class Site
{
public:
	Site(unsigned int key) : numValid(0)  {}

	void	addPoint( const SIndexType &pntIndex )	
	{ 
		removePnt(pntIndex);	
		indices.insert(Indices::value_type(pntIndex,true)); 
		++numValid; 
	}
	int		numPnts() const							{ return numValid; }

	bool	removePnt( SIndexType pntIndex )
	{
		Indices::iterator i = indices.find(pntIndex);
		if (i!=indices.end() && i->second) 
		{
			i->second = false;
			--numValid;
		}
		else return false;
		return true;
	}

	int		getPnts( std::vector<SIndexType> &pnts ) const
	{
		for (Indices::const_iterator i=indices.begin(); i!=indices.end();i++)
		{
			if (i->second) pnts.push_back(i->first);
		}
		return numValid;
	}

	const SKeyType &getKey() const					{ return key; }

	void clear()
	{
		indices.clear();
		numValid = 0;
	}

	const SIndexType &getFirstPnt() 	const		
	{ 
		for (Indices::const_iterator i=indices.begin(); i!=indices.end();i++)
		{
			if (i->second) return (i->first);
		}		
		const_cast<Site*>(this)->numValid=0;
		return indices.begin()->first;	//should never happen
	}

private:
	SKeyType								key;
	typedef std::map<SIndexType,bool>	Indices;
	Indices								indices;
	int									numValid;
};
//-----------------------------------------------------------------------------
// Grid Sites
//-----------------------------------------------------------------------------
// Note: Does not handle negative x,y or z values
//-----------------------------------------------------------------------------
template <class T>
class GridSites
{
public:
	typedef Vector3<T> VecType;

	GridSites( T dist ) : spacing(dist,dist,dist)
	{
	}
	~GridSites()
	{
		clear();
	}
	void setOffset( VecType off ) { offset = off; }
	int numSites() const { return sites.size(); }
	inline unsigned int siteKey( int x, int y, int z) const
	{
		return x + 10000 * y + 100000000 * z; 
	}
	inline SKeyType siteKey( const VecType &p ) const
	{
		int x = floor((p.x + offset.x) / spacing.x);
		int y = floor((p.y + offset.y) / spacing.y);
		int z = floor((p.z + offset.z) / spacing.z);

		return siteKey(x,y,z);		
	}
	inline int xFromKey(const SKeyType &key) const
	{
		return key % 10000;
	}
	inline int yFromKey(const SKeyType &key) const
	{
		return (key % 100000000)/10000;
	}
	inline int zFromKey(const SKeyType &key) const
	{
		return (key / 100000000);
	}
	Site *getSite(SKeyType key, bool create=false)
	{
		Sites::iterator i = sites.find(key);
		if (i == sites.end())
		{
			if (create)
			{
				Site *s = new Site(key);
				sites.insert(Sites::value_type(key, s));
				return s;
			}
			else return 0;
		}
		else return i->second;
	}
	SKeyType addPoint( const VecType &pnt, SIndexType pindex )
	{
		SKeyType key = siteKey(pnt);
		Site *s = getSite(key, true);
		s->addPoint(pindex);
		return key;
	}
	Site* getSiteNeighbour( SKeyType key, int xOffset, int yOffset, int zOffset)
	{
		int x = xFromKey(key) + xOffset;
		int y = yFromKey(key) + yOffset;
		int z = zFromKey(key) + zOffset;

		return getSite( siteKey(x,y,z) ); 
	}
	void clear()
	{
		Sites::iterator i = sites.begin();
		while (i != sites.end())
		{
			delete i->second;
			++i;
		}
		sites.clear();
	}
	int getPntNeighbourhood( const VecType &pnt, double dist, std::vector<SIndexType> &pnts )
	{
		int xw = ceil( dist / spacing.x);
		int yw = ceil( dist / spacing.y);
		int zw = ceil( dist / spacing.z);

		SKeyType k = siteKey(pnt);

		int x = xFromKey(k);
		int y = yFromKey(k);
		int z = zFromKey(k);

		int origSize = pnts.size();

		for (int xi=x-xw; xi<x+xw+1;xi++)
		{
			for (int yi=y-yw; yi<y+yw+1;yi++)
			{
				for (int zi=z-zw; zi<z+zw+1;zi++)
				{
					Site *site = getSite(siteKey(xi,yi,zi));
					if (site)
					{
						site->getPnts(pnts);
					}
				}
			}
		}
		return pnts.size() - origSize;
	}
	bool getNextValidPnt(SIndexType &pnt, SKeyType &site ) const
	{
		Sites::const_iterator i = sites.begin();

		while (i!= sites.end())
		{
			if (i->second->numPnts()) 
			{
				pnt = i->second->getFirstPnt();
				site = i->first;
				
				if (i->second->numPnts())
					return true;
			}
			++i;
		}
		return false;
	}
	// get remaining indices
	int getIndices( std::vector<SIndexType> &indices )
	{
		int count = 0;

		Sites::const_iterator i = sites.begin();

		while (i!= sites.end())
		{
			count += i->second->getPnts(indices);
			++i;
		};
		return count;
	}
	int meanPntsPerSite() const
	{
		int totalPnts = 0;
		Sites::const_iterator i = sites.begin();

		while (i!= sites.end())
		{
			totalPnts += i->second->numPnts();
			++i;
		}
		return totalPnts/sites.size();
	}
private:
	VecType spacing; 
	VecType offset; 
	typedef std::map<SKeyType, Site*>	Sites;
	Sites sites;
};
typedef GridSites<float> GridSitesf;
typedef GridSites<float> GridSitesd;
};
