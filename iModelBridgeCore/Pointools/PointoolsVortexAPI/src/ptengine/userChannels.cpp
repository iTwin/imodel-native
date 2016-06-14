#include "PointoolsVortexAPIInternal.h"
#include <ptengine/userchannels.h>
#include <ptengine/pointsScene.h>
#include <ptengine/renderengine.h>
#include <ptengine/engine.h>

using namespace pcloud;
using namespace pointsengine;

#define BYTES_PER_ELEMENT ((m_multiple * m_bitsize / 8) ? (m_multiple * m_bitsize / 8) : 1)

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
VoxelChannelData::VoxelChannelData( uint num_points, uint bytesPerPnt, uint full_num_points, ubyte setup_flags, uint user_0, uint user_1 ) : 
		data(0), uniform_value(0), min_value(0), max_value(0), numPoints( num_points ), 
			bytesPerPoint( (ubyte)bytesPerPnt ), filepos(PT_NULL_FILE_POS), flags(setup_flags),
			fullNumPoints(full_num_points), user0((ubyte)user_0), user1((ubyte)user_1)
		{
		}
// if OOC
// The data member of the VoxelChannelData points to a buffer shared by the channel
// and does not destroy on destruct
// 
// unlock and lock methods
// ensure that the data is valid and ready to read
//-----------------------------------------------------------------------------
bool VoxelChannelData::allocate()
{
	if (data) return false;
		
	try
	{
		data = new ubyte[bytesPerPoint * numPoints];
		if (uniform_value)
		{
			for (unsigned int i=0; i<numPoints; i++)
				memcpy( &data[i*bytesPerPoint], uniform_value, bytesPerPoint );
		}
	}
	catch (...) { return false; }
	return true;
}
//-----------------------------------------------------------------------------
// VoxelChannelData : Constructor
//-----------------------------------------------------------------------------
void VoxelChannelData::destroy(bool dataOnly)
{
	if (data) delete [] data;
	data = 0;

	if (!dataOnly)
	{
		if (uniform_value) delete uniform_value;
		if (max_value) delete max_value;
		if (min_value) delete min_value;
		uniform_value = 0;
		max_value = 0;
		min_value = 0;
	}	
}

//-----------------------------------------------------------------------------
// CloudChannelData : Constructor
//-----------------------------------------------------------------------------
bool VoxelChannelData::copy(VoxelChannelData &source)
{
															// Get rid of any existing data
	destroy();

	bytesPerPoint	= source.bytesPerPoint;
	flags			= 1;
	fullNumPoints	= source.fullNumPoints;
	min_value		= NULL;
	max_value		= NULL;
	numPoints		= source.fullNumPoints;

															// Set up internal variable size values
	if(source.uniform_value)
	{
		uniform_value = new ubyte[bytesPerPoint];
		memcpy(uniform_value, source.uniform_value, bytesPerPoint);
	}

	if(source.min_value)
	{
		min_value = new ubyte[bytesPerPoint];
		memcpy(min_value, source.min_value, bytesPerPoint);
	}

	if(source.max_value)
	{
		max_value = new ubyte[bytesPerPoint];
		memcpy(max_value, source.max_value, bytesPerPoint);
	}

	if(source.getData())
	{
															// Allocate buffer
		if(allocate() == false)
		{
			return false;
		}
															// Copy buffer
		memcpy(data, source.getData(), bytesPerPoint * numPoints);
	}

															// Return copied OK
	return true;
}
//-----------------------------------------------------------------------------
// CloudChannelData : Constructor
//-----------------------------------------------------------------------------
CloudChannelData::CloudChannelData( const pcloud::PointCloud* cloud, uint bitsize, 
								   uint multiple, void* defaultVal, ubyte flags, CloudChannelData *source, UserChannel *sourceChannel, UserChannel *destChannel)
{
	m_bitsize = bitsize;
	m_multiple = multiple;
	m_flags = flags;

	m_defaultValue = 0;
	if (defaultVal)
	{
		m_defaultValue = new ubyte[BYTES_PER_ELEMENT];
		memcpy(m_defaultValue, defaultVal, BYTES_PER_ELEMENT);
	}

	/* create the voxel channel data and set to default */ 
	if (cloud) // construction from file does not have a cloud pointer
	{
        for (size_t i = 0; i < cloud->voxels().size(); i++)
		{
			const pcloud::Voxel *vox = cloud->voxels()[i];
			data.push_back( VoxelChannelData(static_cast<uint>(vox->fullPointCount()), (bitsize * multiple) / 8, static_cast<uint>(vox->fullPointCount()), flags) );
			VoxelChannelData &v = data.back();

			v.uniform_value = new ubyte[BYTES_PER_ELEMENT];
			if (m_defaultValue) 
				memcpy( v.uniform_value, m_defaultValue, BYTES_PER_ELEMENT );
															// If copying voxel user channel data
			if(source)
			{
				VoxelChannelData &sourceVoxelChannelData = source->data[i];

				if(sourceChannel && destChannel)
				{
															// Unlock source channel voxel data for access
					sourceChannel->unlock(&sourceVoxelChannelData, static_cast<uint>(vox->fullPointCount()));
															// Copy to destination
					v.copy(sourceVoxelChannelData);
															// Lock source channel voxel data
					sourceChannel->lock(&sourceVoxelChannelData);
															// Update destination voxel channel
					destChannel->update(&v, static_cast<uint>(vox->fullPointCount()));
															// Lock destination voxel channel
					destChannel->lock(&v);
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
// CloudChannelData : Desctructor
//-----------------------------------------------------------------------------
CloudChannelData::~CloudChannelData()
{
	for (size_t i=0; i<data.size(); i++)
	{
		VoxelChannelData &v = data[i];
		v.destroy();
	}
}
//-----------------------------------------------------------------------------
// Create Channels Visitor : creates the per cloud channel structures
//-----------------------------------------------------------------------------
struct CreateChannelsVisitor : public PointsVisitor
{
	CreateChannelsVisitor( UserChannel::MapByCloud &m, uint bitsize, uint multiple, void* defaultVal, ubyte flags ) 
		: _channels(m), _bitsize(bitsize), _multiple(multiple), _defaultVal(defaultVal), _flags(flags) {}

	bool cloud(pcloud::PointCloud *cloud)	
	{ 
		_channels.insert( 
			UserChannel::MapByCloud::value_type(
				cloud->guid(), new CloudChannelData( cloud, _bitsize, _multiple, _defaultVal, (ubyte)_flags )));
		return true;
	}
	UserChannel::MapByCloud &_channels;
	uint _bitsize;
	uint _multiple;
	uint _flags;

	void* _defaultVal;
};
//-----------------------------------------------------------------------------
// Create Channels Visitor : creates the per cloud channel structures
//-----------------------------------------------------------------------------
struct CopyChannelsVisitor : public CreateChannelsVisitor
{
	UserChannel *	sourceUserChannel;
	UserChannel *	destUserChannel;

	//CopyChannelsVisitor(UserChannel::MapByCloud &m, uint bitsize, uint multiple, void* defaultVal, ubyte flags) 
	//	: _channels(m), _bitsize(bitsize), _multiple(multiple), _defaultVal(defaultVal), _flags(flags) {}

	CopyChannelsVisitor(UserChannel &sourceChannel, UserChannel &destChannel, uint bitsize, uint multiple, void* defaultVal, ubyte destFlags) : CreateChannelsVisitor(sourceChannel.getData(), bitsize, multiple, defaultVal, destFlags)
	{
		sourceUserChannel	= &sourceChannel;
		destUserChannel		= &destChannel;
	}

	bool cloud(pcloud::PointCloud *cloud)	
	{ 
		UserChannel::MapByCloud::iterator it;

		if((it = _channels.find(cloud->guid())) != _channels.end())
		{
			UserChannel::MapByCloud &destChannels = destUserChannel->getData();

			destChannels.insert(UserChannel::MapByCloud::value_type(cloud->guid(), new CloudChannelData( cloud, _bitsize, _multiple, _defaultVal, (ubyte) _flags, it->second, sourceUserChannel, destUserChannel)));
		}

		return true;
	}
};
//-----------------------------------------------------------------------------
// User Channel : Constructor
//-----------------------------------------------------------------------------
UserChannel::UserChannel(const pt::String &name, uint bitsize, uint multiple, void* defaultVal, ubyte flags, pcloud::Scene *scene )
{
	m_name = name;
	m_bitsize = bitsize;
	m_multiple = multiple;
	m_defaultValue = 0;
	m_flags = flags;

	if (defaultVal)
	{
		m_defaultValue = new ubyte[BYTES_PER_ELEMENT];
		memcpy(m_defaultValue, defaultVal, BYTES_PER_ELEMENT);
	}

	CreateChannelsVisitor v( m_data, bitsize, multiple, defaultVal, flags );

	/* iterate clouds and enter this in */ 
	if (scene)
	{
		for (uint c=0;c<scene->size();c++)
			v.cloud( scene->cloud(c) );
	}
	else
	{
		thePointsScene().visitPointClouds( &v );
	}
}
//-----------------------------------------------------------------------------
// User Channel : Constructor
//-----------------------------------------------------------------------------
UserChannel::UserChannel(UserChannel *sourceChannel, const pt::String &destName, UserChannelFlags destFlags)
{
	if(sourceChannel == NULL)
		return;

	m_name			= destName;
	m_bitsize		= sourceChannel->m_bitsize;
	m_multiple		= sourceChannel->multiple();
	m_flags			= destFlags;
	m_defaultValue	= NULL;

	if(sourceChannel->defaultValue())
	{
		if((m_defaultValue = new ubyte[BYTES_PER_ELEMENT]) == NULL)
		{
			return;
		}

		memcpy(m_defaultValue, sourceChannel->defaultValue(), BYTES_PER_ELEMENT);
	}

	/* iterate clouds and enter this in */ 
	CopyChannelsVisitor v(*sourceChannel, *this, m_bitsize, m_multiple, m_defaultValue, (ubyte)destFlags);

	thePointsScene().visitPointClouds( &v );
}
//-----------------------------------------------------------------------------
// User Channel : Destructor
//-----------------------------------------------------------------------------
UserChannel::~UserChannel()
{
	if (m_defaultValue) delete m_defaultValue;
	for (MapByCloud::iterator i = m_data.begin(); i!= m_data.end(); i++)
	{
		CloudChannelData* c = i->second;
		if (c) delete c;
	}
}
//=============================================================================
// Out of core lock, unlock and update
//=============================================================================
void UserChannel::lock( VoxelChannelData *vcd )
{
	if (!vcd) return;

	if (m_flags && UserChannel_OOC)
		m_ooc.create(this);			// create OOC file if it does not already exist

	if (vcd->isOOC() && !vcd->isUniform())	// cleans up local data if it should be OOC
	{
		vcd->destroy(true);
	}
}
//-----------------------------------------------------------------------------
// Unlock the channel for access : only has affect if OOC
//-----------------------------------------------------------------------------
void UserChannel::unlock( VoxelChannelData *vcd, int numPoints )
{
	assert(vcd);
	if (!vcd) return;

	if (vcd->isOOC() && !vcd->isUniform())
	{
		/* read from OOC if available */ 
		if (!m_ooc.readVCD( vcd, numPoints ))
		{
			/*otherwise create IC for write, OOC will be creaed on first use */ 
			vcd->numPoints = numPoints;
			vcd->allocate();
		}
	}
}
//-----------------------------------------------------------------------------
//	update the user channel OOC store with any changes IC
//-----------------------------------------------------------------------------
void UserChannel::update( VoxelChannelData *vcd, int numPoints )
{
	if (vcd && vcd->isOOC() && !vcd->isUniform())
	{
		m_ooc.create( this );				// ensure the the OOC is setup already
		m_ooc.writeVCD( vcd, numPoints );	
	}	
}
//=============================================================================
//	PERSISTANCE
//=============================================================================
//	write user channel to file
//-----------------------------------------------------------------------------
/*bool UserChannel::writeToFile( ptds::Tracker *tracker, ptds::DataSourcePtr fhandle )
{
	ptds::WriteBlock wb( fhandle, 65536, tracker );

	// Note that a version check on reading was only added recently. This means that
	// this file format is fixed and cannot be added to or changed without
	// breaking loading of newer UserChannel files into older versions of Vortex	
	int version = 1;
	int bytesPerPoint = (m_bitsize / 8) * m_multiple;

	// Meta Data
	bool res = 
			wb.write( version )
		&&	wb.write( m_name )
		&&	wb.write( m_bitsize )
		&&	wb.write( m_multiple );

	if (m_defaultValue)	
	{
		res &=	wb.write( (int)1 )	// marker to indicate default val exists
				&& wb.write( m_defaultValue, bytesPerPoint);
	}
	else wb.write( (int)0 );
	

	res &= wb.write( m_flags );
	res &= wb.write( (int)m_data.size() );	// num of point clouds

	//Channel Data
	if (!res) return false;

	for (MapByCloud::iterator i = m_data.begin(); i!= m_data.end(); i++)
	{
		CloudChannelData* c = i->second;
		
		res &= wb.write ( i->first );				// Point cloud GUID
		res &= wb.write ( (int)c->data.size() );	// per voxel channel data size

		pcloud::PointCloud * cloud = thePointsScene().cloudByGUID( i->first );

		if (!cloud) continue;

		for (int i=0; i< c->data.size(); i++)
		{
			VoxelChannelData &chd = c->data[i];

			unlock( &chd, chd.fullNumPoints );	// unlock to ensure all points are loaded for write
			
			res &= wb.write( chd.fullNumPoints );	
			res &= wb.write( chd.numPoints );	
			res &= wb.write( chd.flags );
			
			uint values = 0;
			
			if (chd.uniform_value)	values |= 1;	// these may be null so need a marker
			if (chd.min_value)		values |= 2;
			if (chd.max_value)		values |= 4;

			res &= wb.write( values );
			res &= chd.uniform_value ?  wb.write( chd.uniform_value, bytesPerPoint ) : true;	// write if available
			res &= chd.min_value ?  wb.write( chd.min_value, bytesPerPoint ) : true; 
			res &= chd.max_value ?  wb.write( chd.max_value, bytesPerPoint ) : true; 

			if (chd.data)	res &= wb.write( (int) 1 );	//marker for data
			else			wb.write( (int) 0 );		//or null data

			if (chd.data)	// write the perpoint channel data
				res &= wb.write( chd.data, chd.fullNumPoints * chd.bytesPerPoint );
			
			lock( &chd );				// relock

			if (!res) return false;
		}
	}
	return true;
}	*/
//-----------------------------------------------------------------------------
bool UserChannel::writeToBranch( pt::datatree::Branch *branch, bool copy )
{
	int version = 2;
	int bytesPerPoint = (m_bitsize / 8) * m_multiple;

	// Meta Data
	branch->addNode("version", version);
	branch->addNode("name", m_name );
	branch->addNode("bitsize", m_bitsize );
	branch->addNode("multiple", m_multiple );
		
	if (m_defaultValue)	
	{
		branch->addBlob("default_value", bytesPerPoint, m_defaultValue, true, true );
	}
	branch->addNode("flags", m_flags );
	branch->addNode("num_clouds", (int)m_data.size() );
	
	pt::datatree::Branch *clouds = branch->addBranch("clouds");

	for (MapByCloud::iterator i = m_data.begin(); i!= m_data.end(); i++)
	{
		CloudChannelData* c = i->second;

		pcloud::PointCloud * cloud = thePointsScene().cloudByGUID( i->first );
		if (!cloud) continue;

		pt::datatree::Branch *cl = clouds->addIndexedBranch();

		cl->addNode("short_guid", i->first );
		cl->addNode("num_leaves", (int)c->data.size() );

		pt::datatree::Branch *leaves = cl->addBranch("leaves");

		for (size_t j=0; j< c->data.size(); j++)
		{
			VoxelChannelData &chd = c->data[j];
			
			pt::datatree::Branch *leaf = leaves->addIndexedBranch();

			unlock( &chd, chd.fullNumPoints );	// unlock to ensure all points are loaded for write
			
			leaf->addNode( "full_num_points", chd.fullNumPoints );
			leaf->addNode( "num_points", chd.numPoints );
			leaf->addNode( "flags", (uint)chd.flags );
			leaf->addNode( "user0", (uint)chd.getUser0() );
			leaf->addNode( "user1", (uint)chd.getUser1() );

			if (chd.uniform_value)
				leaf->addBlob( "uniform", bytesPerPoint, chd.uniform_value, true, true );

			if (chd.min_value)
				leaf->addBlob( "min", bytesPerPoint, chd.min_value, true, true );

			if (chd.max_value)
				leaf->addBlob( "max", bytesPerPoint, chd.max_value, true, true );			
			
			if (chd.data)	// write the per-point channel data
				leaf->addBlob( "data", chd.fullNumPoints * chd.bytesPerPoint, chd.data, copy, true, false/*compress*/);
				
			lock( &chd );				// relock
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
// User Channel construct from file handle
//-----------------------------------------------------------------------------
UserChannel::UserChannel() :  m_multiple(0), m_defaultValue(nullptr), m_flags(0)
{
}
//-----------------------------------------------------------------------------
/*UserChannel *UserChannel::createFromFile( ptds::DataSourcePtr fhandle )
{
	UserChannel *userChannel = new UserChannel();

	ptds::ReadBlock rb( fhandle, 0 );

	int version = 0;
	int hasVal;
	int numCHD = 0;

	rb.read( version );	//basic header

	// Note that this version check was only added recently. This means that
	// this file format is fixed and cannot be added to or changed without
	// breaking loading of newer UserChannel files into older versions of Vortex
	if (version != 1)   
		return NULL;

	rb.read( userChannel->m_name );
	rb.read( userChannel->m_bitsize );
	rb.read( userChannel->m_multiple );
	rb.read( hasVal );	// has default val ?

	int bytesPerPoint = (userChannel->m_bitsize / 8) * userChannel->m_multiple;

	if (hasVal)		// default value
	{
		userChannel->m_defaultValue = new ubyte[bytesPerPoint];
		rb.read( userChannel->m_defaultValue, bytesPerPoint);
	}
	
	rb.read( userChannel->m_flags );
	rb.read( numCHD );

	for (int i=0; i<numCHD; i++)
	{
		int numVoxels =0;
		pcloud::PointCloudGUID guid;

		rb.read( guid );
		rb.read( numVoxels );

		// create the per cloud channel structure
		CloudChannelData * ccd = new CloudChannelData( 0, userChannel->m_bitsize, userChannel->m_multiple, 
			userChannel->m_defaultValue, userChannel->m_flags );

		userChannel->m_data.insert( MapByCloud::value_type( guid, ccd ) );

		for (int j=0; j<numVoxels; j++)
		{
			ptds::DataPointer filepos;
			ubyte flags;
			uint numPoints, fullNumPoints;
			uint user0 = 0, user1 = 0; // note that this is incorrect, but due to the version issue mentioned above, user0 and user1 cannot be written or read from file

			rb.read( fullNumPoints );	
			rb.read( numPoints );	
			rb.read( flags );			

			ccd->data.push_back( VoxelChannelData(numPoints, bytesPerPoint, fullNumPoints, flags, user0, user1) );
			VoxelChannelData &chd = ccd->data.back();
			/* do not set filepos here */ 

		/*	uint values = 0;
			rb.read( values );

			
			if (values & 1)
			{
				chd.uniform_value = new ubyte[bytesPerPoint];
				rb.read( chd.uniform_value, bytesPerPoint );
			}
			if (values & 2)
			{
				chd.min_value = new ubyte[bytesPerPoint];
				rb.read( chd.min_value, bytesPerPoint );
			}
			if (values & 4)
			{
				chd.max_value = new ubyte[bytesPerPoint];
				rb.read( chd.max_value, bytesPerPoint );
			}
			
			rb.read( hasVal );

			if (hasVal)		// read the point data
			{
				if ( chd.allocate() )
					rb.read( chd.data, chd.numPoints * chd.bytesPerPoint );
				else
					rb.advance( chd.numPoints * chd.bytesPerPoint );

				userChannel->update( &chd, chd.numPoints );
				userChannel->lock( &chd );	// clean local store
			}
		}
	}
	return userChannel;
}*/
//-----------------------------------------------------------------------------
/* static */
UserChannel * UserChannel::createFromBranch( pt::datatree::Branch *branch )
{
	UserChannel *userChannel = new UserChannel();

	int version = 0;
	//int hasVal;
	int num_clouds = 0;

	// Meta Data
	branch->getNode("version", version);
	if (version > 2)
		return NULL;
	branch->getNode("name", userChannel->m_name );
	branch->getNode("bitsize", userChannel->m_bitsize );
	branch->getNode("multiple", userChannel->m_multiple );
		
	int bytesPerPoint = (userChannel->m_bitsize / 8) * userChannel->m_multiple;

	const pt::datatree::Blob *default_val = branch->getBlob("default_value");

	if (default_val)	
	{
		userChannel->m_defaultValue = new ubyte[ default_val->_size ];
		memcpy( userChannel->m_defaultValue, default_val->_data, default_val->_size );
	}

	branch->getNode("flags", userChannel->m_flags );
	branch->getNode("num_clouds", num_clouds );
	
	const pt::datatree::Branch *clouds = branch->getBranch("clouds");

	if (!clouds)
	{
		delete userChannel;
		return 0;
	}

	for (int i=0; i<num_clouds; i++)
	{
		const pt::datatree::Branch *cl = clouds->getIndexedBranch(i+1);

		if (!cl) continue;

		int numVoxels =0;
		pcloud::PointCloudGUID guid;

		cl->getNode("short_guid", guid );
		cl->getNode("num_leaves", numVoxels );

		pt::datatree::Branch *leaves = cl->getBranch("leaves");

		if (!leaves) continue;

		// create the per-cloud structure
		CloudChannelData * ccd = new CloudChannelData( 0, userChannel->m_bitsize, userChannel->m_multiple, 
			userChannel->m_defaultValue, (ubyte)userChannel->m_flags );
		
		//add using guid as key
		userChannel->m_data.insert( MapByCloud::value_type( guid, ccd ) );

		// for each leaf node read data
		for (int j=0; j< numVoxels; j++)
		{
			const pt::datatree::Branch *leaf = leaves->getIndexedBranch(j+1);

			if (!leaf) continue;
			
			// get meta for voxel channel data
			uint flags;
			uint numPoints, fullNumPoints;
			uint u0=0, u1=0;

			leaf->getNode( "full_num_points", fullNumPoints );
			leaf->getNode( "num_points", numPoints );
			leaf->getNode( "flags", flags );
			leaf->getNode( "user0", u0 );
			leaf->getNode( "user1", u1 );
			
			// add this to the cloud channel
			ccd->data.push_back( VoxelChannelData(numPoints, bytesPerPoint, fullNumPoints, (ubyte)flags/*, u0, u1*/) );
			VoxelChannelData &chd = ccd->data.back();
			chd.setUser0( (ubyte) u0 );
			chd.setUser1( (ubyte) u1 );

			// read uniforms, min and max
			const pt::datatree::Blob *uniform_val = leaf->getBlob("uniform");
			
			//default value
			if (uniform_val)	
			{
				chd.uniform_value = new ubyte[ uniform_val->_size ];
				memcpy( chd.uniform_value, uniform_val->_data, uniform_val->_size );
			}
			const pt::datatree::Blob *min_val = leaf->getBlob("min");

			// min value
			if (min_val)	
			{
				chd.min_value = new ubyte[ min_val->_size ];
				memcpy( chd.min_value, min_val->_data, min_val->_size );
			}
			const pt::datatree::Blob *max_val = leaf->getBlob("max");

			// max value
			if (max_val)	
			{
				chd.max_value = new ubyte[ max_val->_size ];
				memcpy( chd.max_value, max_val->_data, max_val->_size );
			}
			
			// per-point data
			const pt::datatree::Blob *data = leaf->getBlob( "data" );
			if (data)
			{
				chd.allocate();
				memcpy( chd.data, data->_data, data->_size );
			}
			userChannel->update( &chd, chd.numPoints );			
			userChannel->lock( &chd );				// clean local store
		}
	}
	return userChannel;
}

//-----------------------------------------------------------------------------
void UserChannel::remFromChannel( const pcloud::Scene *scene )
{
	for (uint i=0; i<scene->size(); i++)
	{
		const pcloud::PointCloud *pc = scene->cloud(i);

		MapByCloud::iterator f = m_data.find( pc->guid() );

		if (f != m_data.end())
		{
			CloudChannelData *ch = f->second;
			
			for (size_t j=0; j<ch->data.size(); j++)
				ch->data[j].destroy();
			
			m_data.erase(f);
		}
	}
}
//-----------------------------------------------------------------------------
void UserChannel::addToChannel( const pcloud::Scene *scene )
{
	for (uint i=0; i<scene->size(); i++)
	{
		const pcloud::PointCloud *cloud = scene->cloud(i);

		m_data.insert( 	UserChannel::MapByCloud::value_type(
				cloud->guid(), new CloudChannelData( cloud, m_bitsize, m_multiple, m_defaultValue, (ubyte)m_flags )));
	}
}

//=============================================================================
// OOCFile handling
//=============================================================================
static pt::String s_tempFolder;


//-----------------------------------------------------------------------------
// ~OOCFile
//-----------------------------------------------------------------------------
OOCFile::~OOCFile(void)
{
															// Destroy OOC file when destructor called
	destroy();
}

//-----------------------------------------------------------------------------
// set the OOC Folder ( Static )
//-----------------------------------------------------------------------------
void OOCFile::setOOCFileFolder( const pt::String &folder )
{
	s_tempFolder = folder;
}
//-----------------------------------------------------------------------------
// return the OOC Folder ( Static )
//-----------------------------------------------------------------------------
const pt::String &OOCFile::getOOCFileFolder()
{
	return s_tempFolder;
}
//-----------------------------------------------------------------------------
//	Create the OOC file
//-----------------------------------------------------------------------------
void OOCFile::create( class UserChannel *uchannel )
{
#ifdef BENTLEY_WIN32    //NEEDS_WORK_VORTEX_DGNDB
	if (!fhandle)
	{
		wchar_t filename[MAX_PATH];
		wchar_t path[MAX_PATH];

		if (s_tempFolder.length())
		{
			wcsncpy( path, s_tempFolder.c_wstr(), MAX_PATH-1 );	// user user provided folder
		}
		else
		{
			GetTempPathW(MAX_PATH-14, path);				// Default to using temporary folder
		}

		::GetTempFileNameW( path, L"uch", 0, filename );	// Generate a temporary file name

		fname = pt::String( filename );	// set fname member for delete later on

		ptds::FilePath fp(filename);
		fhandle = ptds::dataSourceManager.openForReadWrite(&fp);	// open for read

		fend = 0;	// filepointer to end of file, reset here
	}
#endif
}
//-----------------------------------------------------------------------------
// destroy the OOC File
//-----------------------------------------------------------------------------
void OOCFile::destroy()
{
	ptds::dataSourceManager.close(fhandle);
    BeFileName::BeDeleteFile(fname.c_wstr());
	fhandle = NULL;
	fend = 0;
	fname = "";
}
//-----------------------------------------------------------------------------
// OOCFile: write the data to file
//-----------------------------------------------------------------------------
bool OOCFile::writeVCD( class VoxelChannelData* vcd, size_t numPoints )
{
	if (!vcd->isOOC()) return false;

	/* write the channel to file */ 
	if (vcd->getData())
	{
		if (!numPoints) numPoints = vcd->getNumPoints();
		if (numPoints > vcd->getNumPoints()) numPoints = vcd->getNumPoints();

		int numBytes = static_cast<int>(numPoints * vcd->getBytesPerPoint());

		uint zeroBufferSize = 0;

		/* position in the file if needed */ 
		if (vcd->getFilePos() == PT_NULL_FILE_POS)
		{
			fhandle->movePointerTo(fend );
			vcd->filepos = fend;
			
			/* must use full point count first time to reserve enough space */ 
			fend += vcd->getBytesPerPoint() * vcd->getFullNumPoints();
		
			zeroBufferSize = static_cast<uint>((vcd->getFullNumPoints() - numPoints) * vcd->getBytesPerPoint());
		}
		else
		{
			fhandle->movePointerTo(vcd->getFilePos());
		}

		/* write actual data part */ 
		int bytesWritten = static_cast<int>(fhandle->writeBytes(vcd->getData(), numBytes ));
		
		/* write zeros if first time and numPoints < numFullPoints - maybe should be default value*/ 
		if (zeroBufferSize)
		{
			ubyte *zeroBuffer = new ubyte[zeroBufferSize];
			memset(zeroBuffer, 0, zeroBufferSize);
			fhandle->writeBytes(zeroBuffer, zeroBufferSize);
			delete [] zeroBuffer;
		}

		return (bytesWritten == numBytes) ? true : false;
	}
	return false;
}
//-----------------------------------------------------------------------------
// OOCFile : read the data from file
//-----------------------------------------------------------------------------
bool OOCFile::readVCD( class VoxelChannelData* vcd, size_t numPoints )
{
	if (!vcd->isOOC()) return false;	// only for OOC

	if (vcd->getFilePos() == PT_NULL_FILE_POS) return false;	// no valid file pointer
	
	if (numPoints) 
	{
		if (numPoints > vcd->numPoints) vcd->destroy(true);		//need larger buffer
		vcd->numPoints = static_cast<uint>(numPoints);
	}

	if (!vcd->getData()) vcd->allocate();
	
	int numBytes = static_cast<int>(numPoints * vcd->getBytesPerPoint());
	fhandle->movePointerTo((ptds::DataPointer)vcd->getFilePos());
	int bytesRead = static_cast<int>(fhandle->readBytes(vcd->getData(), numBytes ));
	return (bytesRead == numBytes) ? true : false;
}
//-----------------------------------------------------------------------------
bool VoxelChannelData::writeToBranch( pt::datatree::Branch *branch )
{
	return false; // unused
}
