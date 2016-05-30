/*----------------------------------------------------------------------*/ 
/* Voxel.cpp															*/ 
/* Voxel Implementation file											*/ 
/*----------------------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004-2010									*/   
/*----------------------------------------------------------------------*/ 
/* Written by Faraz Ravi												*/ 
/*----------------------------------------------------------------------*/ 

#include "PointoolsVortexAPIInternal.h"
#include <assert.h>
#include <stack>

#include <ptcloud2/voxel.h>
#include <ptcloud2/pointcloud.h>
#include <pt/project.h>
#include <ptcloud2/bitVoxelGrid.h>
#include <ptcloud2/octreeIndexing.h>

#include <ptds/DataSourceReadSet.h>

#include <ptapi/PointoolsVortexAPI.h>

#include <ptengine/VoxelLODSet.h>

#include <ptengine/pointsscene.h>
#include <ptengine/pointspager.h>
#include <ptengine/globalpagerdata.h>
#include <random>

#ifndef POINTOOLS_POD_API
extern pointsengine::GlobalPagerData pp;
#else
pointsengine::GlobalPagerData pp;
#endif


#include <stdio.h>
#include <math.h>


#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif


using namespace pcloud;
//-----------------------------------------------------------------------------
// Voxel Constructor
//-----------------------------------------------------------------------------
Voxel::Voxel()
{
	_pointCount = 0;
	_pointcloud = 0;
	_priority = 1.0f;
	_numPointsEdited = 0;
	_usage = 0;
	_strataSpacing = 0;			

	setCurrentLOD(LOD_ZERO);
	setPreviousLOD(LOD_ZERO);
	setStreamLOD(LOD_ZERO);

	m_filepointer	= -1;
	m_fileindex		= FILE_INDEX_NULL;
	m_extents_dirty = 1;

	setLastStreamManagerIteration(0);

	memset(_channels,0, sizeof(void*)*MAX_CHANNELS);

	setResizedToStream(false);

}
//-----------------------------------------------------------------------------
// Voxel Constructor
//-----------------------------------------------------------------------------
Voxel::Voxel(const Voxel &v)
{
	/*copy datas*/ 
	Node *n = static_cast<Node*>(this);
	(*n) = *(static_cast<const Node*>(&v));

	setCurrentLOD(v.getCurrentLOD());
	m_extents_dirty = v.m_extents_dirty;
	m_filepointer = v.m_filepointer;
	m_fileindex = v.m_fileindex;

	_pointcloud = 0;
	_priority = 1.0f;
	_numPointsEdited = v._numPointsEdited;
	_usage = 0;
	_strataSpacing = v._strataSpacing;
	memcpy(_strataSize, v._strataSize, sizeof(int)*NUM_STRATA);

	memcpy(_channels, v._channels, sizeof(void*)*MAX_CHANNELS);

    memcpy(_lower, v._lower, sizeof(_lower));
    memcpy(_upper, v._upper, sizeof(_upper));

	setLastStreamManagerIteration(0);

	flag(ExtentsDirty, true);

	setResizedToStream(false);

}
//-----------------------------------------------------------------------------
// Voxel Constructor
//-----------------------------------------------------------------------------
Voxel::Voxel( float *lower, float *upper, int deep, const pt::BoundingBoxD &worldExt, uint pointcount)
: Node(lower, upper, deep, worldExt, 0), _pointcloud(0), _origextents(worldExt)
{
	_pointCount = pointcount; 
	_pointcloud = 0;
	_priority = 1.0f;
	_numPointsEdited = pointcount;

	_worldExtents = worldExt;
	_strataSpacing = 0;	
	_usage = 0;

	memset( _strataSize, 0, NUM_STRATA * sizeof(int));
	_strataSize[0] = pointcount;

	setCurrentLOD(LOD_ZERO);
	setPreviousLOD(LOD_ZERO);
	setStreamLOD(LOD_ZERO);

	m_filepointer = -1;
	m_extents_dirty = 0;
	m_fileindex = FILE_INDEX_NULL;

	flag(ExtentsDirty, false);

	setLastStreamManagerIteration(0);

	memset(_channels,0, sizeof(void*)*MAX_CHANNELS);

	setResizedToStream(false);
}

//-----------------------------------------------------------------------------
// Voxel Destructor
//-----------------------------------------------------------------------------
Voxel::~Voxel()
{
	for (int i=1; i<MAX_CHANNELS; i++) if (_channels[i]) delete _channels[i];
}

//-----------------------------------------------------------------------------
// Return the number of defined channels
//-----------------------------------------------------------------------------

unsigned int Voxel::getNumChannels(void)
{
	unsigned int numChannels = 0;

	for (int i=1; i < MAX_LOAD_CHANNELS; i++)
	{
		if (_channels[i])
		{
			numChannels++;
		}
	}

	return numChannels;
}

//-----------------------------------------------------------------------------
// Clear Data from Channels
//-----------------------------------------------------------------------------

void Voxel::clearChannels()
{
	setCurrentLOD(LOD_ZERO);
	clearRequestLODs();
	setStreamLOD(LOD_ZERO);

	for (int i=1; i<MAX_LOAD_CHANNELS; i++)
	{
		if (_channels[i])
		{
			_channels[i]->dump();
		}
	}
}

//-----------------------------------------------------------------------------
// Add a Channels
//-----------------------------------------------------------------------------
void Voxel::addChannel(Channel ch, DataType native, DataType storeas, 
					   int multiple, int allocate, double* offset, double *scaler,
					   const void*samples, uint num_samples)
{
	if (!ch) return;

	if (!_channels[ch])
		_channels[ch] = new DataChannel(native, storeas, multiple, offset, scaler, samples, num_samples);
	else 
		_channels[ch]->dump();

	assert(_pointCount);

	if (allocate < 0)
		return;

	if (!allocate)
        allocate = static_cast<int>(lodPointCount());

	if (allocate)
	{
		// SA - is it necessary to set the LOD here? 
		setCurrentLOD(LOD_ZERO);
		setStreamLOD(LOD_ZERO);
		//((float)(allocate / _pointCount)) * 255;
		_channels[ch]->allocate(allocate);
	}
} 
//----------------------------------------------------------------------------------------
// Build Filter Channel
//----------------------------------------------------------------------------------------
void Voxel::buildEditChannel(ubyte default_value)
{
	if (_channels[PCloud_Filter]) 
		return;

	_channels[PCloud_Filter] = new DataChannel(UByte8, UByte8, 1);
	_channels[PCloud_Filter]->allocate(static_cast<int>(fullPointCount()));
	
	/* initialize to 0 */ 
	int fp = static_cast<int>(fullPointCount());
	_numPointsEdited = 0;

	for (int i=0; i<fp; i++)
		((ubyte*)_channels[PCloud_Filter]->data())[i] = default_value;
}
//-----------------------------------------------------------------------------
void Voxel::destroyEditChannel()
{
	if (_channels[PCloud_Filter])
	{
		delete _channels[PCloud_Filter];
		_channels[PCloud_Filter] = 0;
		_numPointsEdited = 0;
	}
}
//----------------------------------------------------------------------------------------
// ReadChannel
//----------------------------------------------------------------------------------------
void Voxel::readChannel(Channel ch, int count,void *data)
{
	_channels[ch]->allocate(count);
	_channels[ch]->readNative(data, 0.001);
}

//----------------------------------------------------------------------------------------
// initialize Channels
//----------------------------------------------------------------------------------------
bool Voxel::initializeChannels(int newsize)
{
	int					i;							// general counter
	std::vector<int>	index;						// point ordering index
	float				density_spacing = 0.25f;	// starting value

	// defaults are ok in case of failure
	memset( _strataSize, 0, NUM_STRATA * sizeof(int));
	_strataSpacing = 0;									//ie no stratification
	_strataSize[0] = static_cast<int>(_pointCount);		// 1 strata

	/* resize channels if needed */ 
	if (newsize >0)
	{
		for (i=0;i<MAX_CHANNELS; i++)
		{
			if (_channels[i]) 
			{
				DataChannel *dc = _channels[i];
				if (!dc->resize(newsize))
					return false;
			}
		}		
		_pointCount = newsize;
	}
	
	// insertion order should not influence point selection in strata - so randomise point order to start
	_channels[PCloud_Geometry]->generateRandomIndex(index);		

	/* re-order channel using this index */ 
	for (i=0;i<MAX_CHANNELS; i++)
	{
		if (_channels[i]) 
			_channels[i]->repositionByIndex(index);
	}
	return true;	// leave in if stratification is not required - ie for current release of Vortex

	// THE FOLLOWING CODE IS WIP FOR DENSITY STRATIFICATION ON IMPORT
	// comment out starting here to remove
	// order points in strata of uniform samples to improve lower lod representations
	float				density = 1.0f;
	int					strata[ NUM_STRATA ];

	if (_pointCount > 500)
	{
		computeStrataIndex(index, strata, density_spacing);
		density = (float)strata[0] / _pointCount;
		memcpy( _strataSize, &strata[0], NUM_STRATA * sizeof(int));		// vector is contiguous, copy is ok
	}
	else 
	{
		density = 1.0f;
		strata[0] = static_cast<int>(_pointCount);
	}
	_strataSpacing = density_spacing;
	/* end comment out here */

	/* re-order channel using this index */ 
	for (i=0;i<MAX_CHANNELS; i++)
	{
		if (_channels[i]) 
			_channels[i]->repositionByIndex(index);
	}
	return true;
}


//--------------------------------------------------------------------------------------
// return current voxel channel storage requirement for a single point
//--------------------------------------------------------------------------------------
unsigned int Voxel::getVoxelPointStorageSize(void)
{
	unsigned int size = 0;
	unsigned int i;

	for (i=0;i<MAX_CHANNELS; i++)
	{
		if(_channels[i]) 
		{
			size += _channels[i]->typesize() * _channels[i]->multiple();
		}
	}

	return size;
}

//--------------------------------------------------------------------------------------
// return number of points that can be read from channels with an existing byte budget
//--------------------------------------------------------------------------------------
unsigned int Voxel::getNumPointsWithBudget(ptds::DataSize budgetBytes, ptds::DataSize &budgetNotUsed)
{
	unsigned int pointStorageSize;
															// Get point storage requirement for voxel's channels	
	if(pointStorageSize = getVoxelPointStorageSize())
	{
															// Return how much of the budget would not fit
		budgetNotUsed = budgetBytes % pointStorageSize;
															// Return conservative number of points that fit into the budget
		return static_cast<uint>(budgetBytes / pointStorageSize);
	}
															// Error, so return zero
	return 0;
}


unsigned int Voxel::getNumPointsAtLOD(LOD lod) const
{
															// Get point count with round up
	unsigned int numPoints = static_cast<unsigned int>((fullPointCount() * lod) + 0.5);
															// Return clipped to maximum number of points
	numPoints = static_cast<uint>(min(numPoints, fullPointCount()));
															// If LOD is non zero, return at least one point
	if(numPoints == 0 && lod > 0)
	{
		numPoints = 1;
	}

	return numPoints;
}


ptds::DataSource::DataSourceForm Voxel::getDataSourceForm(void) const
{
	Voxel::FileIndex file = fileIndex();
															// If file is remote
	if(pp.files[file].getFileRemote())
	{
															// Return that the data source is a networked form
		return ptds::DataSource::DataSourceFormRemote;
	}
															// Otherwise, data source is local and not networked
	return ptds::DataSource::DataSourceFormLocal;
}


//--------------------------------------------------------------------------------------
// return bytes requested
//--------------------------------------------------------------------------------------

float Voxel::getPotentialLOD(int64_t numPointsChanged)
{
	int64_t	newNumPoints;
															// If number of points after change is positive
	if((newNumPoints = lodPointCount() + numPointsChanged) > 0 && _pointCount > 0)
	{
															// Return the LOD that would contain this number of points
		return static_cast<float>(newNumPoints) / static_cast<float>(_pointCount);
	}

	return 0;
}

//--------------------------------------------------------------------------------------
// return bytes requested
//--------------------------------------------------------------------------------------
int Voxel::requestBytes() const
{
	int bytes = 0;
	LOD req = getRequestLOD() - getCurrentLOD();

	for (int i=0;i<MAX_CHANNELS; i++)
	{
		if (_channels[i]) 
			bytes += static_cast<int>(_channels[i]->typesize() * _channels[i]->multiple() * _pointCount * req);
	}
	return bytes;
}
//--------------------------------------------------------------------------------------
// resize Channel To Requested Lod
//--------------------------------------------------------------------------------------
// must use pointer to fill Channel with something after resize!
//--------------------------------------------------------------------------------------
void Voxel::lodRequestSizeInfo(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset)
{
	DataChannel *dc = _channels[ch];

	LOD req = getRequestLOD(); 

	req *= amount;

	if (!dc)
	{
		num_bytes = 0;
		num_points = 0;
		return;
	}
	
	uint prev_size = dc->size();		
	num_points = static_cast<int>(req * _pointCount - prev_size);
	num_bytes = num_points * dc->multiple() * dc->typesize();
	bytes_offset = prev_size * dc->multiple() * dc->typesize();

	if (num_points > -5 && num_points < 0)
	{
		num_bytes = 0;
		num_points = 0;
	}
}
//--------------------------------------------------------------------------------------
// resize Channel To Requested Lod
//--------------------------------------------------------------------------------------
// must use pointer to fill Channel with something after resize!
//--------------------------------------------------------------------------------------
void *Voxel::resizeChannelToRequestedLod(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset, bool multRequest)
{
	setResizedToStream(false);

//#define BLOCK_THE_SIZE 1
	num_bytes = 0;
	num_points=0;

	if (amount == 0)
	{
		_channels[ch]->dump();
															// Set Voxel LOD info (once when geometry channel is set)
		if(ch == PCloud_Geometry)
		{
			setPreviousLOD(getCurrentLOD());
			setCurrentLOD(LOD_ZERO);
		}

		setStreamLOD(LOD_ZERO);
		return 0;
	}

	LOD req = amount;

	if(multRequest)
	{
		req *= getRequestLOD();
	}

	DataChannel *dc = _channels[ch];

	if (!dc)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}
	int bytesPerPnt = dc->multiple() * dc->typesize();
	uint prev_size = dc->size();	
	uint prev_bytesize = dc->bytesize();	

	/* avoid differences of less than 8kb, unless a full load */ 
#ifdef BLOCK_THE_SIZE
	bool doNotResize = num_points ? true : false;
#else
	bool doNotResize = false;

#endif
	if (!doNotResize) num_points = static_cast<int>(req * _pointCount - prev_size);
	num_bytes = abs(num_points * bytesPerPnt);
	
#ifdef BLOCK_THE_SIZE
	/* block to 8Kb */
	int total_bytes_remaining = static_cast<int>((_pointCount - prev_size) * bytesPerPnt);

	if (!doNotResize)
	{
		num_bytes /= 8192;
		num_bytes *= 8192;
		num_bytes += 8192;
		if (num_bytes > total_bytes_remaining) num_bytes = total_bytes_remaining;
		num_points = num_bytes / bytesPerPnt;
	}
#endif
	/* continue */ 
	bytes_offset = prev_size * bytesPerPnt;

	if (num_points > -5 && num_points < 1)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}
															// Set Voxel LOD info (once when geometry channel is set)
	if(ch == PCloud_Geometry)
	{
		setPreviousLOD(getCurrentLOD());
		setCurrentLOD(req);
	}

	setStreamLOD(req);

#ifdef BLOCK_THE_SIZE	
	uint pcount = prev_size + num_points;
#else
//	uint pcount = req * _pointCount;

	uint pcount = getNumPointsAtLOD(req);

#endif

	if (_channels[ch]->resize(pcount))
	{
		num_points = _channels[ch]->size() - prev_size;
		num_bytes = abs((int)(_channels[ch]->bytesize() - prev_bytesize));
	}
	else return 0;

	/* return point to continue from if we have an increase */ 
	return num_points > 0 ? _channels[ch]->element(prev_size) : 0;
}
//--------------------------------------------------------------------------------------
// resize Channel To Requested Lod and fill with data
//--------------------------------------------------------------------------------------
int Voxel::resizeAndFillChannelToRequestedLod(Channel ch, 
												float req, 
												int &num_points, 
												uint &num_bytes, 
												uint &bytes_offset,
												ptds::DataSourcePtr &dataSrc,
												int64_t srcPos,
												void* buffer)
{

	//#define BLOCK_THE_SIZE 1
	num_bytes = 0;
	num_points=0;

	if (req == 0)
	{
		_channels[ch]->dump();

															// Set Voxel LOD info (once when geometry channel is set)
		if(ch == PCloud_Geometry)
		{
			setPreviousLOD(getCurrentLOD());
			setCurrentLOD(LOD_ZERO);
		}

		setStreamLOD(LOD_ZERO);
		return 0;
	}

//	LOD req = getRequestLOD();

//	req *= amount;

// Pip Option
/*
if(req < 1.0f / 255.0f)
{
	req = 1.0f / 255.0f;
}
*/

	DataChannel *dc = _channels[ch];

	if (!dc)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}

	int bytesPerPnt		= dc->multiple() * dc->typesize();
	uint prev_size		= dc->size();	
	uint prev_bytesize	= dc->bytesize();	

	/* avoid differences of less than 8kb, unless a full load */ 
#ifdef BLOCK_THE_SIZE
	bool doNotResize = num_points ? true : false;
#else
	bool doNotResize = false;

#endif

	if (!doNotResize)
	{
		num_points = getNumPointsAtLOD(req) - prev_size;
//		num_points = req * _pointCount - prev_size; 
	}

	num_bytes = abs(num_points * bytesPerPnt);

#ifdef BLOCK_THE_SIZE

	/* block to 8Kb */ 
	int total_bytes_remaining = static_cast<int>((_pointCount - prev_size) * bytesPerPnt);

	if (!doNotResize)
	{
		num_bytes /= 8192;
		num_bytes *= 8192;
		num_bytes += 8192;
		if (num_bytes > total_bytes_remaining) num_bytes = total_bytes_remaining;
		num_points = num_bytes / bytesPerPnt;
	}
#endif

	/* continue */ 
	bytes_offset = prev_size * bytesPerPnt;

	if (num_points > -5 && num_points < 1)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}

#ifdef BLOCK_THE_SIZE	
	uint pcount = prev_size + num_points;
#else

	uint pcount = getNumPointsAtLOD(req);
//	uint pcount = req * _pointCount;

#endif

	if (_channels[ch]->resize(pcount))
	{
		/* special hack for RGB and numPointsEdited if painted */ 
		if (ch == PCloud_RGB && flag( Painted ))
		{
			if (pcount < numPointsEdited()) numPointsEdited(pcount);
		}

		num_points = _channels[ch]->size() - prev_size;
		num_bytes = abs((int)(_channels[ch]->bytesize() - prev_bytesize));
	}
	else
	{
		return 0;
	}

	/* point to continue from if we have an increase */ 

	void* data = num_points > 0 ? _channels[ch]->element(prev_size) : 0;

	srcPos += bytes_offset;


	if (data && num_bytes && dataSrc->movePointerTo(srcPos))
	{
		unsigned int bytesRead;
															// Read now or add to ReadSet if reading is deferred
		bytesRead = static_cast<uint>(dataSrc->readBytes(data, num_bytes));

// Pip Option
/*
dataSrc->setReadSetEnabled(false);
bytesRead = dataSrc->readBytes(data, num_bytes);
dataSrc->setReadSetEnabled(true);
*/
															// If ReadSet is not used, read is not deferred
		if(dataSrc->isReadSetDefined() == false)
		{
															// Set current LOD to the request LOD now

															// Set Voxel LOD info (once when geometry channel is set)
			if(ch == PCloud_Geometry)
			{
				setPreviousLOD(getCurrentLOD());
				setCurrentLOD(req);
			}
															// Voxel channels not currently resized to receive stream data
			setResizedToStream(false);
		}
		else
		{
															// Flag that voxel channels have been resized to receive stream data
			setResizedToStream(true);
		}
															// Set Stream LOD that states how much progress is made towards the request LOD
		setStreamLOD(req);

		return bytesRead;
	}

	return 0;
}

#ifndef NO_DATA_SOURCE_SERVER

unsigned int Voxel::getVoxelDataSourceReadSet(LOD lod, ptds::DataSourceReadSet &readSet)
{
	DataChannel *	dataChannel;
	unsigned int	previousChannelSize = 0;
	int64_t			position;
	unsigned int	i;
	
	for(i = 0; i < MAX_LOAD_CHANNELS; i++)
	{
		if(dataChannel = _channels[i]) 
		{

			position = filePointer() + previousChannelSize;

			getChannelDataSourceRead(*dataChannel, position, lod, readSet);

			previousChannelSize += static_cast<uint>(fullPointCount() * dataChannel->getPointSize());
		}
	}		

	return readSet.getNumReads();
}



void Voxel::getChannelDataSourceRead(DataChannel &dataChannel, int64_t position, LOD lod, ptds::DataSourceReadSet &readSet)
{
	unsigned int	previousSize;
	unsigned int	numPoints;
	unsigned int	numBytes;
	unsigned int	bytesPerPoint;

															// Get bytes used by point
	bytesPerPoint	= dataChannel.getPointSize();
															// Get existing number of points in channel
	previousSize	= dataChannel.size();	
															// Get number of points to read

// Pip Option (Round up)
	numPoints		= getNumPointsAtLOD(lod) - previousSize;

//	numPoints		= (lod * fullPointCount()) - previousSize; 
															// Offset channel base position by existing number of points
	position	   += previousSize * bytesPerPoint;
															// Get number of bytes to read
	numBytes		= numPoints * bytesPerPoint;

	ptds::DataSourceRead read(this, dataChannel.getPointSize(), position, numBytes, NULL);

	readSet.addRead(read);

// (ClientID initClientID, ItemSize initItemSize, DataPointer initPosition, DataSize initSize, Data *initBuffer)

}


#endif

int Voxel::getChannelResizeReadInfo(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset, int64_t &srcPos)
{
	num_bytes	= 0;
	num_points	= 0;

	if(amount == 0)
	{
															// Set Voxel LOD info (once when geometry channel is set)
		if(ch == PCloud_Geometry)
		{
			setPreviousLOD(getCurrentLOD());
			setCurrentLOD(LOD_ZERO);
		}

		setStreamLOD(LOD_ZERO);

		return 0;
	}

	LOD req = getRequestLOD();

	req *= amount;

	DataChannel *dc = _channels[ch];

	if (!dc)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}

	int		bytesPerPnt		= dc->multiple() * dc->typesize();
	uint	prev_size		= dc->size();	
	uint	prev_bytesize	= dc->bytesize();

	/* avoid differences of less than 8kb, unless a full load */ 
#ifdef BLOCK_THE_SIZE
	bool doNotResize = num_points ? true : false;
#else
	bool doNotResize = false;

#endif

	if (!doNotResize)
	{
		num_points = static_cast<int>(req * _pointCount - prev_size);
	}

	num_bytes = abs(num_points * bytesPerPnt);

#ifdef BLOCK_THE_SIZE

	/* block to 8Kb */ 
	int total_bytes_remaining = static_cast<int>((_pointCount - prev_size) * bytesPerPnt);

	if (!doNotResize)
	{
		num_bytes /= 8192;
		num_bytes *= 8192;
		num_bytes += 8192;
		if (num_bytes > total_bytes_remaining) num_bytes = total_bytes_remaining;
		num_points = num_bytes / bytesPerPnt;
	}
#endif

	/* continue */ 

	bytes_offset = prev_size * bytesPerPnt;

	if (num_points > -5 && num_points < 1)
	{
		num_bytes = 0;
		num_points = 0;
		return 0;
	}

	num_points	= _channels[ch]->size() - prev_size;
	num_bytes	= abs((int)(_channels[ch]->bytesize() - prev_bytesize));

	/* point to continue from if we have an increase */ 

	void* data = num_points > 0 ? _channels[ch]->element(prev_size) : 0;

	srcPos += bytes_offset;


	if (data && num_bytes)
	{
		return num_bytes;
	}
	else
	{
		num_bytes  = 0;
		num_points = 0;

		return 0;
	}


	return 0;
}

//--------------------------------------------------------------------------------------
// CoordinateSpaceTransform (effects the transformation of point during visitation)
//--------------------------------------------------------------------------------------
void Voxel::CoordinateSpaceTransform::prepare() 
{
	if (cspace == pt::LocalSpace)
		m = mmatrix4d::identity();
	else if (cspace != pt::WorldSpace)
	{	
		cloud->registration().compileMatrix(m, cspace);
		m >>= cloud->userTransformationMatrix();
	}
	else
	{
		/* special sucker for worldspace */ 
		cloud->registration().compileMatrix(m, pt::ProjectSpace);
		m >>= cloud->userTransformationMatrix();

		/* add project to world space transformation */ 
		m >>= pt::Project3D::project().registration().matrix();
	}
};
//--------------------------------------------------------------------------------------
// Find Nearest Point Visitor
//--------------------------------------------------------------------------------------
struct FindNearest
{
	FindNearest(const pt::vector3d &p) : pnt(p), idx(-1), mindist(100000)
	{
		pnt_d.x = p.x;
		pnt_d.y = p.y;
		pnt_d.z = p.z;
	}

	void point(const pt::vector3d &v, int i, ubyte &f)
	{
		d = pnt_d.dist2(v);

		if (d<mindist)
		{
			idx = i;			
			mindist = d;
			nr.x = v.x;
			nr.y = v.y;
			nr.z = v.z;
		}		
	}

	int idx;
	double d;
	double mindist;

	pt::vector3d pnt_d;
	pt::vector3d pnt;
	pt::vector3d nr;
};
//--------------------------------------------------------------------------------------
// findNearestPoint - usually done after Node.findContainingLeaf
//--------------------------------------------------------------------------------------
// returns point index
//--------------------------------------------------------------------------------------
int Voxel::findNearestPoint(const pt::vector3d &pnt, pt::vector3d &v, double &dist) const
{	
	if (!inBounds(pnt)) return -1;
	
	FindNearest fn(pnt);
	LocalSpaceTransform lst;
	const_cast<Voxel*>(this)->_iterateTransformedPoints(fn, lst);
	v = fn.nr;
	dist = sqrt(fn.mindist);
	return (fn.idx);
}
struct ExpandBox
{
	ExpandBox(pt::BoundingBoxD &_bb) : bb(_bb) {}

	inline void point(const pt::vector3d &pt, int idx, ubyte &f)	{
		bb.expandBy(pt);
	}
	pt::BoundingBoxD &bb;
};
//--------------------------------------------------------------------------------------
// calc extents
//--------------------------------------------------------------------------------------
// If parent matrix changes, must clear extents first
//--------------------------------------------------------------------------------------
void Voxel::computeExtents()
{
	if (!_channels[PCloud_Geometry] || !flag(ExtentsDirty)) return;
	
	_worldExtents.clear();

	/* not enough points for point extents - use node extents*/ 
	if (getCurrentLOD() < 0.1)
	{
		CoordinateSpaceTransform cst( const_cast<PointCloud*>(pointCloud()), pt::WorldSpace );
		cst.prepare();

		pt::BoundingBoxD b;
		getBoundsD(b);

		pt::vector3d v, vt;
		for (int i=0; i<8; i++)
		{
			b.getExtrema(i, v);
			cst.transform(v, vt);

			_worldExtents.expand(vt);
		}
		// but don't set flag to dirty 
	}
	else
	{
		ExpandBox eb(_worldExtents);
		iterateTransformedPointsRange(eb, pt::WorldSpace, false, 0, static_cast<uint>(5e3));
	}
	flag(ExtentsDirty, false);
}
// returns number of close points
int Voxel::intersectRay(const pt::Ray<float> &ray, int *pnt_indices, float tolerance)
{
	return 0;
}

//--------------------------------------------------------------------------------------
// Find Nearest Point Visitor
//--------------------------------------------------------------------------------------
struct FindClosest
{
	FindClosest(const pt::Rayf &r, float tol) 
		: ray(r), idx(-1), mindist(100000), mint(-1.0), tolerance(tol)
	{
	}
	template <typename T>
	void point(const pt::vec3<T> &v, int i, ubyte &f)
	{
		pt::vector3 vf(v); /*cast to float */ 

		if (ray.perpDist2Pnt(vf, d, t))
		{
			if (d < tolerance && t<mindist)
			{
				idx = i;			
				mint = t;
				mindist = d;
				v.cast(nr);
			}		
		}
	}
	int idx;
	double d;
	double t;
	double mindist;
	double mint;
	float tolerance;

	pt::Rayf ray;
	pt::vector3 pnt;
	pt::vector3 nr;
};
//-----------------------------------------------------------------------------
// returns distance to the ray
//-----------------------------------------------------------------------------
int Voxel::intersectRay(const pt::Rayf &ray, double &rayPos, float tolerance)
{
	FindClosest fn(ray, tolerance);
	LocalSpaceTransform lst;
	const_cast<Voxel*>(this)->_iterateTransformedPoints(fn, lst);

	rayPos = fn.mint;
	return (fn.idx);
}
//-----------------------------------------------------------------------------
// returns a points position
//-----------------------------------------------------------------------------
bool	Voxel::pointPosition(unsigned int pntIndex, pt::vector3 &pos) const
{
	if (pntIndex > lodPointCount()) return false;

	const DataChannel*dc = _channels[PCloud_Geometry];
	if (dc->size() < pntIndex) return false;

	if (dc->typesize() == sizeof(short))
	{
		pt::vector3s spos;
		dc->getval(spos, pntIndex);
		pos.x = spos.x;
		pos.y = spos.y;
		pos.z = spos.z;
	}
	else
	{
		dc->getval(pos, pntIndex);
	}
	pos *= dc->scaler();
	pos += dc->offset();
	
	return true;
}

//-----------------------------------------------------------------------------
// returns a points egb
//-----------------------------------------------------------------------------

void Voxel::setVoxelRGB(unsigned char *rgb)
{
	DataChannel	*dc;
	
	if((dc = _channels[PCloud_RGB]) == NULL)
		return;

	unsigned int t;

	for(t = 0; t < dc->size(); t++)
	{
		dc->set(t, rgb);
	}

}

//-----------------------------------------------------------------------------
// returns a points egb
//-----------------------------------------------------------------------------
bool	Voxel::pointRGB(unsigned int pntIndex, unsigned char *rgb) const
{
	if (pntIndex > lodPointCount()) return false;

	const DataChannel*dc = _channels[PCloud_RGB];
	if (!dc || dc->size() < pntIndex) return false;

	pt::vec3<unsigned char> rgbv;
	dc->getval(rgbv, pntIndex);

	memcpy(rgb, &rgbv, sizeof(rgbv));

	return true;
}
//-----------------------------------------------------------------------------
// returns a points intensity
//-----------------------------------------------------------------------------
bool	Voxel::pointIntensity(unsigned int pntIndex, short &intensity) const
{
	if (pntIndex > lodPointCount()) return false;

	const DataChannel*dc = _channels[PCloud_Intensity];
	if (!dc || dc->size() < pntIndex) return false;

	dc->getval(intensity, pntIndex);

	return true;
}
//-----------------------------------------------------------------------------
// returns a points normal
//-----------------------------------------------------------------------------
bool	Voxel::pointNormal(unsigned int pntIndex, pt::vector3s &normal) const
{
	if (pntIndex > lodPointCount()) return false;

	const DataChannel*dc = _channels[PCloud_Normal];
	if (!dc || dc->size() < pntIndex) return false;

	dc->getval(normal, pntIndex);

	return true;
}
//
float	Voxel::strataPosToLod( float strata ) const
{ 
	int strata_i = (int)strata;
	float rem = fmod( strata, 1.0f );

	int pos=0; 
	for (int i=0; i<strata_i; i++)
	{
		pos += _strataSize[i]; 
	}
	return ((float)pos / _pointCount) + ((float)_strataSize[strata_i] * rem) / _pointCount;
}
//--------------------------------------------------------------------------------------
// Find Nearest Point Visitor
//--------------------------------------------------------------------------------------
struct ComputeDensity
{
	ComputeDensity( pt::BitVoxelGrid &grid ) : _grid(grid), _countUnique(0), _countAll(0)
	{
	}
	template <typename T>
	void point(const pt::vec3<T> &v, int i, ubyte &f)
	{
		pt::vector3 vf(v); /*cast to float */ 
		
		bool has = false;
		_grid.get(v, has);
		if (!has) ++_countUnique;
		++_countAll;
		_grid.set(v);
	}
	int _countUnique;
	int _countAll;
	pt::BitVoxelGrid &_grid;
};
//-----------------------------------------------------------------------------
// HashSet
//
class HashPointSet
{
public:
	HashPointSet( float spacing = 0.01f ) : _spacing( spacing ) {};

	void setSpacing( float val )	{ _spacing = val; }
	typedef uint64_t HashVal;
	
	void	clear()	{	_hashSet.clear(); }
	

	bool	add( const HashVal &val )
	{
		if (_hashSet.find( val ) != _hashSet.end())
		{	
			return false; // already exists
		}
		else 
		{
			_hashSet.insert( val );
			return true;
		}
	}
	bool	add( const pt::vector3 &point, bool &valid )
	{
		HashVal	val;
		valid = true;

		if (!hashPoint( point, val ))
		{
			valid = false;
			return false;
		}
		else
		{
			valid = true;
			return add( val );
		}
	}
	bool exists( const HashVal &val ) const 
	{
		return (_hashSet.find( val ) != _hashSet.end()) ? true : false;
	}

	bool exists( const pt::vector3 &point ) const 
	{
		HashVal	val;

		if (!hashPoint( point, val ))
		{
			return false;
		}
		else
		{
			return exists( val );
		}
	}

	bool hashPoint(const pt::vector3 &pt, HashVal &hash) const
	{
		pt::vector3i quantize_multiples(static_cast<int>(pt.x / _spacing), 
                                        static_cast<int>(pt.y / _spacing), 
                                        static_cast<int>(pt.z / _spacing));

		hash =  (((uint64_t)quantize_multiples.x) & 0x1fffff);
		hash |= (((uint64_t)quantize_multiples.y) << 21) & 0x3FFFFE00000;
		hash |= (((uint64_t)quantize_multiples.z) << 42) & 0x7FFFFC0000000000;
		return true;
	}

	float distToNearestValue(const pt::vector3 &pt) const	// approx dist, not cartesian
	{
		float d[] =
		{	(float)std::abs(fmod(pt.x, _spacing)) / _spacing,
			(float)std::abs(fmod(pt.y, _spacing)) / _spacing,
			(float)std::abs(fmod(pt.z, _spacing)) / _spacing};		// normalise to make simple
		
		return  0.33333f * _spacing *
			(fabs( d[0] - 0.5f ) + fabs( d[1] - 0.5f ) + fabs( d[2] - 0.5f ));	// return dist where -0.5*spacing < d < 0.5*spacing
	}
		
	inline float spacing() const { return _spacing; };
private:
	typedef	std::set< HashVal>		HashSet;
	float							_spacing;
	HashSet							_hashSet;	// point has value / point index
};

//--------------------------------------------------------------------------------------
// Strata Ordering
//--------------------------------------------------------------------------------------
// Generates an index to position points in to strata
// Each strata gives an overal representation of the points in this voxel
// This is done by using a spatial uniform filtering to ensure that only one 
// point is entered with the a cube (spacing x spacing x spacing) ie voxelisation. 
// Each strata reduces the spacing value by half, thereby enabling us to extract
// representations (approximations) of points at a given spacing - rather than equally 
// sized strata
//
// However this method does not bias points that are more central to each cube to be the
// ones that are chosen for that strata - the result would therefore be balanced strata but
// without uniformity, ie points would not be gridded in positioning which is not so good for
// lod and causes gaps to appear when the lod is just right. Unformity is approximately 
// achieved by running the algorithm 4 times, each time a point is rejected if it is 
// too far from the centre of the cube. The 'too far' distance (maxdist) is increased on
// each iteration to ensure that all points make it into a strata. hasEntered[] keeps track
// of points that have been entered in a previous iteration
//--------------------------------------------------------------------------------------
struct StrataOrdering
{
	StrataOrdering( Voxel*voxel, float spacing ) 
	{
		uspacing = spacing;
		useOverflow = false;
		for (int s=0;s<NUM_STRATA-1;s++)
		{
			hps[s].setSpacing( spacing );
			spacing *= 0.5f;
		}
		extents = voxel->extents();

		pointsEntered = 0;
		corner = pt::vector3( static_cast<float>(voxel->extents().lx()), 
                              static_cast<float>(voxel->extents().ly()), 
                              static_cast<float>(voxel->extents().lz()));
		hasEntered = new bool[ voxel->fullPointCount() ];
		memset(hasEntered, 0, sizeof(bool) * voxel->fullPointCount() );
	}
	void setMaxDist( float md ) { maxDist = md; }
	void setUseOverflow( bool use ) { useOverflow = use; } // overflow is last strata when other entries fail
	~StrataOrdering()
	{
		for (int s=0;s<NUM_STRATA-1;s++)
			hps[s].clear();
		delete [] hasEntered;
	}
	bool isOnEdge( const pt::vector3 &p, float tolerance ) const
	{
		return  (	p.x - extents.lx() < tolerance
			|| p.y - extents.ly() < tolerance
			|| p.z - extents.lz() < tolerance
			|| extents.ux() - p.x < tolerance
			|| extents.uy() - p.y < tolerance
			|| extents.uz() - p.z < tolerance)
			? true : false;
	}
	template <typename T>
	void point(const pt::vec3<T> &v, int i, ubyte &f)
	{
		int edgeFlip = 0;

		if (hasEntered[i]) return;	// already entered in previous iteration

		pt::vector3 vf(v); //cast to float  
		//vf -= corner;
		bool didInsert = false;

		HashPointSet::HashVal hash, sshash;
		
		float maxDistInStrata = maxDist;

		for (int s=0;s<NUM_STRATA-1;s++)
		{
			float dist = hps[s].distToNearestValue(vf);
 
			//
			{
				// check proximity to edge to avoid doubling on edge due to neighbouring voxels
				if (isOnEdge(vf, hps[s].spacing()) && edgeFlip %1) continue;
				++edgeFlip;

				hps[s].hashPoint(vf,hash);
				newPt = !hps[s].exists( hash );
					
				if (newPt)
				{
					if (dist > maxDistInStrata)		break;

					hps[s].add( hash );
					/* must add to remaining hashsets */
					for (int j=s+1; j<NUM_STRATA-1; j++)
					{
						hps[j].hashPoint(vf,sshash);				
						hps[j].add( sshash );
					}
				
					strata[s].push_back(i);
					hasEntered[i] = true;		// need to track to prevent double entry in future iterations
					++pointsEntered;			// for debugging and to prevent iterations when pointsEntered = voxel->fullPointCount()
					didInsert = true;
					break;
				}
				maxDistInStrata *= 0.5f;
			}
		}
		if (useOverflow && !didInsert)	// only in last iteration
		{
			strata[NUM_STRATA-1].push_back(i);	// last one has remaining points
		}
	}
	bool				valid;
	bool				newPt;
	bool				*hasEntered;
	bool				useOverflow;

	pt::vector3			corner;
	float				maxDist;
	HashPointSet		hps[NUM_STRATA];
	std::vector<int>	strata[NUM_STRATA];;
	pt::BoundingBoxD	extents;
	float uspacing;
	int					pointsEntered;
};


struct UniformOrderingBitOctree
{
	UniformOrderingBitOctree( float lx, float ly, float lz, float dimx, float dimy, float dimz, float spacing ) 
	{
		_countAll = 0;
		dimx += spacing * 2;
		dimy += spacing * 2;
		dimz += spacing * 2;
		for (int i=0; i<NUM_STRATA; i++)
			grids[i] = new pt::BitVoxelGrid( lx - spacing, ly - spacing, lz - spacing, 
			dimx, dimy, dimz, spacing );
		col = 0;
	}
	~UniformOrderingBitOctree()
	{
		for (int i=0; i<NUM_STRATA; i++)
			delete grids[i];
	}
	template <typename T>
	void point(const pt::vec3<T> &v, int index, ubyte &f)
	{
		pt::vector3 vf(v); //cast to float  
		
		bool didInsert = false;

		for (int i=0; i<NUM_STRATA; i++)
		{
			bool has = false;
			bool res = grids[i]->get(v, has);

			if (!has && res)
			{
				strata[i].push_back( _countAll );
				didInsert = true;
				grids[i]->set(v);
				break;
			}
		}
		if (!didInsert) last.push_back( _countAll );

		++_countAll;
	}
	ubyte*						col;// used for debugging only
	pt::BitVoxelGrid			*grids[NUM_STRATA];
	std::vector<int>			strata[NUM_STRATA];
	std::vector<int>			last;
	int _countAll;
};

//--------------------------------------------------------------------------------------
// Find Nearest Point Visitor
//--------------------------------------------------------------------------------------
struct ComputeLocalBounds
{
	template <typename T>
	void point(const pt::vec3<T> &v, int i, ubyte &f)
	{
		bb.expandBy( v );
	}
	pt::BoundingBox bb;
};

/*****************************************************************************/
/**
* @brief
* @param index
* @return void
*/
/*****************************************************************************/
int		Voxel::computeStrataIndex(std::vector<int> &index, int *strata/*[NUM_STRATA]*/, float strataSpacing)	
{
	/*randomize order of points*/ 
    std::random_device rd;
    std::mt19937 _rng(rd());

	for (int i=0; i<NUM_STRATA; i++)
		strata[i] =0;


	index.clear();
	index.reserve(_pointCount);

	// if this is being done and not all points are loaded and available
	// then stratification is not possible
	if (_channels[PCloud_Geometry]->size() < _pointCount)
	{
		std::shuffle(index.begin(), index.end(), _rng);
		for (int i=0; i<_pointCount; i++) index.push_back(i);
        strata[0] = static_cast<int>(index.size());
		return 1;		// we need all points available to do stratification
	}

	// stratified point order
	ComputeLocalBounds	bounds;
	LocalSpaceTransform	lst;
	
	StrataOrdering			uniform( this, strataSpacing );

	// debug test only - will colour first strata
	//uniform.col = _channels[ PCloud_RGB ] ? _channels[ PCloud_RGB ]->data() : 0;

	float maxDistCoeff = 0.0156276f;
		
	for (int dst=0; dst<6; dst++)
	{
		if (dst == 5) uniform.setUseOverflow( true );
		uniform.setMaxDist( maxDistCoeff * strataSpacing );
		_iterateTransformedPoints( uniform, lst );
		maxDistCoeff *= 2.0f;
	}
	int i= 0;

	// transfer strata into the index - each one is randomised to avoid patterns formed by insertion order
	for (i=0; i<NUM_STRATA; i++)
	{		
		strata[i] = static_cast<int>(uniform.strata[i].size());

		if (!strata[i]) continue;

		std::shuffle(uniform.strata[i].begin(), uniform.strata[i].end(), _rng);
		index.insert( index.end(),	uniform.strata[i].begin(), uniform.strata[i].end() );
	}

	//if (uniform.last.size())
	//{
	//	std::random_shuffle(uniform.last.begin(), uniform.last.end(), rng);
	//	index.insert( index.end(), uniform.last.begin(), uniform.last.end() );
	//}
	
	// count how many strata are used
	int numStrataUsed = 0;
	for (int i=0; i<NUM_STRATA; i++)
		if (uniform.strata[i].size()) ++numStrataUsed;
	
	return numStrataUsed;
}
//-----------------------------------------------------------------------------
// set strat sizes
//-----------------------------------------------------------------------------
void Voxel::setStrataSizes( int numStrata, int *strata, float spacing )
{
	if (numStrata > NUM_STRATA)
		memcpy(_strataSize, strata, NUM_STRATA *sizeof(int));
	else
		memcpy(_strataSize, strata, numStrata *sizeof(int));

	_strataSpacing = spacing;
}


pcloud::Voxel::LODComparison Voxel::compareLOD(LOD a, LOD b)
{
	unsigned int numPointsA, numPointsB;

	numPointsA = getNumPointsAtLOD(a);
	numPointsB = getNumPointsAtLOD(b);

	if(numPointsA < numPointsB)
		return LODLess;

	if(numPointsA > numPointsB)
		return LODGreater;

	return LODEqual;
}

void Voxel::clearRequestLODs(void)
{
															// Clear all LODs
	voxelLODSet.clear();
}

unsigned int Voxel::getNumRequestLODs(void)
{
	return voxelLODSet.getNumEntries();
}

void Voxel::clipRequestLODMax(LOD maxLOD)
{
	voxelLODSet.clipRequestLODMax(maxLOD);
}

void Voxel::setRequestLOD(LOD lod)
{
															// Request LOD Client IDs are bound to Viewport IDs
															// Add the request LOD for the current viewport
#ifndef POINTOOLS_POD_API
	voxelLODSet.setRequestLOD(ptCurrentViewport(), lod);
#endif
}


Voxel::LOD Voxel::getRequestLOD(void) const
{
#ifndef POINTOOLS_POD_API
	return voxelLODSet.getRequestLOD(ptCurrentViewport());
#else
	return 0;
#endif
}


Voxel::LOD Voxel::getRequestLODMax(void) const
{
	return voxelLODSet.getRequestLODMax();
}


bool pcloud::Voxel::removeRequestLOD(pointsengine::VoxelLODSet::ClientID id)
{
	return voxelLODSet.remove(id);
}
