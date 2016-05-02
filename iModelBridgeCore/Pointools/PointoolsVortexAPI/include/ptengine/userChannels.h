/*----------------------------------------------------------*/ 
/* UserChannels.h											*/ 
/* User Channels Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2008-09							*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#pragma once

#include <set>
#include <loki/assocvector.h>

#include <pt/datatreeBranch.h>

#include <ptfs/filepath.h>
#include <ptfs/iohelper.h>

#include <ptcloud2/Voxel.h>
#include <ptcloud2/PointCloud.h>

#include <ptengine/ptengine_api.h>
#include <ptengine/module.h>


#define PT_NULL_FILE_POS 0xFFFFFFFFFFFFFFFF
  
namespace pointsengine
{
#define ChannelMapType Loki::AssocVector

enum UserChannelFlags
{
	UserChannel_OOC = 1
};
struct VoxelID
{
	pcloud::PointCloudGUID pc;
	unsigned int voxelIndex;

	static void fromVoxel( VoxelID &id, const pcloud::Voxel*v ) 
	{ 
		id.pc = v->pointCloud()->guid(); 
		id.voxelIndex = v->indexInCloud(); 
	}
};
struct OOCFile
{
	OOCFile() : fhandle(0), fend(0) {};
	~OOCFile();
	void create( class UserChannel *uchannel );
	void destroy();
	bool writeVCD( class VoxelChannelData* vcd, size_t numPoints=0 );
	bool readVCD( class VoxelChannelData* vcd, size_t numPoints=0 );

	static void setOOCFileFolder( const pt::String &folder );
	static const pt::String &getOOCFileFolder();

	ptds::DataSourcePtr fhandle;
	ptds::DataPointer	fend;
	pt::String fname;
};
/* Voxel Channel data */ 
/* The channel data for one voxel */ 
class VoxelChannelData
{
public:
	VoxelChannelData( uint num_points, uint bytesPerPnt, uint full_num_points, ubyte setup_flags=0, uint user_0 = 0, uint user_1 = 0 );
	~VoxelChannelData() {} /* do not destroy here */

	/* flags */ 
	inline bool isOOC() const { return flags & UserChannel_OOC ? true : false; }
	inline bool isLocked() const { return (isOOC() && !data) ? true : false; }
	inline bool isUniform() const { return (uniform_value) ? true : false; }

	bool copy(VoxelChannelData &source);

	/* get value */ 
	template <class T>
	bool getValR( int i, T &v ) const
	{
		if (!data)
		{
			if (uniform_value) memcpy(&v, uniform_value, sizeof(v));
			else return false;
		}
		else
			memcpy(&v, &data[i*bytesPerPoint], sizeof(T));
		return true;
	}

	bool getVal( int i, void* v ) const
	{
		if (!data)
		{
			if (uniform_value) memcpy(v, uniform_value, bytesPerPoint);
			else return false;
		}
		else 
			memcpy(v, &data[i*bytesPerPoint], bytesPerPoint);
		return true;
	}
	
	/* set value */ 
	template <class T>
	bool setValR(uint i, T &v )
	{
		return setVal( i, &v );
	}

	template <class T>
	bool setVal(uint i, T *v )
	{
		if (i >= numPoints) return false;
		if (!data)
		{
			/* if not uniform or value differs from uniform */ 
			if (!isUniform() || memcmp(v, uniform_value, bytesPerPoint)!=0) 
			{
				/* if ooc and has a filepos it needs to be loaded, ie unlocked */ 
				if (isOOC() && this->filepos != PT_NULL_FILE_POS) return false;
				
				if (!allocate()) return false;

				if (uniform_value) delete uniform_value;
				uniform_value = 0;
			}
			else return false;
		}
		memcpy(&data[i*bytesPerPoint], v, bytesPerPoint);		
		return true;
	}

	template <class T> 
	bool setUniformVal(T &v) 
	{ 
		destroy(true);

		if (uniform_value) delete uniform_value;
		uniform_value = new T;
		*uniform_value = v;
	} 
	template <class T> 
	bool setMinVal(T &v) 
	{ 
		if (max_value) delete max_value;
		max_value = new T;
		*max_value = v;
	} 
	template <class T> 
	bool setMaxVal(T &v) 
	{ 
		if (min_value) delete min_value;
		min_value = new T;
		*min_value = v;
	} 
	/* getters */ 
	template <class T> 
	bool getMinVal(T &v)					{ if (!min_value) return false; memcpy(&v, min_value, sizeof(T)); return true; } 

	template <class T> 
	bool getMaxVal(T &v)					{ if (!max_value) return false; memcpy(&v, max_value, sizeof(T)); return true; } 

	template <class T> 
	bool getUniformVal(T &v)				{ if (!uniform_value) return false; memcpy(&v, uniform_value, sizeof(T)); return true; } 

	const void *getData() const				{ return data; }
	void *getData()							{ return data; }

	uint getBytesPerPoint() const			{ return bytesPerPoint; }
	uint getFlags() const					{ return flags; }
	uint getNumPoints() const				{ return numPoints; }
	uint getFullNumPoints() const			{ return fullNumPoints; }
	uint getUser0() const					{ return user0; }
	uint getUser1() const					{ return user1; }
	void setUser0( ubyte u )				{ user0 = u; }
	void setUser1( ubyte u )				{ user1 = u; }
	ptds::DataPointer getFilePos() const	{ return filepos; }

	void operator = ( const VoxelChannelData &vc ) { memcpy( this, &vc, sizeof(VoxelChannelData) ); }

	/* io */ 
	bool writeToFile( ptds::DataSourcePtr fhandle );
	bool writeToBranch( pt::datatree::Branch *branch );
	
private:

	friend struct CloudChannelData;
	friend class UserChannel;
	friend OOCFile;

	bool allocate();
	void destroy(bool dataOnly =false);

	ubyte *data;
	ubyte *uniform_value;
	ubyte *max_value;
	ubyte *min_value;
	
	ubyte	bytesPerPoint;
	ubyte	flags;
	ubyte	user0;				// typically unused, but for layers used for full occ
	ubyte	user1;				// typically unused, but for layers used for part occ
	uint	numPoints;			// num points in this
	uint	fullNumPoints;		// full num points for leaf

	ptds::DataPointer filepos;
};

/* the point clouds data */ 
/* a collection of voxelchanneldata */ 
struct CloudChannelData
{
	CloudChannelData( const pcloud::PointCloud* cloud, uint bitsize, uint multiple, void* defaultVal, ubyte flags=0, CloudChannelData *source = NULL, UserChannel *sourceChannel = NULL, UserChannel *destChannel = NULL);
	~CloudChannelData();

	std::vector<VoxelChannelData> data;	

	uint	m_bitsize;
	uint	m_multiple;
	uint	m_flags;

	void	*m_defaultValue;
};

/* the user channel object */ 
class UserChannel
{
	UserChannel( const pt::String &name, uint bitsize, uint multiple, void* defaultVal, ubyte flags=0, pcloud::Scene *scene=0 );
	UserChannel( UserChannel *sourceChannel, const pt::String &destName, UserChannelFlags destFlags );
	UserChannel();

	~UserChannel();
	friend class UserChannelManager;	

	// removed to due to versioning issues
//	bool		writeToFile( ptds::Tracker *tracker, ptds::DataSourcePtr fhandle );	

	pt::String	m_name;
	uint		m_bitsize;
	uint		m_multiple;
	void		*m_defaultValue;
	uint		m_flags;
	
	// removed to due to versioning issues
//	static UserChannel *createFromFile( ptds::DataSourcePtr fhandle );	

public:
	typedef  ChannelMapType<pcloud::PointCloudGUID, CloudChannelData*> MapByCloud;

	const pt::String	&name() const			{ return m_name; }
	uint				bitsPerValue() const	{ return m_bitsize; }
	uint				multiple() const		{ return m_multiple; }
	uint				bitsPerPoint() const	{ return m_bitsize * m_multiple; }
	const void*			defaultValue() const	{ return m_defaultValue; }
	const uint			flags() const			{ return m_flags; }

	void				remFromChannel( const pcloud::Scene *scene );
	void				addToChannel( const pcloud::Scene *scene );

	bool				writeToBranch( pt::datatree::Branch *branch, bool copy=true );
	static UserChannel*	createFromBranch( pt::datatree::Branch *branch );

	inline VoxelChannelData *voxelChannel( VoxelID id ) 
	{ 
		MapByCloud::iterator i = m_data.find( id.pc );
		return ( i == m_data.end() ? 0 : &i->second->data[ id.voxelIndex ]);
	}
	inline const VoxelChannelData *voxelChannel( VoxelID id )  const
	{ 
		MapByCloud::const_iterator i = m_data.find( id.pc );
		return ( i == m_data.end() ? 0 : &i->second->data[ id.voxelIndex ]);
	}

	inline VoxelChannelData *voxelChannel( const pcloud::Voxel *v ) 
	{ 
		MapByCloud::iterator i = m_data.find( v->pointCloud()->guid() );
		return ( i == m_data.end() ? 0 : &i->second->data[ v->indexInCloud() ]);
	}
	inline const VoxelChannelData *voxelChannel( const pcloud::Voxel *v ) const
	{ 
		MapByCloud::const_iterator i = m_data.find( v->pointCloud()->guid() );
		return ( i == m_data.end() ? 0 : &i->second->data[ v->indexInCloud() ]);
	}

	template <class T>
	inline bool getValue( VoxelID id, int point, T &v ) const
	{
		const VoxelChannelData *ch = cloudChannel( id );
		return ch ? ch->getVal( point, v ) : false;
	}
	template <class T>
	inline bool setValue( VoxelID id, int point, T &v )
	{
		VoxelChannelData *ch = cloudChannel( id );
		return ch ? ch->setVal( point, v ) : false;
	}
	/* this is used when data is not modified, just read */ 
	void lock( VoxelChannelData* );

	/* if locked, user will try to unlock */ 
	/* unlock will load the data for the channel */ 
	/* or else if not ooc yet, the memory is allocated in-core */ 
	/* and saved ooc on update */ 
	void unlock( VoxelChannelData*, int numPoints=0 );

	/* update the OOC data if IC is changed */ 
	void update( VoxelChannelData*, int numPoints=0 );

	MapByCloud &getData(void) {return m_data;}

private:
	MapByCloud	m_data;
	OOCFile		m_ooc;
};

class UserChannelManager
{
public:
	UserChannelManager();
	~UserChannelManager();

	static UserChannelManager* instance();

	/* channels */ 
	UserChannel *createChannel( const pt::String &name, uint bitsize, uint multiple, void* defaultVal, uint flags=0 );
	UserChannel *copyChannel(UserChannel *channel, const pt::String &destName, UserChannelFlags destFlags);
	UserChannel *channelByName( const pt::String &name );

	UserChannel *createChannelFromLayers( const pt::String &name, const pcloud::Scene *scene = 0 );	// copies the edit layers into a user channel
	bool		applyChannelToLayers( const UserChannel *ch, pcloud::Scene *scene = 0 );		// reverse opp, applies the channel data to layers

	/* erase */ 
	bool eraseChannel( UserChannel *channel );
	void eraseAllChannels();

	/* rendering */ 
	bool renderAsOffset( UserChannel* channel, float blend );
	bool renderAsRGB( UserChannel* channel );
	bool renderAsTexture( UserChannel* channel, int stride );
	bool renderAsZShift( UserChannel* channel, int stride );
	
	float renderOffsetBlendValue() const;

	UserChannel *renderOffsetChannel();
	UserChannel *renderTextureChannel();
	UserChannel *renderRGBChannel();
	UserChannel *renderZChannel();

	/* persistence */ 
	int saveChannelsToFile( const pt::String &filename, int numChannels=-1, UserChannel** channels=0, ptds::DataSourcePtr *dataSourceMemory = NULL);
	int loadChannelsFromFile( const pt::String &filename, std::vector<UserChannel*> &channels, ptds::DataSource::Data *sourceBuffer = NULL, ptds::DataSource::DataSize sourceBufferSize = 0);

private:
	UserChannel *_renderOffsetChannel;
	UserChannel *_renderTextureChannel;
	UserChannel *_renderRGBChannel;
	UserChannel *_renderZChannel;

	float _renderOffsetBlend;
};
}