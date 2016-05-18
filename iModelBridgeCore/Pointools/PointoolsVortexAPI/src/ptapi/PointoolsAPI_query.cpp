#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/PointsExchanger.h>
#include <ptengine/VisibilityEngine.h>
#include <ptengine/pointLayers.h>
#include <ptengine/userChannels.h>
#include <ptengine/renderContext.h>
#include <ptengine/engine.h>
#include <ptengine/queryScene.h>
#include <ptengine/clipManager.h>

#include <ptedit/edit.h>
 
#include <diagnostics/diagnostics.h>

#include <ptcloud2/bitvoxelgrid.h>
#include <ptcloud2/pod.h>

#ifdef HAVE_OPENGL
#include <ptgl/glcamera.h>
#include <ptgl/gltext.h>
#endif

#include <pt/project.h>
#include <pt/timer.h>

#include <math/matrix_math.h>

#include <ptengine/StreamManager.h>

#include <pt/trace.h>

using namespace pt;
using namespace pcloud;
using namespace pointsengine;

#define RAY_LARGE_DISTANCE 20000000
#define MAX_NUM_USER_CHANNELS 32
#define VORTEX_TEST_BUILD 1 //must not be defined in release
#define VORTEX_TEST_OUTPUT_PATH "E:\\vortexStats.txt"

//-----------------------------------------------------------------------------
// EXTERNS
//-----------------------------------------------------------------------------
extern pcloud::Scene*			sceneFromHandle(PThandle handle);
extern pcloud::PointCloud*		cloudFromHandle(PThandle cloud);
extern const pcloud::Scene*		constSceneFromHandle(PThandle handle);
extern const pcloud::PointCloud *constCloudFromHandle(PThandle cloud);
extern pt::ViewParams*			g_currentViewParams;
extern PTdouble					g_unitScale;
extern PTdouble					g_units;
extern int						setLastErrorCode( int );
extern UserChannel *			_ptGetUserChannel( PThandle h );
extern RenderContext*           g_currentRenderContext;

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// NULL Point Query
//-----------------------------------------------------------------------------

struct NullCondition
{
				NullCondition	(void)							{}

	static bool boundsCheck		(const pt::BoundingBoxD &box)	{ return true; }
	static bool nodeCheck		(const Node *n)					{return true;}
	static bool processWhole	(const Node *n)					{return true;}
	static bool escapeWhole		(const Node *N)					{return false;}
	static bool validPoint		(const Node *N)					{return true;}

	static bool validPoint		(const vector3d &pnt, ubyte &f)	{return true;}
	static const char* name()  { return "NULL"; }
};


namespace querydetail
{
    struct FrustumCondition;

	/* query base class */
	class Query
	{
	public:
		Query()
		{
			resetQuery();

			density = PT_QUERY_DENSITY_VIEW;
			densityCoeff = 1.0f;
			rgbMode = PT_QUERY_RGB_MODE_ACTUAL;
			gather = false;
			pointLimit = 0;
			scope = 0;
			pcloudOnly = 0;
			sceneOnly = 0;
			layerMask = 255;
		}
		//---------------------------------------------------------------------
		virtual ~Query()
		{

		}
		//---------------------------------------------------------------------
		virtual void resetQuery()
		{
			lastPnt = 0;
			lastVoxel = 0;
			voxels = 0;
		}

		void setLastPartiallyIteratedVoxel(pcloud::Voxel *voxel)
		{
			lastPartiallyIteratedVoxel = voxel;
		}

		pcloud::Voxel *getLastPartiallyIteratedVoxel(void)
		{
			return lastPartiallyIteratedVoxel;
		}

		void setLastPartiallyIteratedVoxelUnloadLOD(float amount)
		{
			lastPartiallyIteratedVoxelUnloadLOD = amount;
		}

		float getLastPartiallyIteratedVoxelUnloadLOD(void)
		{
			return lastPartiallyIteratedVoxelUnloadLOD;
		}

		//---------------------------------------------------------------------
		void setScope( PThandle cloudOrScene )
		{
			scope = cloudOrScene;
			sceneOnly = sceneFromHandle( cloudOrScene );
			pcloudOnly = cloudFromHandle( cloudOrScene );
		}
		//---------------------------------------------------------------------
		void setDensity( PTenum type, PTfloat coeff )
		{
			density = type;
			densityCoeff = coeff;
		
			if (type == PT_QUERY_DENSITY_LIMIT && coeff > 1.0f)
			{
				pointLimit = (int)coeff;
				densityCoeff = 1.0f;
			}
		}
		//
		void setLayers( PTuint layer_mask )
		{
			layerMask = layer_mask;
		}
		//---------------------------------------------------------------------
		PTenum			getDensityType() const		{ return density; }
		float			getDensityCoeff() const		{ return densityCoeff; }
		//---------------------------------------------------------------------
		void			setRGBMode( PTenum mode )	{ rgbMode = mode;	}
		//---------------------------------------------------------------------
		virtual int		runQuery(int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )=0;
		virtual int		runQuery(int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )=0;

		/** \brief Run a query that can write into an array of packed struct points. */
		virtual int		runQueryStrided(const size_t buffersize, PTfloat * const geomBuffer, const size_t geomStride, ubyte *const rgbBuffer, const size_t rgbStride) 
		{ 
			throw "No strided implementation for this query."; 
		}
		
		virtual int		runDetailedQuery( int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
			PTubyte *filter, PTubyte *classification, 
			PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )=0;

		virtual int		runDetailedQuery( int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
			PTubyte *filter, PTubyte *classification, 
			PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )=0;

		// select the points that satisfy the query, no points returned in buffers
		virtual uint	selectQueryPoints(bool select)=0;

		virtual void	submitChannelUpdate(PThandle channel)=0;

		virtual int		runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTfloat **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten) = 0;
		virtual int		runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTdouble **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten) = 0;
		
		virtual bool	doesOwnDensityLimitCompute() const { return false; } 

		//virtual int64_t computePntsInQuery()=0;

		bool			isBeingRun() const					{ return lastPnt && lastVoxel ? true : false; }
		bool			isReset() const						{ return !(isBeingRun()); }
		PTint64			getDensityLimit() const				{ return pointLimit; };

		virtual int64_t computeNumPointsInQuery()	{ return -1; }

	protected:

		pcloud::Voxel	*lastVoxel;			// last voxel query returned points from, used to continue query
		int				lastPnt;			// last point delivered, used to continue query when buffer full
		float			densityCoeff;		// den coeff, meaning depends on density type
		int64_t			pointLimit;			// total number of points limit
		bool			gather;			
		PTenum			density;			// density type
		PTenum			rgbMode;		
		int64_t			pointCount;			// point count of points returned
		PThandle		scope;				// point cloud or scene (pod) scope
		uint			layerMask;

		static pcloud::Voxel *	lastPartiallyIteratedVoxel;
		static float			lastPartiallyIteratedVoxelUnloadLOD;

		const pcloud::PointCloud			*pcloudOnly;
		const pcloud::Scene					*sceneOnly;

		std::vector<pcloud::Voxel *>		*voxels;
	
	public:
		ptdg::QueryData1					stats;
	};	// Class Query

	typedef std::map<PThandle, Query*> QueryMap;
	static QueryMap s_queries;

	pcloud::Voxel *	Query::lastPartiallyIteratedVoxel = NULL;
	float			Query::lastPartiallyIteratedVoxelUnloadLOD = 0;

	//---------------------------------------------------------------------
	/* Gather Voxels Visitor - used in ordered voxel querying for display */
	//---------------------------------------------------------------------
	template <class NodeCondition>
	class GatherVoxelsVisitor : public PointsVisitor
	{
	public:
		GatherVoxelsVisitor(NodeCondition *condition, std::vector<pcloud::Voxel *> &vec, PThandle scope)
			: voxels(vec), fullPointCount(0), lodPointCount(0), C(condition)
		{
			pcloudOnly = 0;
			sceneOnly = 0;

			if (scope)
			{
				pcloudOnly = cloudFromHandle(scope);
				sceneOnly = sceneFromHandle(scope);
			}
			world2prj.set(static_cast<float>(-pt::Project3D::project().registration().matrix()(3,0)),
                          static_cast<float>(-pt::Project3D::project().registration().matrix()(3,1)),
                          static_cast<float>(-pt::Project3D::project().registration().matrix()(3, 2)));
        };
        //---------------------------------------------------------------------
		bool scene(pcloud::Scene *sc)
		{
			if (sceneOnly && sc != sceneOnly) return false;
			return C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool cloud(pcloud::PointCloud *cloud)
		{
			if (pcloudOnly && cloud != pcloudOnly) return false;
			return C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool visitNode(const pcloud::Node *n)
		{
			objCsBounds = n->extents();
			objCsBounds.translateBy(world2prj);

			bool res = C->nodeCheck(n) && C->boundsCheck(objCsBounds);

			if (n->isLeaf() && res)
			{
				Voxel *v = static_cast<Voxel*>(const_cast<pcloud::Node*>(n));
				
				voxels.push_back( v );
				lodPointCount += v->lodPointCount();
				fullPointCount += v->fullPointCount();
			}

			return res;
		}
		//---------------------------------------------------------------------
		NodeCondition *C;
		bool voxel(pcloud::Voxel *vox) { return true; }
		std::vector<pcloud::Voxel *>	&voxels;
		int64_t lodPointCount;
		int64_t fullPointCount;
		const pcloud::PointCloud *pcloudOnly;
		const pcloud::Scene *sceneOnly;
		vector3d world2prj;
	};
	//---------------------------------------------------------------------
	// WRITE POINT CHANNELS VISITOR
	//---------------------------------------------------------------------
	//template <class NodeCondition, class T = PTdouble>
	//class WritePointChannels : public ReadPoints<NodeCondition, T>
	//{
	//public:
	//	WritePointChannels( NodeCondition	*condition, 
	//						int				geomBuffersize, 										
	//						Voxel			*continueFromVoxel	= 0, 
	//						int				continueFromPnt		= 0,
	//						PTenum			queryDensity		= PT_QUERY_DENSITY_FULL, 
	//						float			denCoeff			= 1.0f, 
	//						pt::BitVoxelGrid *grid				= 0, 			
	//						PTuint			_numPointChannels	= 0, 
	//						const PThandle  *_pointChannelsReq	= 0, 
	//						PTvoid			**_pointChannels	= 0 
	//						)
	//		: 
	//		ReadPoints<NodeCondition, T>(
	//						condition,
	//						geomBuffersize,
	//						0,0,0,
	//						true,
	//						continueFromVoxel, 
	//						continueFromPnt,
	//						queryDensity, 
	//						denCoeff, 
	//						grid, 
	//						0, 0, 0, 
	//						_numPointChannels, 
	//						_pointChannelsReq, 
	//						_pointChannels)
	//	{
	//	}

	//};
	
	template <class NodeCondition>
	class SelectPoints : public PointsVisitor
	{
	public:
		SelectPoints (	NodeCondition	*condition,
						bool			select=true, // or deselect (false)
						PTenum			queryDensity = PT_QUERY_DENSITY_FULL, 
						float			denCoeff	 = 1.0f, 
						pt::BitVoxelGrid *grid		 = 0)
		{
			counter = 0;
			density = queryDensity;
			densityCoeff = denCoeff;
			cs = pt::ProjectSpace;
			pcloudOnly = 0;
			sceneOnly = 0;
			ugrid = grid;
			spatialGridFailure = false;
			doSelect = select;

			world2prj.set(static_cast<float>(-pt::Project3D::project().registration().matrix()(3,0)),
				          static_cast<float>(-pt::Project3D::project().registration().matrix()(3,1)),
				          static_cast<float>(-pt::Project3D::project().registration().matrix()(3,2)));
                                                                                                   
		}
		//---------------------------------------------------------------------
		bool scene(pcloud::Scene *sc)
		{
			if ( sceneOnly && sc != sceneOnly ) return false;
			return C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool cloud(pcloud::PointCloud *cloud)
		{
			if ( pcloudOnly && cloud != pcloudOnly ) return false;
			return  C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool visitNode(const pcloud::Node *n)
		{
			objCsBounds = n->extents();
			objCsBounds.translateBy(world2prj);

			bool res = C->boundsCheck(objCsBounds) && C->nodeCheck(n);

			if (res && n->isLeaf())
				return voxel(static_cast<Voxel*>(const_cast<pcloud::Node*>(n)));

			return res;
		}
		//---------------------------------------------------------------------
		float voxelDensityValue( pcloud::Voxel *vox )
		{
			if (vox->densityValue() < 0.000001f)
			{
				pointsengine::VoxelLoader load(vox, 1.0f, false, true, true);
			}
			return vox->densityValue();
		}
		//---------------------------------------------------------------------
		bool voxel(pcloud::Voxel *vox)
		{
			if ( C->escapeWhole(vox) ) return true;

			bool doload = false;
			float amount = densityCoeff;

			if (vox->flag( pcloud::OutOfCore )) 
				doload = true;

			switch (density)
			{
			case PT_QUERY_DENSITY_SPATIAL:					// spatially uniform density
				amount = 1.0f;
				doload = true;
				break;

			case PT_QUERY_DENSITY_FULL:						// full density, but modulated by coeff
				doload = true;	//should be true
				break;

			case PT_QUERY_DENSITY_VIEW:						// current view based density
				amount = densityCoeff * vox->getRequestLOD();
				break;

			case PT_QUERY_DENSITY_VIEW_COMPLETE:			// current view based density with load
				amount = densityCoeff * vox->getRequestLOD();
				doload = true;
				break;

			case PT_QUERY_DENSITY_LIMIT:					// density limited to a number of points
				amount = densityCoeff;						// problem is this is the density of selected points only;
				doload = true;
				break;
			}

			currentVoxel = vox;

			/* rgb */
            std::lock_guard<std::mutex> lock(vox->mutex()); ///!ToDo: can we put this inside a nested scope with some of the code below in order the relinquish the lock ASAP? I.e.,  { boost::mutex::scoped_lock lock(vox->mutex()); vortex::VoxelLoader load( doload ? vox : 0, amount, false, false); /* ... */ ; }  /* ... more code before end of function */
			pointsengine::VoxelLoader load( doload ? vox : 0, amount, false, false); ///!Trac ticket #312 Here we feed in the overall density to be used per-voxel that can be tiny (truncate to integer zero) if the cloud is large and the query is small.

			// get channels
			DataChannel *crgb = vox->channel( pcloud::PCloud_RGB );
			DataChannel *cfilter = vox->channel( pcloud::PCloud_Filter );

			spatialGridFailure = false;
			float lod_amount = 0;

			if (vox->lodPointCount() > 0)
			{
				lod_amount = amount * (float)vox->fullPointCount() / vox->lodPointCount();	// amount is % of the lod not % of full, so we need to compensate
			}
			if (lod_amount > 1.0f)
			{
				lod_amount = 1.0f;
			}

			///!ToDo: Work out the right version of the point count to send to the condition (we should be passing the same number that we will eventually send into the in/out per point test).
			vox->iterateTransformedPoints(*this, cs, 255, lod_amount);

			if (spatialGridFailure)
			{
				return false;
			}

			return true; /* ie continue */
		}
		//---------------------------------------------------------------------
		bool point(const vector3d &pnt, int index, ubyte &f)
		{
			C->validPoint(pnt, f);

			if (doSelect)	f |= SELECTED_PNT_BIT;
			else 			f &= ~SELECTED_PNT_BIT;

			return true;
		}

		bool						doSelect;
		NodeCondition				*C;
		uint						counter;
		PTenum						density;
		float						densityCoeff;
		pt::CoordinateSpace			cs;
		pt::vector3d				world2prj;

		const pcloud::PointCloud	*pcloudOnly;		// limit scope to a pcloud. Maybe null
		const pcloud::Scene			*sceneOnly;			// limit scope to a pod, maybe null

		bool						spatialGridFailure;

		pt::BitVoxelGrid			*ugrid;				// for spatial density option only
	};
	//---------------------------------------------------------------------
	// READ POINTS VISITOR
	// extracts point channels into buffers for query
	// maintains state of data extraction to enable iterative extraction
	//---------------------------------------------------------------------
	template <class NodeCondition, class T = PTdouble>
	class ReadPoints : public PointsVisitor
	{

	protected:

		typedef std::vector<Voxel *>	VoxelPtrSet;

	public:

		ReadPoints	(void)
		{
			ReadPoints(NULL, 0, NULL);
		}

		ReadPoints( 
					NodeCondition	*condition, 
					int				geomBuffersize, 
					T				*geomBuffer, 
					PTubyte			*rgb				= 0, 
					PTshort			*inten				= 0, 
					bool			writeMode			= false, 
					Voxel			*continueFromVoxel	= 0, 
					int				continueFromPnt		= 0,
					PTenum			queryDensity		= PT_QUERY_DENSITY_FULL, 
					float			denCoeff			= 1.0f, 
					pt::BitVoxelGrid *grid				= 0, 
					PTenum			rgbType				= PT_QUERY_RGB_MODE_ACTUAL, 
					PTubyte			*_filterBuffer		= 0, 
					PTubyte			*classification		= 0, 
					PTuint			_numPointChannels	= 0, 
					const PThandle  *_pointChannelsReq	= 0, 
					PTvoid			**_pointChannels	= 0,
					pcloud::Voxel	*lastPartiallyIteratedVoxel = NULL,
					float			 lastPartiallyIteratedVoxelUnloadLOD = 0,
					PTubyte			layer_mask			= 255
					)
			: C(condition)
		{
			rwPos.reset();
			rwPos.continueFrom = continueFromVoxel;
			rwPos.continuePnt = continueFromPnt;
			rwPos.setLastPartiallyIteratedVoxel(lastPartiallyIteratedVoxel);
			rwPos.setLastPartiallyIteratedVoxelUnloadLOD(lastPartiallyIteratedVoxelUnloadLOD);
			begin = false;
			rwPos.counter = 0;
			rwPos.counterPotential = 0;
			buffer = geomBuffer;
			bufferSize = geomBuffersize;
			colBuffer = rgb;
			intenBuffer = inten;
			classBuffer = classification;
			density = queryDensity;
			densityCoeff = denCoeff;
			rgbMode = rgbType;
			cs = pt::ProjectSpace;
			isWriter = writeMode;
			pcloudOnly = 0;
			sceneOnly = 0;
			ugrid = grid;
			pointLimit = 0;
			spatialGridFailure = false;
			layerMask = layer_mask;
			pntsFailed = 0;
			pntsTotal = 0;
			layerMask = layer_mask;

			if (g_currentRenderContext)
			{
				selcol[0] = g_currentRenderContext->settings()->selectionColour()[0];
				selcol[1] = g_currentRenderContext->settings()->selectionColour()[1];
				selcol[2] = g_currentRenderContext->settings()->selectionColour()[2];
			}
			else
			{
				selcol[0] = 255;
				selcol[1] = 0;
				selcol[2] = 0;
			}

			world2prj.set(static_cast<float>(-pt::Project3D::project().registration().matrix()(3,0)),
                          static_cast<float>(-pt::Project3D::project().registration().matrix()(3,1)),
                          static_cast<float>(-pt::Project3D::project().registration().matrix()(3,2)));
                                                                                                 
			/* detailed */
			if (_numPointChannels > 32) _numPointChannels = 32;

			filterBuffer = _filterBuffer;
			numPointChannels = _numPointChannels;

			memset( uchannels, 0, sizeof(void*)*32);

			for (uint c=0;c<numPointChannels; c++)
			{
				pointChannelsReq[c] = _pointChannelsReq[c];
				pointChannels[c] = (PTubyte*)_pointChannels[c];
				uchannels[c] = _ptGetUserChannel(pointChannelsReq[c]);
			}
		}
		//---------------------------------------------------------------------
		bool scene(pcloud::Scene *sc)
		{
			if ( sceneOnly && sc != sceneOnly ) return false;
			return C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool cloud(pcloud::PointCloud *cloud)
		{
			if ( pcloudOnly && cloud != pcloudOnly ) return false;
			return  C->boundsCheck(objCsBounds);
		}
		//---------------------------------------------------------------------
		bool visitNode(const pcloud::Node *n)
		{
			if (layerMask & (n->layers(0)|n->layers(1)) )
			{
				getNodeBounds(n, objCsBounds);

				bool res = C->boundsCheck(objCsBounds) && C->nodeCheck(n);
 
				if (res && n->isLeaf())
					return voxel(static_cast<Voxel*>(const_cast<pcloud::Node*>(n)));

				return res;
			}
			return false;
		}
		//---------------------------------------------------------------------
		bool getNodeBounds(const pcloud::Node *n, pt::BoundingBoxD &bounds)
		{
			if(n == NULL)
				return false;

			bounds = n->extents();
			bounds.translateBy(world2prj);

			return true;
		}

		//---------------------------------------------------------------------
		float voxelDensityValue( pcloud::Voxel *vox )
		{
			if (vox->densityValue() < 0.000001f)
			{
				pointsengine::VoxelLoader load(vox, 1.0f, false, true, true);
				//vox->computeDensityValue( 0.01f );//bb.diagonal().length() * 0.0025f );	
			}
			return vox->densityValue();
		}

		//---------------------------------------------------------------------

		float calculateVoxelLoad(pcloud::Voxel *vox, bool &doLoad)
		{
			doLoad = false;

			float amount = densityCoeff;

			if (vox->flag( pcloud::OutOfCore )) 
				doLoad = true;

			switch (density)
			{
			case PT_QUERY_DENSITY_SPATIAL:					// spatially uniform density
				amount = 1.0f;
				doLoad = true;
				break;

			case PT_QUERY_DENSITY_FULL:						// full density, but modulated by coeff
				doLoad = true;	//should be true
				break;

			case PT_QUERY_DENSITY_VIEW:						// current view based density without load
				amount = densityCoeff * std::min(vox->getCurrentLOD(), vox->getRequestLOD());
				break;

			case PT_QUERY_DENSITY_VIEW_COMPLETE:			// current view based density with load
				amount = densityCoeff * vox->getRequestLOD();
				doLoad = true;
				break;

			case PT_QUERY_DENSITY_LIMIT:					// density limited to a number of points
				amount = densityCoeff;						// problem is this is the density of selected points only;
				doLoad = true;
				break;
			}


			if ( C->processWhole(vox))
			{
				filter = 255;
			}
			else
			{
				filter = SELECTED_PNT_BIT;
			}

			currentVoxel = vox;

			Voxel::LODComparison lodComparison = vox->compareLOD(amount, vox->getCurrentLOD());
															// If LOD is less
			if(lodComparison == pcloud::Voxel::LODLess)
			{
															// Remember current LOD as previous LOD for unload because this won't be done when resizing channels due to not loading
				vox->setPreviousLOD(vox->getCurrentLOD());

				doLoad = false;
			}

			if(lodComparison == pcloud::Voxel::LODEqual)
			{
				doLoad = false;
			}


			return amount;
		}

		//---------------------------------------------------------------------

		bool addDeferredVoxel(pcloud::Voxel *voxel)
		{
			if(voxel)
			{
				deferredVoxels.push_back(voxel);

				return true;
			}

			return false;
		}

		void clearDeferredVoxels(void)
		{
			deferredVoxels.clear();
		}

		//---------------------------------------------------------------------

/*
	Query Algorithm Overview:

	- Query traversal may start from continueFrom voxel (with start point continuePnt) and will end at a voxel lastVoxel and point lastPoint when buffer is full or no voxels are left
	- Remote voxels on a server that require more data being fetched are deferred to deferredVoxels and batched into MultiReadSets 
	- Voxels requiring no fetch are iterated first to the buffer with a conservative guarantee that all deferred voxels (except last if last iterated) will fit on the end of the buffer
	- The last traversed voxel in the iteration may not fit into the buffer, whether non fetched or deferred.
	- Because deferred voxel iteration is conservative and may result in fewer points in the buffer, the buffer may not be full after processDeferredVoxels() is run
	- If voxel() completes without a full buffer, any next voxel traversed may then continue to fill the buffer, resulting in multiple processDeferredVoxel() executions per iteration
	- The start (voxel, point) to last (voxel, point) may contain any number of local, non fetched or deferred voxels
	- First or last voxels may be local, non fetched or deferred
	- The first voxel may be also the last voxel, e.g. when a voxel is several times larger than the buffer size and is either local, non fetched or deferred
	- If last voxel is not deferred, it is iterated after processDeferredVoxels() is executed. In this case, all deferred voxels will fit in the buffer.
	- If last voxel is deferred, it is processed in processDeferredVoxels() and may not be guaranteed to fit in the buffer.

	- If the query iteration traversal completes without filling the buffer, any remaining deferred voxels are processed by a final call to processDeferredVoxels() in query

*/

		bool voxel(pcloud::Voxel *vox)
		{
			float							amount;
			float							lod_amount;
			bool							doLoad;
			
			bool							voxelLoadDump = true;
			pcloud::Voxel				*	lastPartiallyIteratedVoxel = NULL;

			if(rwPos.counter >= bufferSize)
				return false;

			if(C->escapeWhole(vox))
				return true;

			if ( !((vox->layers(0)|vox->layers(1)) & layerMask))
				return true;

			setVoxel(vox);
	
			if(!begin)
				return true;

															
            std::unique_lock<std::mutex> lock(vox->mutex());

															// Calculate load details based on LODs and density settings
			amount = calculateVoxelLoad(vox, doLoad);

#ifdef NEEDS_WORK_VORTEX_DGNDB 
            DataSource					*	dataSource;
            StreamDataSource			*	streamDataSource;
            bool							streamDataSourceCreated;											// Get whether data source is local or remote
            DataSource::DataSourceForm dataSourceForm = vox->getDataSourceForm();
            
            // If DataSource is remote
			if(doLoad && dataSourceForm == ptds::DataSource::DataSourceFormRemote)
			{
															// Get existing open DataSource or create an open data source associated with this thread
				if((dataSource = getDataSourceManager().getOrCreateOpenVoxelDataSource(vox, 1)) == NULL)
				{
					Status::log(L"ReadPoints::voxel() ", L"data source failed");
					return false;
				}
															// Make sure all voxels are unlocked before locking StreamManager (otherwise hierarchical lock issue can occur)
				lock.unlock();
															// Make sure StreamManager is locked to this thread and cleared before use
															// Note: There can be multiple begin() calls but only one end() call.
				if(getStreamManager().begin() == false)
				{
					Status::log(L"ReadPoints::voxel() ", L"stream manager begin failed");
					return false;
				}

				lock.lock();
															// Calculate load details based on LODs and density settings
				amount = calculateVoxelLoad(vox, doLoad);

															// Create a StreamDataSource managed by StreamHost
															// This provides the DataSource with a ReadSet
				if((streamDataSource = getStreamManager().addReadVoxel(vox, dataSource, &streamDataSourceCreated)) == NULL)
				{
					Status::log(L"ReadPoints::voxel() ", L"add read voxel failed");
					return false;
				}
															// If active StreamDataSource has just been created
				if(streamDataSourceCreated)
				{
															// Make sure DataSource's ReadSet is enabled
					dataSource->setReadSetEnabled(true);
															// Set up DataSource ReadSet
					streamDataSource->beginReadSet();
				}
															// Associate following Reads with the Voxel
				dataSource->beginReadSetClientID(vox);

															// Reads are deferred, so do not dump voxel data at the end of this scope
				voxelLoadDump = false;
			}

#endif 
															// Carry out load with instruction to dump on scope destruction if memory is out of core
			pointsengine::VoxelLoader load(doLoad ? vox : 0, amount, false, false, voxelLoadDump);
															// Force loader not to lock
			load.setLock(false);


			bool finalVoxel;
															// Calculate total of currently iterated points in buffer and conservative potential points from deferred voxels
			unsigned int totalCurrentAndPotential	= rwPos.counter + rwPos.counterPotential;
															// Calculate the additional affect of this voxel (deferred or not deferred)
			unsigned int newPotential				= vox->getNumPointsAtLOD(amount);
															// If this voxel is the start voxel for this iteration, decrement number of points already iterated
			if(vox == rwPos.continueFrom)
			{
				newPotential -= rwPos.continuePnt;
			}
															// Calculate whether this is the final voxel that some or all of which will fit in the buffer
			finalVoxel = (totalCurrentAndPotential + newPotential) >= bufferSize;


			bool voxelDeferred = false;
#ifdef NEEDS_WORK_VORTEX_DGNDB 
															// If DataSource is remote
			if(doLoad && dataSourceForm == ptds::DataSource::DataSourceFormRemote)
			{
															// Disassociate voxel with DataSource
				dataSource->endReadSetClientID();
															// Add this voxel as deferred
				addDeferredVoxel(vox);

				voxelDeferred = true;
															// Update number of potential points available
				rwPos.counterPotential += newPotential;
			}
#endif
															// If real and potential number of points to be returned exceeds buffer size
			if(finalVoxel)
			{
				lock.unlock();
															// Read all deferred DataSources and iterate transformed points
				bool r = processDeferredVoxels();
															// If last voxel was deferred, it was iterated in processDeferredVoxel() so return
				if(voxelDeferred)
				{
					return r;
				}
															// Last voxel is local, so lock to iterate voxel
				lock.lock();
			}

															// If last voxel is remote, no need to iterate this voxel so return
			if(voxelDeferred)
			{
				return true;
			}
															// Make sure this voxel is specified as the last processed (processDeferredVoxels() can over write this)
//			rwPos.lastVoxel = vox;

			if(vox != rwPos.continueFrom)
			{
				rwPos.lastPoint = 0;
			}

															// Execute query over points and return whether to continue traversal
			bool result = iterateTransformedPoints(vox, amount, lod_amount);

															// If not all of the requested range of the voxel was iterated
			if(rwPos.lastPoint < vox->getNumPointsAtLOD(amount))
			{
			//	assert(rwPos.counter == bufferSize);
															// Remember that this voxel is not unloaded so it can be unloaded if query is reset or destroyed
				rwPos.setLastPartiallyIteratedVoxel(vox);
				rwPos.setLastPartiallyIteratedVoxelUnloadLOD(vox->getPreviousLOD());
															// Cancel dump of this voxel as it may be needed again
				load.setVoxel(NULL);
				load.setDump(false);
			}
			else
			{
															// Only dump if query is not View based (no loading). NOTE: Crashes if not present but shouldn't !
				if(density != PT_QUERY_DENSITY_VIEW)
				{
					load.setVoxel(vox);
					load.setDump(true);

					load.setPreviousLOD(vox->getPreviousLOD());
				}
															// If last partially iterated voxel is now fully iterated
				if(vox == rwPos.getLastPartiallyIteratedVoxel())
				{
															// Set last partially iterated voxel to NULL
					rwPos.setLastPartiallyIteratedVoxel(NULL);
					rwPos.setLastPartiallyIteratedVoxelUnloadLOD(0);
				}
			}

			return result;
		}

		unsigned int getNumDeferredVoxels(void)
		{
			return static_cast<uint>(deferredVoxels.size());
		}

		bool processDeferredVoxels(void)
		{
															// If no deferred voxels exist, nothing to do so exit OK
			if(getNumDeferredVoxels() == 0)
			{
				return true;
			}

			float						amount;
			float						lod_amount;
			bool						doLoad;
			VoxelPtrSet::iterator		it;
			Voxel					*	voxel;
			pcloud::Voxel			*	previousLastVoxel = rwPos.lastVoxel;
			PTint						previousLastPoint = rwPos.lastPoint;
															// End ReadSet generation
			getStreamManager().endReadSets();
															// Execute MultiReadSets/ReadSets
			if(getStreamManager().processStreamHostsRead() == false)
			{
				Status::log(L"ReadPoints::processDeferredVoxels() process stream hosts failed", L"");
				return true;
			}

//			rwPos.continueFrom = *(deferredVoxels.begin());
															// For each StreamDataSource voxel
			for(it = deferredVoxels.begin(); it != deferredVoxels.end(); it++)
			{
				if((voxel = *it) == NULL)
				{
					Status::log(L"ReadPoints::processDeferredVoxels() iterator failed failed", L"");
					return false;
				}

                std::lock_guard<std::mutex> lock(voxel->mutex());
                
															// Set the bounds
				getNodeBounds(voxel, objCsBounds);
				bool res = C->boundsCheck(objCsBounds);

															// Calculate load details based on LODs and density settings
				amount = calculateVoxelLoad(voxel, doLoad);

				{
															// Create Loader to carry out dump (but skip load)
					pointsengine::VoxelLoader load(doLoad ? voxel : NULL, amount, false, false, true, 1, true);
															// Set unload LOD
					load.setPreviousLOD(voxel->getPreviousLOD());
															// Reset last point processed so that point() starts from the beginning in iterateTransformedPoints
					rwPos.lastPoint = 0;
					rwPos.lastVoxel = voxel;

					rwPos.continuePnt = 0;
															// If the continueFrom was from this voxel, set up continue point for iterateTransformedPoints
					if(voxel == rwPos.continueFrom)
					{
						rwPos.continuePnt = rwPos.initialContinuePnt;
					}

					iterateTransformedPoints(voxel, amount, lod_amount);
															// If not all of the requested range of the voxel was iterated
					if(rwPos.lastPoint < voxel->getNumPointsAtLOD(amount))
					{
						assert(rwPos.counter == bufferSize);
															// Remember that this voxel is not unloaded so it can be unloaded if query is reset or destroyed
						rwPos.setLastPartiallyIteratedVoxel(voxel);
						rwPos.setLastPartiallyIteratedVoxelUnloadLOD(voxel->getPreviousLOD());
															// Cancel dump of this voxel as it may be needed again
						load.setVoxel(NULL);
					}
					else
					{
															// If last partially iterated voxel is now fully iterated
						if(voxel == rwPos.getLastPartiallyIteratedVoxel())
						{
															// Set last partially iterated voxel to NULL
							rwPos.setLastPartiallyIteratedVoxel(NULL);
							rwPos.setLastPartiallyIteratedVoxelUnloadLOD(0);
						}
					}
															// Not continuing after first voxel
					rwPos.continuePnt			= 0;
					rwPos.initialContinuePnt	= 0;
					rwPos.continueFrom			= NULL;
				}
			}
 
															// Deferred voxels processed, so clear
			clearDeferredVoxels();
															// Clear active data in StreamManager
			getStreamManager().clearActive();
															// Potential points now catered for so zero
			rwPos.counterPotential = 0;
															// If last deferred voxel is not last traversed
			if(voxel != previousLastVoxel)
			{
															// Reinstate the last point iterated
															// Else the last point from the deferred voxels will be preserved
				rwPos.lastPoint = previousLastPoint;
			}
															// Reinstate the last voxel iterated (deferred or non deferred)
			rwPos.lastVoxel = previousLastVoxel;
															// Return OK
			return true;
		}


		bool iterateTransformedPoints(pcloud::Voxel * vox, float amount, float &lod_amount)
		{
															// Set up channels
			beginChannels(vox, amount);

			spatialGridFailure = false;

			voxelSizing(vox);

															// Iterate over transformed points applying points call
			lod_amount = iterateTransformedPoints(vox, amount);

															// Update stats
			pntsFailed += vox->getNumPointsAtLOD(vox->getRequestLOD()) - rgbSize;
			pntsTotal  += vox->getNumPointsAtLOD(amount);

			if(spatialGridFailure)
				return false;

			endChannels(vox, amount);

			return true;									// i.e. continue
		}

/*
		float iterateTransformedPoints(pcloud::Voxel * vox, float amount) 
		{

			float lod_amount = 0;

			if (vox->lodPointCount() > 0)
			{
				lod_amount = amount * (float)vox->fullPointCount() / vox->lodPointCount();	// amount is % of the lod not % of full, so we need to compensate
			}
			if (lod_amount > 1.0f) 
				lod_amount = 1.0f;

			vox->iterateTransformedPoints(*this, cs, true, lod_amount);

			return lod_amount;
		}
*/


		float iterateTransformedPoints(pcloud::Voxel * vox, float amount) 
		{
			if (amount > 1.0f) 
			{
				amount = 1.0f;
			}

			if ( layerMask == 0 || ((vox->layers(0) | vox->layers(1)) & layerMask) )
			{
				vox->iterateTransformedPoints(*this, cs, layerMask, amount);
			}
			return amount;
		}



		void beginChannels(pcloud::Voxel *vox, float amount) 
		{
			// get channels
			DataChannel *crgb = vox->channel( pcloud::PCloud_RGB );
			DataChannel *cinten = vox->channel( pcloud::PCloud_Intensity );
			DataChannel *cfilter = vox->channel( pcloud::PCloud_Filter );
			DataChannel *cclass = vox->channel( pcloud::PCloud_Classification );


			if(cinten)
				ichannel = (PTshort*)cinten->data();
			else
				ichannel = 0;

			if (crgb) 
				rgbchannel = crgb->data();
			else 
				rgbchannel = 0;

			if (cclass) 
				clschannel = cclass->data();
			else 
				clschannel = 0;

			fchannel = 0;

			/* detail */
			if (filterBuffer && !cfilter)
			{
				pntFilterVal = vox->layers(0);
			}
			else if (cfilter) 
				fchannel = cfilter->data();

			uint c = 0;
			/* setup user channels for voxel */
			for (c = 0; c<numPointChannels; c++)
			{
				if (uchannels[c])
				{
					uvchannel[c] = uchannels[c]->voxelChannel( vox );

					if (uvchannel[c])
					{
						uchannels[c]->unlock( uvchannel[c], (int)(vox->getNumPointsAtLOD(amount)) );
						channelBPP[c] = uvchannel[c]->getBytesPerPoint();
					}
				}
				else
				{
					uvchannel[c] = 0;
					channelBPP[c] = 0;
				}
			}
		}


		void endChannels(pcloud::Voxel *vox, float amount) 
		{
			uint c = 0;
															// lock channels for OOC use
			for (c = 0; c<numPointChannels; c++)
			{
				if (uchannels[c])
				{
					uvchannel[c] = uchannels[c]->voxelChannel( vox );

					if(isWriter)
						uchannels[c]->update( uvchannel[c], vox->getNumPointsAtLOD(amount) );

					uchannels[c]->lock( uvchannel[c] );
				}
			}
		}


		//
		bool updatePointChannel(int index)
		{
			if (filterBuffer)
			{
				if (fchannel)
					fchannel[rwPos.lastPoint] = filterBuffer[rwPos.counter];
				else if (filterBuffer[rwPos.counter] != pntFilterVal)
				{
					/* TODO: create a filter channel */
				}
			}
			if (numPointChannels)
			{
				int ch = 0;
				for (uint c=0; c<numPointChannels; c++)
				{
					if (uvchannel[c])
					{
						// is this the one we want to update?
						if (pointChannelsReq[c]==writeChannel)
						{						
							PTubyte *ptr = &(pointChannels[c][channelBPP[c]*rwPos.counter]);							
#ifdef DEBUG		
							assert(uvchannel[c]->setVal(index, ptr));	// check for value not saved
#else
							uvchannel[c]->setVal(index, ptr);
#endif
							return true;
						}
					}
				}				
			}
			return false;
		}
		//---------------------------------------------------------------------
		bool point(const vector3d &pnt, uint index, ubyte &f)
		{
			if (rwPos.counter >= bufferSize) return false;

			if (rwPos.counter < bufferSize					&&
				(filter == 255 || C->validPoint(pnt, f))	&&
				rwPos.lastPoint >= rwPos.continuePnt )
			{
				bool b;
				
				if (density == PT_QUERY_DENSITY_SPATIAL && ugrid)
				{
					if (ugrid->get(pnt, b) && !b)
					{
						if (isWriter) return true;
						else  if (!ugrid->set(pnt, true))
						{
							spatialGridFailure = true;
							return true;
						}
					}
					else if (!isWriter) return true;
				}
				// WRITER
				if (isWriter)
				{
					updatePointChannel(index);
				}
				else
				{
					if (buffer)
					{
						buffer[rwPos.counter*3] =   static_cast<T>(pnt.x);
						buffer[rwPos.counter*3+1] = static_cast<T>(pnt.y);
						buffer[rwPos.counter*3+2] = static_cast<T>(pnt.z);
					}

					if (colBuffer)
					{
						colBuffer[rwPos.counter*3] = 255;
						colBuffer[rwPos.counter*3+1] = 255;
						colBuffer[rwPos.counter*3+2] = 255;

						if (rgbMode == PT_QUERY_RGB_MODE_ACTUAL && rgbchannel)
						{
							colBuffer[rwPos.counter*3] = rgbchannel[index*3];
							colBuffer[rwPos.counter*3+1] = rgbchannel[index*3+1];
							colBuffer[rwPos.counter*3+2] = rgbchannel[index*3+2];
						}
						else /* get a computed colour */
						{
							if ( (voxelSelected || f & SELECTED_PNT_BIT)
								&& rgbMode != PT_QUERY_RGB_MODE_SHADER_NO_SELECT)
							{
								colBuffer[rwPos.counter*3] = selcol[0];
								colBuffer[rwPos.counter*3+1] = selcol[1];
								colBuffer[rwPos.counter*3+2] = selcol[2];
							}
							else
							{
								if (rgbSize > index)
								{

									if (g_currentRenderContext)
									{
										g_currentRenderContext->renderer()->computeActualColour(
											&colBuffer[rwPos.counter*3], pnt,
											ichannel ? &ichannel[index] : 0,
											rgbchannel ? &rgbchannel[index*3] : 0,
											&pntFilterVal);	
									}			
									else if (rgbchannel)
									{
										colBuffer[rwPos.counter*3] = rgbchannel[index*3];
										colBuffer[rwPos.counter*3+1] = rgbchannel[index*3+1];
										colBuffer[rwPos.counter*3+2] = rgbchannel[index*3+2];
									}
								}
							}
						}
					}
					if (intenBuffer)
					{
						if (ichannel)
							intenBuffer[rwPos.counter] = ichannel[index];
						else intenBuffer[rwPos.counter] = 0;
					}
					if (filterBuffer)
					{
						if (fchannel)
							filterBuffer[rwPos.counter] = fchannel[index];
						else filterBuffer[rwPos.counter]  = pntFilterVal;
					}
					if (classBuffer)
					{
						if (clschannel)
							classBuffer[rwPos.counter] = clschannel[index];
						else classBuffer[rwPos.counter] = 0;
					}		
					if (numPointChannels)
					{
						int ch = 0;
						for (uint c=0; c<numPointChannels; c++)
						{
							if (uvchannel[c])
								uvchannel[c]->getVal(index, &(pointChannels[ch++][channelBPP[c]*rwPos.counter]));
						}
					}
				} /* reader / writer */
				++rwPos.counter;
			}	/* inclusion */
			++rwPos.lastPoint;
			return true;
		}
		//---------------------------------------------------------------------
		bool point(const vector3 &pnt, int index, ubyte &f)
		{
			vector3d pntd(pnt);
			return point(pntd,index,f);
		}
		//---------------------------------------------------------------------
		inline bool setVoxel(Voxel *vox)
		{
			if (!begin)
			{
				if (!rwPos.continueFrom)
				{
					begin = true;
				}
				else
					if(vox == rwPos.continueFrom)
					{
						begin = true;

						rwPos.initialContinuePnt = rwPos.continuePnt;
					}
			}
			else
				if(vox != rwPos.continueFrom)
				{
					rwPos.continuePnt = 0;
				}

				rwPos.lastPoint = 0;
				rwPos.lastVoxel = vox;

				return true;
		}
		//---------------------------------------------------------------------
		void voxelSizing(Voxel *vox)
		{
			/* more logic needed here to check RGB mode and so on */
			rgbSize = vox->channel(PCloud_Geometry)->size();

			PTuint csize = vox->channel(PCloud_RGB) ? vox->channel(PCloud_RGB)->size() : 0;
			
			if (csize && csize < rgbSize) rgbSize = csize;
			
			csize = vox->channel(PCloud_Intensity) ? vox->channel(PCloud_Intensity)->size() : 0;
			if (csize && csize < rgbSize) rgbSize = csize;

			voxelSelected = vox->flag(pcloud::WholeSelected);
		};
		//---------------------------------------------------------------------
		NodeCondition		*C;					// condition object

		PTbool				begin;
		
		PTbool				spatialGridFailure;
		PTubyte				filter;
		PTubyte				pntFilterVal;

		// selection state / colour
		PTbool				voxelSelected;
		PTubyte				selcol[3];

		VoxelPtrSet			deferredVoxels;

		// Buffers
		T					*buffer;		// geom buffer, can be float or double
		PTubyte				*colBuffer;		// rgb buffer
		PTshort				*intenBuffer;	// intensity buffer
		PTubyte				*classBuffer;	// classification buffer
		PTuint				bufferSize;		// buffer sizes as num points
		PTuint				rgbSize;		// not sure

		PTubyte				*rgbchannel;
		PTshort				*ichannel;
		PTubyte				*fchannel;
		PTubyte				*clschannel;
		PTubyte				*filterBuffer;		// the layer channel		
		
		PTubyte				layerMask;

		PTenum				rgbMode;
		
		// coordinate space transforms
		pt::CoordinateSpace	cs;
		pt::vector3d		world2prj;

		// query density options
		PTenum				density;
		PTfloat				densityCoeff;
		PTuint				pointLimit;			// max number of points to return - used by QUERY_DENSITY_LIMIT

		// to track state of query
		struct	ReadWritePos
		{
			PTuint				counter;
			PTuint				counterPotential;
			Voxel*				continueFrom;				
			PTuint				continuePnt;				
			PTint				initialContinuePnt;
			Voxel*				lastVoxel;					
			PTuint				lastPoint;

			Voxel *				lastPartiallyIteratedVoxel;
			Voxel::LOD			lastPartiallyIteratedVoxelUnloadLOD;
		
			ReadWritePos()
			{
				reset();
			}

			void operator = ( const ReadWritePos &rw )	
			{
				lastPoint = rw.lastPoint;
				lastVoxel = rw.lastVoxel;
				counter = rw.counter;
				counterPotential = rw.counterPotential;
				continueFrom=rw.continueFrom;				
				continuePnt=rw.continuePnt;			
				initialContinuePnt=rw.initialContinuePnt;
			}

			void reset()
			{
				lastPoint = 0;
				lastVoxel = 0;
				counter = 0;
				counterPotential = 0;
				continueFrom=0;				
				continuePnt=0;
				initialContinuePnt=0;

				setLastPartiallyIteratedVoxel(NULL);
				setLastPartiallyIteratedVoxelUnloadLOD(0);
			}

			void setLastPartiallyIteratedVoxel(Voxel *voxel)
			{
				lastPartiallyIteratedVoxel = voxel;
			}

			Voxel *getLastPartiallyIteratedVoxel(void)
			{
				return lastPartiallyIteratedVoxel;
			}

			void setLastPartiallyIteratedVoxelUnloadLOD(Voxel::LOD lod)
			{
				lastPartiallyIteratedVoxelUnloadLOD = lod;
			}

			Voxel::LOD getLastPartiallyIteratedVoxelUnloadLOD(void)
			{
				return lastPartiallyIteratedVoxelUnloadLOD;
			}

		};

		ReadWritePos				rwPos;

		pt::BitVoxelGrid			*ugrid;				// for spatial density option only

		const pcloud::PointCloud	*pcloudOnly;		// limit scope to a pcloud. Maybe null
		const pcloud::Scene			*sceneOnly;			// limit scope to a pod, maybe null
		
		// USER CHANNELS
		UserChannel					*uchannels[MAX_NUM_USER_CHANNELS];
		VoxelChannelData			*uvchannel[MAX_NUM_USER_CHANNELS];
		PTubyte						channelBPP[MAX_NUM_USER_CHANNELS];			// user channel bytes per point
		PTuint						numPointChannels;							// number of user channels
		PThandle					pointChannelsReq[MAX_NUM_USER_CHANNELS];	// an array of user channel handles to read
		PTubyte						*pointChannels[MAX_NUM_USER_CHANNELS];		// an array of buffers to read user channels data. 
																				// This array is updated by client and Submitted to update actual channel values
		PThandle					writeChannel;								// the channel to write update

		bool						isWriter;			// is intended to write back channel updates

		/* stats */ 
		int64_t						pntsFailed;			// point unavailable that are out of core
		int64_t						pntsTotal;			// total pnts read
	};


	//---------------------------------------------------------------------
	/* conditional query */
	//---------------------------------------------------------------------
	template <class Condition>
	struct CountPoints 
	{
		CountPoints( Condition *condition ) : C(condition), count(0) {} 
		
		template <class Real>
		void point(const pt::vec3<Real> &pnt, int index, ubyte &f) 
		{
			count += C->validPoint( vector3d(pnt), f ) ? 1 : 0;
		}
		int64_t count;
		Condition *C;	
	};

	template <class Condition>
	class ConditionQuery : public Query
	{
	public:
		/* condition must have copy constructor */
		ConditionQuery<Condition>(const Condition &condition, PThandle h) : C(condition)
		{
			cs = pt::ProjectSpace;
			_writer = 0;
			ugrid = 0;
			stats.m_numPoints = 0;
			stats.m_queryID = h;
			strcpy_s(stats.m_queryType, sizeof(stats.m_queryType), Condition::name());
		}
		//---------------------------------------------------------------------
		ConditionQuery<Condition>()
		{
			cs = pt::ProjectSpace; 
			_writer = 0;
			ugrid = 0; 
			stats.m_numPoints = 0;
			stats.m_queryID = 0;
			strcpy_s(stats.m_queryType, sizeof(stats.m_queryType), Condition::name());
		}
		//---------------------------------------------------------------------
		void setUpStats()
		{
			stats.m_density = Query::getDensityType();
			stats.m_densityVal = Query::getDensityCoeff();
			stats.m_numPoints = 0;
			strcpy_s( stats.m_queryType, sizeof(stats.m_queryType), Condition::name() );
		}
		//---------------------------------------------------------------------
		void createGrid()
		{
			PTdouble lower[3], upper[3];
			ptGetLowerBound( lower );
			ptGetUpperBound( upper );

			/* this is used for uniform density and takes little mem before use*/
			ugrid = new pt::BitVoxelGrid( lower[0], lower[1], lower[2],
				upper[0]-lower[0], upper[1]-lower[1], upper[2]-lower[2], densityCoeff);

		}
		//---------------------------------------------------------------------
		virtual ~ConditionQuery()
		{
			if (ugrid) delete ugrid;
			if (_writer) delete _writer;
		}
		//---------------------------------------------------------------------
		virtual int64_t computeNumPointsInQuery() 
		{
			std::vector <pcloud::Voxel*> voxels;
			GatherVoxelsVisitor<Condition> gather(&C, voxels, scope);
			thePointsScene().visitNodes(&gather, false);			
			
			int64_t count = 0;

			for (int i=0; i<gather.voxels.size(); i++)
			{
				pcloud::Voxel *v = gather.voxels[i];

				// need to handle layer masks here and condition dismissal
				if ( C.processWhole( v ) && v->layers(0) & layerMask )
				{
					switch (density)
					{
					case PT_QUERY_DENSITY_SPATIAL:					// spatially uniform density, can't evaluate
						count += v->fullPointCount();
						break;

					case PT_QUERY_DENSITY_FULL:						// full density, but modulated by coeff
                        count = static_cast<int64_t>(count + (v->fullPointCount() * densityCoeff));
						break;

					case PT_QUERY_DENSITY_VIEW_COMPLETE:
					case PT_QUERY_DENSITY_VIEW:						// current view based density
						count = static_cast<int64_t>(count + (densityCoeff * v->getRequestLOD() * v->fullPointCount()));
						break;
					}
				}
				else 	// per point, expensive
				{
					if ( (v->layers(1) | v->layers(0)) & layerMask)
					{
						CountPoints<Condition> counter( &C );
					
                        std::lock_guard<std::mutex> lock(v->mutex());

						// need to load to make this evaluation
						pointsengine::VoxelLoader load( v, densityCoeff, false, false);
						v->iterateTransformedPoints( counter, pt::ProjectSpace, layerMask );
						count += counter.count;
					}
				}
			}
			return count;
		}
		//---------------------------------------------------------------------
		template <class T>
		int runQueryT( int buffersize, T *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )
		{
			if (!lastVoxel)					//first iteration
			{
				ptdg::Time::stamp( stats );
				stats.m_bufferSize = buffersize;
				setUpStats();
			}

			if (!ugrid && density == PT_QUERY_DENSITY_SPATIAL)
				createGrid();

			if(density != PT_QUERY_DENSITY_VIEW)
			{
				thePointsPager().pause();
			}

			ReadPoints<Condition, T>
					reader( &C, buffersize, geomBuffer, rgbBuffer, inten, false, lastVoxel, lastPnt,
					density, densityCoeff, ugrid, rgbMode, layers, classification, 0, 0, 0, 
					getLastPartiallyIteratedVoxel(), getLastPartiallyIteratedVoxelUnloadLOD(),
					layerMask );

			reader.cs = cs;
			reader.pcloudOnly = pcloudOnly;
			reader.sceneOnly = sceneOnly;

			thePointsScene().visitNodes(&reader, false);
															// Process any remaining voxels that did not get processed during visitNodes()
			reader.processDeferredVoxels();
															// Release the stream manager in case any remote access was carried out
															// Note: This end() does not need to have a matching begin()
			getStreamManager().end();

			if(density != PT_QUERY_DENSITY_VIEW)
			{
				thePointsPager().unpause();
			}


			lastPnt = reader.rwPos.lastPoint;
			lastVoxel = reader.rwPos.lastVoxel;

			setLastPartiallyIteratedVoxel(reader.rwPos.getLastPartiallyIteratedVoxel());
			setLastPartiallyIteratedVoxelUnloadLOD(reader.rwPos.getLastPartiallyIteratedVoxelUnloadLOD());
			
			stats.m_numPoints += reader.rwPos.counter;
			
			if (buffersize != reader.rwPos.counter)	//completed query
			{
				ptdg::Time::endStamp( stats );
				ptdg::Diagnostics::instance()->addMetric( stats );
				stats.m_numPoints = 0;
			}

			return reader.spatialGridFailure ? -1 : reader.rwPos.counter;
		}
		//---------------------------------------------------------------------
		uint selectQueryPoints(bool select)
		{
			if (!ugrid && density == PT_QUERY_DENSITY_SPATIAL)
				createGrid();

			SelectPoints<Condition> selPoints( &C, select, density, densityCoeff, ugrid );

			selPoints.cs = cs;
			selPoints.pcloudOnly = pcloudOnly;
			selPoints.sceneOnly = sceneOnly;

			thePointsScene().visitNodes(&selPoints, false);

			return selPoints.spatialGridFailure ? -1 : selPoints.counter;
		}
		//---------------------------------------------------------------------
		int runQuery( int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers  )
		{
			return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, classification, layers );
		}
		//---------------------------------------------------------------------
		int runQuery( int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers   )
		{
			return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, classification, layers );
		}
		//---------------------------------------------------------------------
		virtual int runQueryStrided(const size_t buffersize, PTfloat * const geomBuffer, const size_t geomStride, ubyte *const rgbBuffer, const size_t rgbStride)
		{

			// Implement the strided query for revit's specific format:
			const size_t GEOM_STRIDE = 4;
			const size_t RGB_STRIDE = 16;
			assert(geomStride == GEOM_STRIDE);
			assert(rgbStride == RGB_STRIDE);
			if(geomStride != GEOM_STRIDE || rgbStride != RGB_STRIDE)
			{
				throw "Passed-in strides are not currrently specialised for.";
			}

			if (!lastVoxel)					//first iteration
			{
				ptdg::Time::stamp( stats );
				stats.m_bufferSize = static_cast<int>(buffersize);
				setUpStats();
			}

			if (!ugrid && density == PT_QUERY_DENSITY_SPATIAL)
				createGrid();

			
			ReadPoints<Condition, float>
					reader( &C, static_cast<int>(buffersize), geomBuffer, rgbBuffer, 0 /* intensity */, false, lastVoxel, lastPnt,
					density, densityCoeff, ugrid, rgbMode,0, 0 /*classification*/, 0, 0, 0, 0, 0, layerMask );

			reader.cs = cs;
			reader.pcloudOnly = pcloudOnly;
			reader.sceneOnly = sceneOnly;

			thePointsScene().visitNodes(&reader, false);

			lastPnt = reader.rwPos.lastPoint; ///!Trac ticket #312 We can reach here having read all voxels and no points at all (reader.lastPoint == 0).
			lastVoxel = reader.rwPos.lastVoxel;
			
			stats.m_numPoints += reader.rwPos.counter;
			
			if (buffersize != reader.rwPos.counter)	//completed query
			{
				ptdg::Time::endStamp( stats );
				ptdg::Diagnostics::instance()->addMetric( stats );
				stats.m_numPoints = 0;
			}

			return reader.spatialGridFailure ? -1 : reader.rwPos.counter;
		}

		//---------------------------------------------------------------------
		template <class T>
		int runDetailedQueryT( int buffersize, T *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
			PTubyte *filter, PTubyte *cf/*classification*/, PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
		{
			if (!lastVoxel)					// new run of query
			{
				ptdg::Time::stamp( stats );
				stats.m_bufferSize = buffersize;
				setUpStats();
			}

			if (!ugrid && density == PT_QUERY_DENSITY_SPATIAL)
				createGrid();

			if(density != PT_QUERY_DENSITY_VIEW)
			{
				thePointsPager().pause();
			}

			ReadPoints<Condition, T>
					reader( &C, buffersize, geomBuffer, rgbBuffer, inten, false, lastVoxel, lastPnt, density, densityCoeff, ugrid,
					rgbMode, filter, cf, numPointChannels, pointChannelsReq, pointChannels, 
					getLastPartiallyIteratedVoxel(), getLastPartiallyIteratedVoxelUnloadLOD(), layerMask);

			reader.pcloudOnly = pcloudOnly;
			reader.sceneOnly = sceneOnly;
			reader.cs = cs;

			/* create the write object that is used to write channel data back */
			/* write object stores query state to replicate exact point order for write back*/
			if (_writer) delete _writer;

			ReadPoints<Condition, T> *writer = new ReadPoints<Condition, T>( &C, buffersize, geomBuffer, rgbBuffer, inten, true, lastVoxel, lastPnt,
				density, densityCoeff, ugrid, rgbMode, filter, cf, numPointChannels, pointChannelsReq, pointChannels, 0, 0, layerMask);

			writer->pcloudOnly = pcloudOnly;
			writer->sceneOnly = sceneOnly;

			_writer = writer;

			thePointsScene().visitNodes(&reader, false);

															// Process any remaining voxels that did not get processed during visitNodes()
			reader.processDeferredVoxels();
															// Release the stream manager in case any remote access was carried out
															// Note: This end() does not need to have a matching begin()
			getStreamManager().end();

			if(density != PT_QUERY_DENSITY_VIEW)
			{
				thePointsPager().unpause();
			}

			lastPnt = reader.rwPos.lastPoint;
			lastVoxel = reader.rwPos.lastVoxel;

			setLastPartiallyIteratedVoxel(reader.rwPos.getLastPartiallyIteratedVoxel());
			setLastPartiallyIteratedVoxelUnloadLOD(reader.rwPos.getLastPartiallyIteratedVoxelUnloadLOD());

			stats.m_numPoints += reader.rwPos.counter;
			
			if (buffersize != reader.rwPos.counter)	//completed query
			{
				ptdg::Time::endStamp( stats );
				ptdg::Diagnostics::instance()->addMetric( stats );
				stats.m_numPoints = 0;
			}

			return reader.rwPos.counter;
		}
		//---------------------------------------------------------------------
		void submitChannelUpdate(PThandle channel)
		{
			if (_writer)
			{
				// we don't know what the parameters for ReadPoints are when _writer was creates
				// but it doesn't matter because the memory layout is the same
				// and the size of ReadPoints is the same
				// but just to check
				assert( sizeof(ReadPoints<NullCondition, float>) == sizeof(ReadPoints<FrustumCondition, double>));

				ReadPoints<NullCondition> *writer = static_cast<ReadPoints<NullCondition>* >(_writer);
				writer->writeChannel = channel;
				ReadPoints<NullCondition>::ReadWritePos pos( writer->rwPos );

				thePointsScene().visitNodes( _writer, false );
				writer->writeChannel = 0;
				writer->rwPos = pos;	// restore position for further channel updates
			}
		}
		//---------------------------------------------------------------------
		int runDetailedQuery( int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
			PTubyte *layers, PTubyte *cf, PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
		{
			return runDetailedQueryT( buffersize, geomBuffer, rgbBuffer, inten, layers, cf, numPointChannels, pointChannelsReq, pointChannels );
		}
		//---------------------------------------------------------------------
		int runDetailedQuery( int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
			PTubyte *layers, PTubyte *cf, PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
		{
			return runDetailedQueryT( buffersize, geomBuffer, rgbBuffer, inten, layers, cf, numPointChannels, pointChannelsReq, pointChannels );
		}
		//---------------------------------------------------------------------
		void resetQuery()
		{
			Query::resetQuery();
			if (ugrid) ugrid->clear();
			
			ptdg::Diagnostics::instance()->addMetric( stats );
			stats.m_numPoints = 0;
		}

		//---------------------------------------------------------------------
		int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTfloat **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
		{
			return 0;
		}

		//---------------------------------------------------------------------
		int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTdouble **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
		{
			return 0;
		}


		//---------------------------------------------------------------------
		pt::BitVoxelGrid	*ugrid;
		PointsVisitor		*_writer;
		pt::CoordinateSpace cs;
		Condition			C;
	};

//-----------------------------------------------------------------------------
// LoadedVoxel
//-----------------------------------------------------------------------------

	class LoadedVoxel
	{
	public:

		typedef float		LOD;

	protected:

		pcloud::Voxel *		voxel;
		LOD					previousLOD;

	public:

		LoadedVoxel(void)
		{
			setVoxel(NULL);
		}

		LoadedVoxel(pcloud::Voxel *v, LOD prevLOD)
		{
			setVoxel(v);
			setPreviousLOD(prevLOD);
		}

		void setVoxel(pcloud::Voxel *v)
		{
			voxel = v;
		}

		pcloud::Voxel *getVoxel(void)
		{
			return voxel;
		}

		void setPreviousLOD(LOD l)
		{
			previousLOD = l;
		}

		LOD getPreviousLOD(void)
		{
			return previousLOD;
		}

		void loadLOD(LOD l)
		{
																// Load full data into cache
			pointsengine::VoxelLoader	loader(voxel, 1, true, true, false, 1);
		}

		void requestLOD(LOD newLOD)
		{
			if(getVoxel())
			{
				getVoxel()->setRequestLOD(newLOD);
			}
		}

		void requestPreviousLOD(void)
		{
			requestLOD(getPreviousLOD());
		}
	};



//-----------------------------------------------------------------------------
// AnalyticalQuery
//-----------------------------------------------------------------------------

	class AnalyticalQuery : public Query
	{
	public:

		typedef float						LOD;
		typedef std::vector<LoadedVoxel>	LoadedVoxelSet;

	protected:

		LoadedVoxelSet	lvs;

	public:

		AnalyticalQuery(void)
		{

		}

		void loadVoxel(pcloud::Voxel *voxel, LOD lod)
		{
															// Create record of voxel and its previous LOD
			LoadedVoxel	loadedVoxel(voxel, voxel->getCurrentLOD());

			lvs.push_back(loadedVoxel);
															// Load to specified LOD
			loadedVoxel.loadLOD(lod);
		}

		void restorePreviousVoxelLODs(void)
		{
			LoadedVoxelSet::iterator	i;
															// For each loaded Voxel
			for(i = lvs.begin(); i != lvs.end(); i++)
			{
															// Request previous LOD
				i->requestLOD(i->getPreviousLOD());
			}
		}

	};

//-----------------------------------------------------------------------------
// KNearestNeighbourQuery
//-----------------------------------------------------------------------------

template<class Node> class PriorityQueue
{

protected:

	typedef std::priority_queue<Node, std::deque<Node> >	Queue;

public:

	typedef Node											QueueNode;

protected:

	Queue		priorityQueue;

public:


	inline void add(QueueNode &entry)
	{
		priorityQueue.push(entry);
	}

	inline QueueNode *getFirst(void)
	{
		return const_cast<QueueNode *>(&(priorityQueue.top()));
	}

	inline void pop(void)
	{
		return priorityQueue.pop();
	}

	inline unsigned long getSize(void)		
	{
		return static_cast<unsigned long>(priorityQueue.size());
	}

	inline void clear(void)
	{
		// Possibly assign an empty priority queue here

		while(priorityQueue.empty() == false)
		{
			priorityQueue.pop();
		}
	}

};


enum SortMode
{
	SortModeNULL,

	SortModeMiniMin,
	SortModeMiniMax,

	SortModeMin,
	SortModeMax,

	SortModeEnd
};


template<class Real, SortMode sortMode> class VoxelPointNode;

// Represents a node in a MIN priority queue
template<class Real, SortMode sortMode> class KNearestNeighbourNode
{

protected:

	typedef KNearestNeighbourNode<Real, sortMode>		this_type;
	typedef ReadPoints<NullCondition, float>			Visitor;

	typedef VoxelPointNode<Real, SortModeMax>			PointNode;
	typedef float										LOD;

protected:

	pcloud::Node	*	node;
	Real				distanceMin;		// Query AABB's nearest distance to this node's AABB
	Real				distanceMax;		// Query AABB's farthest distance to this node's AABB

	Visitor			*	visitor;

// Test
public:
pt::BoundingBox	extents;


public:

	KNearestNeighbourNode(void)
	{

	}

	KNearestNeighbourNode(pcloud::Node *n, Real distMin, Real distMax, Visitor *v)
	{
		setNode(n);
		setDistanceMin(distMin);
		setDistanceMax(distMax);
		setVisitor(v);
	}

	void setVisitor(Visitor *v)
	{
		visitor = v;
	}

	Visitor *getVisitor(void)
	{
		return visitor;
	}

	void setNode(pcloud::Node *n)
	{
		node = n;
	}

	pcloud::Node *getNode(void)
	{
		return node;
	}

	void setDistanceMin(Real dist)
	{
		distanceMin = dist;
	}

	Real getDistanceMin(void) const
	{
		return distanceMin;
	}

	void setDistanceMax(Real dist)
	{
		distanceMax = dist;
	}

	Real getDistanceMax(void) const
	{
		return distanceMax;
	}

	unsigned int getPointCount(void)
	{
		if(getNode()->isLeaf())
            return static_cast<uint>(reinterpret_cast<pcloud::Voxel *>(getNode())->fullPointCount());

		return static_cast<uint>(getNode()->lodPointCount());
	}

															// Implements Min Priority queue by inverting comparison
	bool operator<(const this_type &n) const
	{
															// If sorting by minimum distance value, compare
		if(sortMode == SortModeMiniMin)
			return getDistanceMin() > n.getDistanceMin();
															// Else sorting by Maximum distance value
		if(sortMode == SortModeMiniMax)
			return getDistanceMax() > n.getDistanceMax();
															// This shouldn't happen
		return false;
	}
															// Get the index'th point from the voxel
	void getPoint(unsigned int index, PointNode &result)
	{
		// assert(getNode() != NULL && getNode()->isLeaf());

		result.setVoxel(reinterpret_cast<pcloud::Voxel *>(getNode()));
		result.setIndex(index);
	}
};

class VoxelPoint
{

public:

	typedef unsigned long	PointIndex;

protected:


	pcloud::Voxel	*		voxel;
	PointIndex				index;

public:

	VoxelPoint(void)
	{
		setVoxel(NULL);
	}

	VoxelPoint(pcloud::Voxel *v, PointIndex i)
	{
		setVoxel(v);
		setIndex(index);
	}

	void setVoxel(pcloud::Voxel *v)
	{
		voxel = v;
	}

	pcloud::Voxel *getVoxel(void)
	{
		return voxel;
	}

	void setIndex(PointIndex i)
	{
		index = i;
	}

	PointIndex getIndex(void)
	{
		return index;
	}

	template<typename Real> void getVertex(matrix<3, 1, Real> &result)
	{
		getVertex(result.data());
	}


	template<typename Real> void getVertex(Real *result)
	{
		assert(getVoxel() != NULL);

pcloud::Voxel::CoordinateSpaceTransform	transform(const_cast<pcloud::PointCloud *>(getVoxel()->pointCloud()), pt::ProjectSpace);
transform.prepare();

		// NOTE: This needs to be optimized to copy directly to the result

		pt::vector3	vCloudSpace;
		pt::vector3 vProjectSpace;

		getVoxel()->pointPosition(getIndex(), vCloudSpace);

		transform.transform(vCloudSpace, vProjectSpace);

		// Note: Speed this up so copy not needed

		result[0] = vProjectSpace.x;
		result[1] = vProjectSpace.y;
		result[2] = vProjectSpace.z;

	}

	void getRGB(unsigned char *data)
	{
		assert(voxel);
		voxel->pointRGB(getIndex(), data);
	}

	void getIntensity(PTshort &data)
	{
		assert(voxel);
		voxel->pointIntensity(getIndex(), data);
	}

};


template<class Real, SortMode sortMode> class VoxelPointNode : public VoxelPoint
{

public:

	typedef VoxelPointNode<Real, sortMode>	this_type;

protected:

	Real	queryPointDistance;

public:

	VoxelPointNode(void)
	{

	}

	VoxelPointNode(pcloud::Voxel *v, PointIndex i, Real queryDistance = 0) : VoxelPoint(v, i)
	{
		setQueryPointDistance(queryDistance);
	}


	void setQueryPointDistance(Real dist)
	{
		queryPointDistance = dist;
	}

	Real getQueryPointDistance(void) const
	{
		return queryPointDistance;
	}

	void getVoxelPoint(VoxelPoint &result)
	{
		result = static_cast<VoxelPoint>(*this);
	}

	bool operator < (const VoxelPointNode &p) const
	{
		if(sortMode == SortModeMax)
			return getQueryPointDistance() < p.getQueryPointDistance();

		return getQueryPointDistance() > p.getQueryPointDistance();
	}
};


template<class Real> class KNNQueryPoint
{

public:

	typedef VoxelPointNode<Real, SortModeMax>		VoxelPointNodeMax;
	typedef PriorityQueue<VoxelPointNodeMax>		PriorityQueueMax;
	typedef KNNQueryPoint<Real>						this_type;

protected:

	matrix<3,1, Real>		vertex;
	Real					maxDistance;

	PriorityQueueMax		pointPriorityQueue;

	unsigned int			k;

	pcloud::Voxel		*	voxel;

public:

	KNNQueryPoint(Real *v, unsigned int initK)
	{
		if(v != NULL)
		{
			setVertex(v);
			setK(initK);
		}

	}

	inline void setVertex(Real *v)
	{
		vertex = v;
	}

	inline void getVertex(matrix<3, 1, Real> &result)
	{
		result = vertex;
	}

	void setVoxel(pcloud::Voxel *initVoxel)
	{
		voxel = initVoxel;
	}

	pcloud::Voxel *getVoxel(void)
	{
		return voxel;
	}

	//void getVertex(Real *dest)
	//{
	//	dest[0] = vertex(0, 0);
	//	dest[1] = vertex(1, 0);
	//	dest[2] = vertex(2, 0);
	//}

	inline void setMaxDistance(Real dist)
	{
		maxDistance = dist;
	}

	inline Real getMaxDistance(void)
	{
		return maxDistance;
	}

	inline void setK(unsigned int initK)
	{
		k = initK;
	}

	inline unsigned int getK(void)
	{
		return k;
	}

	inline bool isKFull(void)
	{
		return pointPriorityQueue.getSize() == getK();
	}

	inline unsigned long getNumResults(void)
	{
															// Return number of results. When processed, usually k
		return pointPriorityQueue.getSize();
	}

	inline void getResultPop(VoxelPoint &result)
	{
		pointPriorityQueue.getFirst()->getVoxelPoint(result);

		pointPriorityQueue.pop();
	}

	inline Real getDistanceSquared(const pt::vector3d &p)
	{
		Real deltaX = static_cast<Real>(p.x) - vertex(0, 0);
		Real deltaY = static_cast<Real>(p.y) - vertex(1, 0);
		Real deltaZ = static_cast<Real>(p.z) - vertex(2, 0);

		return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
	}

	inline Real getDistanceSquared(VoxelPointNodeMax &p)
	{
		matrix<3, 1, Real>	v;
		matrix<3, 1, Real>	delta;

		p.getVertex(v);

		delta = vertex - v;

		delta = delta * delta;

		return delta(0, 0) + delta(1, 0) + delta(2, 0);
	}

	inline Real getDistanceSquared(this_type &p)
	{
		matrix<3, 1, Real>	delta;
		
		delta = vertex - p.vertex;

		return delta(0, 0) * delta(0, 0) + delta(1, 0) * delta(1, 0) + delta(2, 0) * delta(2, 0);
	}

	inline Real getDistanceSquared(Real *data)
	{
        Real delta_x = vertex.x - data[0];
        Real delta_y = vertex.y - data[1];
        Real delta_z = vertex.z - data[2];

		return delta_x * delta_x + delta_y * delta_y * delta_z * delta_z;
	}


															// Attempt to add it to query point priority queue
	inline void point(const pt::vector3d &point, uint index, ubyte &f)
	{
															// Calculate squared distance from this query point to the given test point
		Real distanceSquared = getDistanceSquared(point);
															// If queue has k items
		if(pointPriorityQueue.getSize() == k)
		{
															// If distance is within current k range
			if(distanceSquared >= getMaxDistance())
			{
															// Return not in k distance range
				return;
			}
			else
			{
				VoxelPointNodeMax p;

				p.setVoxel(getVoxel());
				p.setIndex(index);
															// Add test point to qiery point priority queue
				p.setQueryPointDistance(distanceSquared);

				pointPriorityQueue.add(p);
															// Cull largest to reduce back to k items
				pointPriorityQueue.pop();

			}
		}
		else
		{
			VoxelPointNodeMax p;

			p.setVoxel(getVoxel());
			p.setIndex(index);
															// Query point priority queue not yet full
			p.setQueryPointDistance(distanceSquared);
															// Add test point to k
			pointPriorityQueue.add(p);
		}

															// Calculate current furthest point
		Real distanceToMax = getDistanceSquared(*pointPriorityQueue.getFirst());
															// Set distance to furthest point within current k set
		setMaxDistance(distanceToMax);

	}


	// Returns true if item added to k, false if point culled

	inline void testNearestInK(VoxelPointNodeMax &p)
	{
															// Calculate squared distance from this query point to the given test point
		Real distanceSquared = getDistanceSquared(p);
															// If queue has k items
		if(pointPriorityQueue.getSize() == k)
		{
															// If distance is within current k range
			if(distanceSquared >= getMaxDistance())
			{
															// Return not in k distance range
				return;
			}
			else
			{
															// Add test point to qiery point priority queue
				p.setQueryPointDistance(distanceSquared);
				pointPriorityQueue.add(p);
															// Cull largest to reduce back to k items
				pointPriorityQueue.pop();

			}
		}
		else
		{
															// Query point priority queue not yet full
			p.setQueryPointDistance(distanceSquared);
															// Add test point to k
			pointPriorityQueue.add(p);
		}

															// Calculate current furthest point
		Real distanceToMax = getDistanceSquared(*pointPriorityQueue.getFirst());
															// Set distance to furthest point within current k set
		setMaxDistance(distanceToMax);
															// Return point within current k distance range
	}
};


class KNearestNeighbourQuery : public AnalyticalQuery, public ReadPoints<NullCondition, PTfloat>
{

public:

	typedef float											Real;
	typedef std::deque<KNNQueryPoint<Real> >				QueryPointSet;

	typedef KNearestNeighbourNode<PTfloat, SortModeMiniMax>	PriorityQueueNodeMiniMax;
	typedef KNearestNeighbourNode<PTfloat, SortModeMiniMin>	PriorityQueueNodeMiniMin;

	typedef PriorityQueue<PriorityQueueNodeMiniMax>			PriorityQueueMiniMax;
	typedef PriorityQueue<PriorityQueueNodeMiniMin>			PriorityQueueMiniMin;

	typedef ReadPoints<NullCondition, float>				Visitor;
	typedef std::deque<Visitor>								VisitorSet;


protected:

	QueryPointSet				queryPoints;
	pt::BoundingBox				queryBox;
	Real						k;
	PTfloat						queryLOD;

	PriorityQueueMiniMax		e;

	VisitorSet					visitors;

public:

	KNearestNeighbourQuery(void)
	{

	}
	static const char* name()  { return "KNN"; }

	KNearestNeighbourQuery(PTfloat *vertices, PTint numQueryVertices, PTint k, float queryLOD) : AnalyticalQuery(), ReadPoints(NULL, 0, NULL)
	{
															// Validate parameters
		if(vertices == NULL || numQueryVertices == 0 || k == 0)
		{
															// Note: Flag error here or throw exception
			return;
		}
															// Remember K, the number of points to search for
		setK(k);

		setQueryLOD(queryLOD);

        PTint	t;
															// Clear box ready for new sizing
		queryBox.clear();

		matrix<3,1>	vertex;
															// Insert all query points into query bounding box														
		for(t = 0; t < numQueryVertices; t++)
		{
															// Get vertex			
			vertex = &(vertices[t * 3]);
															// Add point to bounding box (expands extents)
			queryBox.expand(vertex.data());

			KNNQueryPoint<Real>	p(vertex.data(), k);
															// Copy point to internal querySet
			queryPoints.push_back(p);
		}
	}

	void setK(PTint initK)
	{
		k = static_cast<Real>(initK);
	}

    PTuint getK(void)
	{
		return static_cast<PTuint>(k);
	}

	void setQueryLOD(PTfloat lod)
	{
		queryLOD = lod;
	}

	float getQueryLOD(void)
	{
		return queryLOD;
	}

	void resetQuery() 
	{
	}

	void setScope(PThandle cloudOrScene) 
	{
	}

	void setDensity(PTenum type, PTfloat coeff) 
	{
	}

	uint selectQueryPoints(bool select)
	{
		return 0;
	}

	void setRGBMode(PTenum mode) 
	{
	}

	template <class T> int runQueryT( int buffersize, T *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )
	{
		return 0;
	}

	template<class T> int runQueryMultiT(int numResultSets, int bufferSize, PTuint *resultSetSize, T **geomBufferArray, ubyte **rgbBufferArray, PTshort **intensityArray)
	{

		PriorityQueueMiniMax	f;
		PriorityQueueMiniMin	neighbourhood;
															// Validate parameters
		if(numResultSets <= 0 || bufferSize <= 0 || resultSetSize == NULL || geomBufferArray == NULL)
		{
			setLastErrorCode(PTV_INVALID_VALUE_FOR_PARAMETER);
			return 0;
		}
															// Make sure Query has been created properly
		if(queryBox.isEmpty() || queryPoints.size() == 0)
			return 0;

															// Create initial priority queue by traversing to all
															// clouds and adding a queue entry for each octree root
		NullCondition	nullCondition;
		C = &nullCondition;
		thePointsScene().visitPointClouds(this);


		thePointsPager().pause();
															// Process MiniMax priority queue to prune distance
															// and calculate MiniMax prune queue a
		float pruneDistance = calculateNeighbourhood(e, neighbourhood, getK());
															// Calculate K-NN for each point based on g
		//calculatePointKNN(g, numResults, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, intensityArray);
		calculatePointKNNCoherent(neighbourhood, numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, intensityArray);
															// Restore all voxel LODs to previous states (not immediate)
		restorePreviousVoxelLODs();

		thePointsPager().unpause();

#ifdef _DEBUG
		verifyKNNResults(numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, intensityArray);
#endif

		return numResultSets;

	}

	// QueryPointSet must already be set up

	float calculateNeighbourhood(PriorityQueueMiniMax &e, PriorityQueueMiniMin &result, PTuint k)
	{
		float							pruneDistance;
		PriorityQueueNodeMiniMax	*	h;
		unsigned long					s;

		unsigned int					totalPointCount;

		PriorityQueueMiniMax			f;
		PriorityQueueMiniMax		*	source	= &e;
		PriorityQueueMiniMax		*	dest	= &f;
		bool							refined = true;

															// Iterate until all nodes in destination are leaves
		while(source->getSize() > 0 && refined)
		{
			refined = false;

			totalPointCount = 0;
															// Include and refine until point count exceeds k
															// (Important that the last item is included)
			for(s = source->getSize(); s >= 0 && totalPointCount < k; s--)
			{
				h = source->getFirst();

				totalPointCount += h->getPointCount();

				refined |= refineToChildren(h, *dest);
															// Update prune distance
				pruneDistance = h->getDistanceMax();

				source->pop();
			}

															// Include and refine overlapping regions
															// (note: this is faster than sorting MiniMin)
			while(s > 0)
			{
				h = source->getFirst();

				if(h->getDistanceMin() < pruneDistance)
				{
					refined |= refineToChildren(h, *dest);
				}

				source->pop();
				s--;
			}

															// Destination becomes source
			std::swap(source, dest);
		}


															// Write result to MiniMin priority queue
		for(s = source->getSize(); s > 0; s--)
		{
															// Get volume
			h = source->getFirst();

			pcloud::Voxel *voxel = reinterpret_cast<pcloud::Voxel *>(h->getNode());

															// Load voxel and keep record of previous LOD
			loadVoxel(voxel, getQueryLOD());
															// Add volume to results in MiniMin order
			result.add(*reinterpret_cast<PriorityQueueNodeMiniMin *>(h));

pt::BoundingBox	ext;
getExtents(h->getNode(), *(h->getVisitor()), ext);
h->extents = ext;

			source->pop();
		}


															// Return the prune distance
		return pruneDistance;
	}


	template<class T> void calculatePointKNN(PriorityQueueMiniMin &g, int numResults, int bufferSize, T **geomBufferArray, ubyte **rgbBufferArray, PTshort **intensityArray)
	{
		std::list<KNNQueryPoint<PTfloat> *>					activeQueries;
		std::list<KNNQueryPoint<PTfloat> *>::iterator		i;
		unsigned int										s;
		unsigned int										j;
		PriorityQueueNodeMiniMin						*	b;
		VoxelPointNode<T, SortModeMax>						p;
		KNNQueryPoint<PTfloat>							*	queryPoint;


		thePointsPager().pause();

															// Add all queries to active query list
		for(s = queryPoints.size(); s > 0; s--)
		{
			activeQueries.push_front(&(queryPoints[s - 1]));
		}

															// For each volume in MiniMin queue
		for(s = g.getSize(); s > 0 && activeQueries.size() > 0; s--)
		{
															// Get next volume in MiniMin order
			b = g.getFirst();

			i = activeQueries.begin();

			while(i != activeQueries.end())
			{
				queryPoint = *i;
															// If queryPoint has k items and it's range is nearer than volume
				if(queryPoint->isKFull() && queryPoint->getMaxDistance() < b->getDistanceMin())
				{
															// Query point is solved, so remove from active list
					i = activeQueries.erase(i);
				}
				else
				{
					i++;
				}
			}
															// If there are query points active
			if(activeQueries.size() > 0)
			{
															// For each point in volume
				for(j = b->getPointCount(); j > 0; j--)
				{
															// Get volume point to test against active queries
					b->getPoint(j - 1, p);
															// For each active query
					for(i = activeQueries.begin(); i != activeQueries.end(); i++)
					{
															// Get query point
						queryPoint = *i;
															// Attempt to add it to query point priority queue
						queryPoint->testNearestInK(p);
					}
				}
			}
															// Pop current volume (and move to next)
			g.pop();
		}


		transferKNNResults(geomBufferArray, rgbBufferArray, intensityArray);
	}



	template<class T> inline void calculateSinglePointKNN(KNNQueryPoint<T> &queryPoint, std::vector<PriorityQueueNodeMiniMin> &neighbourhood)
	{
		unsigned int					s, j;
		PriorityQueueNodeMiniMin	*	b;
		VoxelPointNode<T, SortModeMax>	p;

															// While neighbourhood boxes are within query point max distance
		for(s = 0; s < neighbourhood.size() && (b = &(neighbourhood[s]))->getDistanceMin() < queryPoint.getMaxDistance(); s++)
		{
			for(j = b->getPointCount(); j > 0; j--)
			{
															// Get volume point to test against active queries
				b->getPoint(j - 1, p);
															// Attempt to add it to query point priority queue
				queryPoint.testNearestInK(p);
			}
		}
	}



	template<class T> inline void calculateSinglePointKNN2(KNNQueryPoint<T> &queryPoint, std::vector<PriorityQueueNodeMiniMin> &neighbourhood)
	{
		unsigned int					s;
		PriorityQueueNodeMiniMin	*	b;
		VoxelPointNode<T, SortModeMax>	p;

															// While neighbourhood boxes are within query point max distance
		for(s = 0; s < neighbourhood.size() && (b = &(neighbourhood[s]))->getDistanceMin() < queryPoint.getMaxDistance(); s++)
		{
			pcloud::Voxel *voxel = reinterpret_cast<pcloud::Voxel *>(b->getNode());

			queryPoint.setVoxel(voxel);

			voxel->iterateTransformedPoints(queryPoint);
		}
	}

	

	template<class T> void calculatePointKNNCoherent(PriorityQueueMiniMin &initNeighbourhood, int numResultSets, int bufferSize, PTuint *resultSetSize, T **geomBufferArray, ubyte **rgbBufferArray, PTshort **intensityArray)
	{
		unsigned int										s;
		unsigned int										m;
		KNNQueryPoint<PTfloat>							*	queryPoint;
		std::vector<PriorityQueueNodeMiniMin>				neighbourhood;
															// Copy neighbourhood to a vector
		for(s = initNeighbourhood.getSize(); s > 0; s--)
		{
			neighbourhood.push_back(*initNeighbourhood.getFirst());
			initNeighbourhood.pop();
		}
															// Calculate first query point KNN
		queryPoint = &(queryPoints[0]);
		queryPoint->setMaxDistance(1000000000000);
		calculateSinglePointKNN2(*queryPoint, neighbourhood);

		unsigned int	t;
		unsigned int	v;
		float			tMax;
		float			minimumMax;
															// For each remaining query point
		for(t = 1; t < queryPoints.size(); t++)
		{
			queryPoint = &(queryPoints[t]);

			minimumMax = sqrt(queryPoint->getDistanceSquared(queryPoints[0])) + sqrt(queryPoints[0].getMaxDistance());
			minimumMax *= minimumMax;
															// Limit solved search to 50 items (for large voxels)
			m = t;
			if(m > 50)
				m = 50;
															// For each previously solved query point
			for(v = 0; v < m; v++)
			{
				tMax = sqrt(queryPoint->getDistanceSquared(queryPoints[v])) + sqrt(queryPoints[v].getMaxDistance());
				tMax *= tMax;

				minimumMax = std::min(minimumMax, tMax);
			}
																		// Inherit max threshold
			queryPoint->setMaxDistance(tMax);
																		// Calculate query point KNN
			calculateSinglePointKNN2(*queryPoint, neighbourhood);

		}

		transferKNNResults(resultSetSize, geomBufferArray, rgbBufferArray, intensityArray);


	}


	template<class T> bool verifyKNNResults(int numResultSets, int bufferSize, PTuint *resultSetSize, T **geomBufferArray, ubyte **rgbBufferArray, PTshort **intensityArray)
	{
		if(resultSetSize == NULL || geomBufferArray == NULL)
			return false;

		if(numResultSets == 0)
			return false;

		unsigned int		t, p;
		T				*	base;
		pt::BoundingBox		queryBoxT;
															// Get 5% enlarged query box
		queryBoxT = queryBox;
		float offset = 0.01;
		queryBoxT.expandByOffset(offset, offset, offset);


		for(t = 0; t < numResultSets; t++)
		{
															// Make sure there are K results for each point
			if(resultSetSize[t] != getK())
				return false;
															// Make sure there's enough buffer
			if(resultSetSize[t] > bufferSize)
				return false;
															// Get base address of result set for geometry
			base = geomBufferArray[t];
															// Make sure all points are in original query box
			for(p = 0; p < resultSetSize[t]; p++)
			{
//				if(queryBoxT.inBounds(&(base[p*3])) == false)
					return false;
			}
		}
																			// Return OK
		return true;
	}


/*
		if (!ugrid) createGrid();

		ReadPoints<Condition, T>
				reader( C, buffersize, geomBuffer, rgbBuffer, inten, false, lastVoxel, lastPnt,
				density, densityCoeff, ugrid, rgbMode );

		reader.cs = cs;
		reader.pcloudOnly = pcloudOnly;
		reader.sceneOnly = sceneOnly;

		thePointsScene().visitNodes(&reader, false);

		lastPnt = reader.lastPoint;
		lastVoxel = reader.lastVoxel;

		return reader.counter;
*/


	template<class T> void transferKNNResults(PTuint *resultSetSize, T **geomBufferArray, ubyte **rgbBufferArray, PTshort **intensityBufferArray)
	{
		if(resultSetSize == NULL)
			return;

		T					*	baseGeom;
		ubyte				*	baseRGB;
		PTshort				*	baseIntensity;

		KNNQueryPoint<Real>	*	queryPoint;
		VoxelPoint				voxelPoint;

		if(geomBufferArray == NULL)
			return;

															// Faster geometric copy
		if(rgbBufferArray == NULL && intensityBufferArray == NULL)
		{
            size_t n = queryPoints.size();
															// For each query point
			for(size_t p = 0; p < n; p++)
			{
															// Get it's geom destination buffer base
				baseGeom = geomBufferArray[p];
															// Make sure it's defined
				if(baseGeom)
				{
															// Get query point object
					queryPoint = &(queryPoints[p]);
															// Get number of results (may be less than k)
					PTuint numResults = queryPoint->getNumResults();
															// Set number of results returned for this result set
					resultSetSize[p] = numResults;
															// Copy each result to supplied geometry buffer
					for(int64_t r = numResults - 1; r >= 0; r--)
					{
						queryPoint->getResultPop(voxelPoint);

						voxelPoint.getVertex(&(baseGeom[r * 3]));
					}
				}
			}
		}
		else
		{
            size_t n = queryPoints.size();
															// For each query point
			for(size_t p = 0; p < n; p++)
			{
															// Get it's geom destination buffer base
				baseGeom = geomBufferArray[p];

				if(rgbBufferArray)
					baseRGB  = rgbBufferArray[p];

				if(intensityBufferArray)
					baseIntensity = intensityBufferArray[p];

															// Make sure it's defined
				if(baseGeom)
				{
															// Get query point object
					queryPoint = &(queryPoints[p]);
															// Get number of results (may be less than k)
					PTuint numResults = queryPoint->getNumResults();
															// Set number of results returned for this result set
					resultSetSize[p] = numResults;
															// Copy each result to supplied geometry buffer
					for(int64_t r = numResults - 1; r >= 0; r--)
					{
                        int64_t i = r * 3;

						queryPoint->getResultPop(voxelPoint);

						voxelPoint.getVertex(&(baseGeom[i]));

						if(baseRGB)
							voxelPoint.getRGB(&(baseRGB[i]));

						if(baseIntensity)
							voxelPoint.getIntensity(baseIntensity[r]);
					}
				}
			}

		}
	}



	bool refineToChildren(PriorityQueueNodeMiniMax *h, PriorityQueueMiniMax &queue)
	{
		unsigned int		c;
		pcloud::Node	*	node;
		pcloud::Node	*	child;
		pt::BoundingBox		extents;
		float				distMin;
		float				distMax;
		bool				refined = false;

		node = h->getNode();
															// If leaf item, just copy and don't refine
		if(node->isLeaf())
		{
			queue.add(*h);
			return false;
		}
															// Refine to child nodes if defined
		for(c = 0; c < 8; c++)
		{
			if((child = node->child(c)) != NULL)
			{
				getExtents(child, *h->getVisitor(), extents);

				distMin = queryBox.minDistanceSquared(extents);
				distMax = queryBox.maxDistanceSquared(extents);

				PriorityQueueNodeMiniMax item(child, distMin, distMax, h->getVisitor());

				queue.add(item);

				refined = true;


			}
		}

		return refined;
	}

/*
	float calculatePruneDistance(PriorityQueueMiniMax &a, PriorityQueueMiniMax &e, int k)
	{
		float			pruneDistance = LARGE_NUMBER;
		unsigned long	aNumPoints = 0;

		PriorityQueueNodeMiniMax * h;

		unsigned int	priorityQueueSize;
		unsigned int	aSize;
		unsigned int	hNumPoints;
		float			distMax;

		pt::BoundingBox	extents;


		for(h = e.getFirst(); aNumPoints < k && e.getSize() > 0; h = e.getFirst())
		{
			e.popFirst();

			pcloud::Node *n = h->getNode();

priorityQueueSize = e.getSize();
aSize = a.getSize();

			if(n->isLeaf() == false)
				hNumPoints = n->lodPointCount();
			else
				hNumPoints = static_cast<pcloud::Voxel *>(n)->fullPointCount();

			if(aNumPoints + hNumPoints > k)
			{
				if(n->isLeaf())
				{
					a.add(*h);
					aNumPoints += hNumPoints;

					pruneDistance = h->getDistanceMax();
				}
				else
				{

					unsigned int	c;
					pcloud::Node *	child;

					for(c = 0; c < 8; c++)
					{
						if((child = n->child(c)) != NULL)
						{
							getExtents(child, *h->getVisitor(), extents);

							distMax = queryBox.maxDistanceSquared(extents);

							PriorityQueueNodeMiniMax item(child, 0, distMax);

float distMin = queryBox.minDistanceSquared(extents);

							e.add(item);
						}
					}
				}
			}
			else
			{
				a.add(*h);
				aNumPoints += hNumPoints;

				pruneDistance = h->getDistanceMax();
			}

		}


		return pruneDistance;
	}
*/
	/** Note that use of this version of getExtents that returns a float bounding box can result in
	 loss of precision with positions far from the origin, using the version below that returns a
	 double bounding box will give more accurate results.
	 */
	void getExtents(pcloud::Node *node, ReadPoints &visitor, pt::BoundingBox &result)
	{
        result.setBox(static_cast<float>(node->extents().ux()), static_cast<float>(node->extents().lx()),
					  static_cast<float>(node->extents().uy()), static_cast<float>(node->extents().ly()),
					  static_cast<float>(node->extents().uz()), static_cast<float>(node->extents().lz()));
		result.translateBy(pt::vector3(static_cast<float>(visitor.world2prj.x), static_cast<float>(visitor.world2prj.y), static_cast<float>(visitor.world2prj.z)));
	}

	void getExtents(pcloud::Node *node, ReadPoints &visitor, pt::BoundingBoxD &result)
	{
		result = node->extents();
		result.translateBy(visitor.world2prj);
	}

/*
	void calculatePrunedQueue(PriorityQueueMiniMax &a, PriorityQueueMiniMax &e, PriorityQueueMiniMin &g, float pruneDistance)
	{
		unsigned int				t;
		unsigned int				s;
		PriorityQueueNodeMiniMax *	h;

		s = a.getSize();
															// Add all items in 'a' sorted for MiniMin ordering
		for(t = 0; t < s; t++)
		{
			h = a.getFirst();
			a.popFirst();
															// Calculate query box to node's bounding box min distance
			h->setDistanceMin(queryBox.minDistanceSquared(h->getNode()->extents()));
															// Add using MiniMin
			g.add(*reinterpret_cast<PriorityQueueNodeMiniMin *>(h));
		}

		s = e.getSize();
															// Add all items in 'e' that overlap pruneDistance
															// sorted for MiniMin ordering
		for(t = 0; t < s; t++)
		{
			h = e.getFirst();
			e.popFirst();
															// Calculate query box to node's bounding box min distance
			h->setDistanceMin(queryBox.minDistanceSquared(h->getNode()->extents()));
															// If box is in range, 
			if(h->getDistanceMin() < pruneDistance)
			{
															// Add using MiniMin
				g.add(*reinterpret_cast<PriorityQueueNodeMiniMin *>(h));

h->getNode()->flag(WholeSelected, true);
			}
		}
	}
*/

	bool scene(pcloud::Scene *scene)
	{
		Query::sceneOnly = sceneFromHandle(scope);
		if(Query::sceneOnly != NULL && Query::sceneOnly != scene)
			return false;

		return true;
	}

												// Used to create initial set of clouds
	bool cloud(pcloud::PointCloud *cloud)
	{
		Query::pcloudOnly = cloudFromHandle(scope);
		if(Query::pcloudOnly != NULL && Query::pcloudOnly != cloud)
			return false;

												// Create a new entry in priorityQueue using current state of ReadPoints
		pcloud::Node *root = const_cast<pcloud::Node *>(cloud->root());

		pt::BoundingBox	extents;

		getExtents(root, *this, extents);

		float distMin = queryBox.minDistanceSquared(extents);
		float distMax = queryBox.maxDistanceSquared(extents);

												// Create new visitor for this cloud / octree
		visitors.push_back(*this);
												// Copy ReadPoints visitor
		PriorityQueueNodeMiniMax entry(const_cast<pcloud::Node *>(root), distMin, distMax, &(visitors.back()));

												// Add to priority queue
		e.add(entry);

		root->calcLodPointCount();
												// Return false so traversal doesn't go to child
		return false;
	}


	int runQuery(int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers)
	{
		return runQueryT(buffersize, geomBuffer, rgbBuffer, inten, classification, layers);
	}

	int runQuery(int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers)
	{
		return runQueryT(buffersize, geomBuffer, rgbBuffer, inten, classification, layers);
	}

	int runDetailedQuery(int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *filter, PTubyte *classification,
		PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels)
	{
		return 0;
	}

	int	runDetailedQuery(int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *filter, PTubyte *classification,
		PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels)
	{
		return 0;
	}

	void submitChannelUpdate(PThandle channel)
	{
	}


	int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTfloat **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
	{
		return runQueryMultiT(numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, inten);
	}

	int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTdouble **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
	{
		return runQueryMultiT(numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, inten);
	}

};



int s_lastQueryHandle;

};
/* Query */


//-----------------------------------------------------------------------------
// ptPointData
//-----------------------------------------------------------------------------
PTbool	PTAPI ptPointData( PThandle cloud, PThandle pntPartA, PThandle pntPartB,
						  PTdouble *position, PTshort *intensity,
						  PTubyte *rgb, PTfloat *normal )
{
	PointCloud *pc = cloudFromHandle(cloud);
	if (!pc) return false;

	if (pc->voxels().size() < pntPartA)
	{
		const Voxel *v = pc->voxels()[pntPartA];
		if (!v) return false;

		pt::vector3s snormal;
		pt::vector3 localpos;

		v->pointPosition(pntPartB, localpos);

		position[0] = localpos.x;
		position[1] = localpos.y;
		position[2] = localpos.z;

		pc->transform().matrix().mat3_multiply_vec3(position);

		v->pointRGB(pntPartB, rgb);
		v->pointIntensity(pntPartB, *intensity);
		v->pointNormal(pntPartB, snormal);

		normal[0] = (float)(snormal.x) / 32768;
		normal[1] = (float)(snormal.y) / 32768;
		normal[2] = (float)(snormal.z) / 32768;

		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
// ptPointData
//-----------------------------------------------------------------------------
PTuint	PTAPI ptPointAttributes( PThandle cloud,
								PThandle pntPartA, PThandle pntPartB  )
{
	PointCloud *pc = cloudFromHandle(cloud);
	if (!pc) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );	
		return 0;
	}

	PTuint attrib = 0;

	if (pc->voxels().size() < pntPartA)
	{
		const Voxel *v = pc->voxels()[pntPartA];
		if (!v)
		{
			setLastErrorCode( PTV_INVALID_HANDLE );		
			return 0;
		}

		if (v->channel(PCloud_RGB)) attrib |= PT_HAS_RGB;
		if (v->channel(PCloud_Intensity)) attrib |= PT_HAS_INTENSITY;
		if (v->channel(PCloud_Normal)) attrib |= PT_HAS_NORMAL;
	}
	return attrib;
}

//-----------------------------------------------------------------------------
// ptPointData
//-----------------------------------------------------------------------------
PTbool	PTAPI ptGetPointAttribute( PThandle cloud,
								  PThandle pntPartA, PThandle pntPartB,
								  PTuint attribute, void* data )
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	debugAssertM(0, _T("ptGetPointAttribute Not Implemented"));
	return false;
}
//-----------------------------------------------------------------------------
// find the Nearest point to this one
//-----------------------------------------------------------------------------
PTfloat PTAPI ptFindNearestPoint( PThandle scene, const PTdouble *pnt, PTdouble *nearest )
{
	PTTRACE_FUNC_P4( scene, pnt[0], pnt[1], pnt[2] )

	pt::vector3d vnearest;
	pt::vector3d npnt (pnt);
	npnt.x /= g_unitScale;
	npnt.y /= g_unitScale;
	npnt.z /= g_unitScale;

	if (scene)
	{
		const pcloud::Scene *sc = thePointsScene().sceneBySceneOrCloudKey(scene);
		if (sc)
		{
			PTdouble dist = sc->findNearestPoint(npnt, vnearest, pt::ProjectSpace );
			vnearest.get(nearest);
			nearest[0] *= g_unitScale;
			nearest[1] *= g_unitScale;
			nearest[2] *= g_unitScale;
			return static_cast<PTfloat>(dist * g_unitScale);
		}
		else
		{
			setLastErrorCode( PTV_INVALID_HANDLE );		
			return -1.0f;			
		}
	}
	else
	{
		PTdouble ndist = -1e6;
		for (int i=0; i<thePointsScene().size();i++)
		{
			pcloud::Scene *sc = thePointsScene()[i];
			PTdouble dist = sc->findNearestPoint( npnt, vnearest, pt::ProjectSpace );
			if (dist >= 0 && fabs(ndist) > dist)
			{
				vnearest.get(nearest);
				ndist = dist;
			}
		}
		nearest[0] *= g_unitScale;
		nearest[1] *= g_unitScale;
		nearest[2] *= g_unitScale;
		ndist *= g_unitScale;
        return static_cast<PTfloat>(ndist >= 0 ? ndist : -1.0f);
	}
	return -1.0f;
}
//
// Ray intersection cache
// cache the last 10 intersecting voxels because its likely to be the same ones again
//

//struct VoxelCache
//{
//	VoxelCache() {}
//	void addVoxel(Voxel*v)
//	{
//		if (!)
//		_voxels.push_back(v);
//
//		if (_voxels.size() > 10)
//			_voxels.erase(_voxels.begin());
//	}
//
//	std::list<Voxel*> _voxels;
//	std::set<Voxel*> _voxset;
//};
//typedef std::map <PThandle, VoxelCache*> SceneVoxelCache;
//SceneVoxelCache s_sceneVoxelCache;
struct RayIntersectionData
{
	RayIntersectionData()
	{
		cloudGUID = 0;
		voxelIndexInCloud = -1;
		intersectionRadius = 0.03;
		intersectionRadius2 = intersectionRadius * intersectionRadius;
	}
	pcloud::PointCloudGUID	cloudGUID;
	PTint					voxelIndexInCloud;
	PTfloat					intersectionRadius;
	PTfloat					intersectionRadius2;
};
RayIntersectionData	g_rayIntData;

//-----------------------------------------------------------------------------
// Get voxel 'amount' value and whether to load based on density type and density ceofficient
//-----------------------------------------------------------------------------

PTfloat getVoxelAmount(Voxel *vox, PTenum density, PTfloat densityCoeff, bool &doload)
{
	PTfloat amount;

	if(vox == NULL)
		return 0;


	switch (density)
	{
	case PT_QUERY_DENSITY_SPATIAL:					// spatially uniform density
		amount = 1.0f;
		doload = true;
		break;

	case PT_QUERY_DENSITY_VIEW:						// current view based density
		amount = densityCoeff * vox->getRequestLOD();
		break;

	case PT_QUERY_DENSITY_VIEW_COMPLETE:			// current view based density with load
		amount = densityCoeff * vox->getRequestLOD();
		doload = true;
		break;

	case PT_QUERY_DENSITY_LIMIT:					// density limited to a number of points
		amount = densityCoeff * vox->getRequestLOD();
		break;
													// Default to max so that something is visible	
	case PT_QUERY_DENSITY_FULL:						// full density, but modulated by coeff
	default:
		doload = true;
		amount = 1;
		break;
	}

	return amount;
}


//-----------------------------------------------------------------------------
// Distance to Ray PointsVisitor
//-----------------------------------------------------------------------------
template<class Real> class Dist2Ray : public PointsVisitor
{
public:

	Dist2Ray(const pt::Ray<Real> &_ray, const pcloud::Scene* sc, PTenum initDensity, PTfloat initDensityCoeff)
		: ray(_ray), mindist(RAY_LARGE_DISTANCE), rayToPntDist(RAY_LARGE_DISTANCE), minVoxelDist(RAY_LARGE_DISTANCE), checkScene(sc),
		_isolationFilterIntersect(pointsengine::ClipManager::instance().getIntersectFunction()),
		_isolationFilterInside(pointsengine::ClipManager::instance().getInsideFunction())	
	{
		ray.normalise();
		visLayers = thePointLayersState().pointVisLayerMask();

		density			= initDensity;
		densityCoeff	= initDensityCoeff;

		_basePoint = pt::vector3d(Project3D::project().registration().matrix()(3,0), 
					Project3D::project().registration().matrix()(3,1), 
					Project3D::project().registration().matrix()(3,2));
	}

	bool visitNode(const pcloud::Node *n)
	{
		return voxel(static_cast<Voxel*>(const_cast<pcloud::Node*>(n)));
	}

	bool scene(const pcloud::Scene *sc)	{ return sc == checkScene;  }

	bool cloud(pcloud::PointCloud *cloud)
	{
		if (cloud->displayInfo().visible())
			return (cloud->root()->extents().intersectsRay(ray));
		return false;
	}

	bool voxel(pcloud::Voxel *vox)
	{
											// move node extents into current project space
		pt::BoundingBoxD box = vox->extents();												
		box.translateBy(-_basePoint);																							
		ptedit::SelectionResult isolationRes = _isolationFilterIntersect(box);

		if ( vox->flag(pcloud::WholeClipped) || vox->flag(pcloud::WholeHidden) || (isolationRes == ptedit::FullyOutside))
			return true;

		vec3<Real> cen = vox->extents().center();
		vec3<Real> cenp;
		pt::Project3D::project().world2ProjectSpace(cen, cenp);

											// check if this voxel is closer to start of ray
		voxeldist = cenp.dist(ray.origin); 

		if ( voxeldist < minVoxelDist )
		{
			currentVoxel = vox;

            std::unique_lock<std::mutex> lock(vox->mutex(), std::try_to_lock);
			if (lock.owns_lock())
			{
				bool	doload = false;

				PTfloat amount = getVoxelAmount(currentVoxel, density, densityCoeff, doload);

				// adjust for point editing - so invisble points etc are not picked
				PTfloat edited_proportion = (float)currentVoxel->numPointsEdited() / currentVoxel->fullPointCount();

				if (currentVoxel->numPointsEdited() && amount > edited_proportion)
				{
					amount = edited_proportion;
				}

				// NOTE: SHOULD A LOAD EVER BE NECESSARY ???
//				pointsengine::VoxelLoader load( doload ? vox : 0, amount, false, false);

				vox->iterateTransformedPoints(*this, pt::ProjectSpace, 255, amount); 
			}
		}

		return true; /* ie continue */ 
	} 


	void point(const pt::vec3<Real> &pnt, int index, ubyte &f) 
	{
		double dist, t;

		if (!(f & visLayers)) return;

		// check point against clip planes							
		if (!_isolationFilterInside(0, pnt))
			return;

		if (ray.perpDistSq2Pnt(pnt, dist, t))
		{
			if (fabs(dist) < g_rayIntData.intersectionRadius2 &&  fabs(t) < (mindist+g_rayIntData.intersectionRadius*5))
			{
				mindist = fabs(t);
				rayToPntDist = fabs(dist);
				closestPnt.set(pnt);

				if (currentVoxel != closestVoxel)
				{
					minVoxelDist = voxeldist;
					closestVoxel = currentVoxel;
				}
			}
		}
	}

	inline bool setVoxel(Voxel *vox) { return true; };

protected:

	const pcloud::Scene		*	checkScene;
	pt::Ray<Real>				ray;
	double						minVoxelDist;
	double						voxeldist;

	ubyte						visLayers;

	ptedit::IsolationFilter::IntersectCallback	_isolationFilterIntersect;
	ptedit::IsolationFilter::InsideCallback		_isolationFilterInside;

	pt::vector3d				_basePoint;

public:
	
	double						rayToPntDist;
	PTenum						density;
	PTfloat						densityCoeff;
	double						mindist;
	pt::vector3d				closestPnt;
	pcloud::Voxel			*	closestVoxel;

};


//-----------------------------------------------------------------------------
// intersection radius
//-----------------------------------------------------------------------------
PTres PTAPI ptSetIntersectionRadius(PTfloat radius)
{
	PTTRACE_FUNC_P1( radius )

	if (fabs(radius) < 1e-6) 
		return setLastErrorCode( PTV_VALUE_OUT_OF_RANGE );

	g_rayIntData.intersectionRadius = radius;
	g_rayIntData.intersectionRadius2 = radius * radius;

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTfloat PTAPI ptGetIntersectionRadius(void)
{
	return g_rayIntData.intersectionRadius;
}
//-----------------------------------------------------------------------------
// Ray Intersection
//-----------------------------------------------------------------------------
PTbool PTAPI ptIntersectRay(PThandle scene, const PTdouble *origin,
							const PTdouble *direction, PTdouble *pnt, PTenum densityType, PTfloat densityValue)
{
	if (!direction || !origin || !pnt)
	{
		PTTRACE_FUNC

		return false;
	}
	PTTRACE_FUNC_P7(scene, origin[0], origin[1], origin[2], direction[0], direction[1], direction[2] )

	pt::BoundingBoxD bb;
	int numScenes = thePointsScene().size();
	double mindist = RAY_LARGE_DISTANCE;

	for (int i=0; i<(scene ? 1 : numScenes); i++)
	{
		const pcloud::Scene *sc = scene ?
			thePointsScene().sceneBySceneOrCloudKey(scene) :
			thePointsScene()[i];

		if (!sc || !sc->displayInfo().visible())
			continue;

		bb = sc->projectBounds().bounds();

		pt::Ray<double> ray;
		pt::vector3d nearest;

		ray.origin.set( origin );
		ray.origin.x /= g_unitScale;
		ray.origin.y /= g_unitScale;
		ray.origin.z /= g_unitScale;
		ray.direction.set( direction );

		Dist2Ray<double> raydistVisitor(ray, sc, densityType, densityValue);
 
		if (bb.intersectsRay(ray))
		{		
			int numClouds = sc->numObjects();
			for (int c=0; c<numClouds; c++)
			{
				const pcloud::PointCloud *pc = sc->cloud(c);

				/* move ray into world cs */
				pt::vec3<double> rayo;
				pt::Project3D::project().project2WorldSpace(ray.origin, rayo);
				ray.origin = rayo;

				if (pc->displayInfo().visible())
				{
					pc->root()->traverseRayIntersecting(&raydistVisitor, ray);
				}
			}
			//to do: a node based approach for faster results
			//thePointsScene().visitVoxels( &raydistVisitor );

			if (raydistVisitor.mindist < mindist)
			{
				raydistVisitor.closestPnt.get(pnt);
				pnt[0] *= g_unitScale;
				pnt[1] *= g_unitScale;
				pnt[2] *= g_unitScale;
				g_rayIntData.voxelIndexInCloud = raydistVisitor.closestVoxel->indexInCloud();
				g_rayIntData.cloudGUID = raydistVisitor.closestVoxel->pointCloud()->guid();

				mindist = raydistVisitor.mindist;
			}
		}
	}
	return mindist == RAY_LARGE_DISTANCE ? false : true;
}
//-----------------------------------------------------------------------------
// Intersect Ray
//-----------------------------------------------------------------------------
PTbool PTAPI ptIntersectRayPntIndex(PThandle scene, const PTdouble *origin,
							const PTdouble *direction, PThandle *cloud,
							PThandle *pntPartA, PThandle *pntPartB )
{
	debugAssertM(0, L"ptIntersectRayPntIndex: Method not imlpemented, try ptIntersectRay");
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	return false;
}
//-----------------------------------------------------------------------------
// Interpolated Ray Intersection, not implemented
//-----------------------------------------------------------------------------
PTbool PTAPI ptIntersectRayInterpolated(PThandle scene, const PTdouble *origin, const PTdouble *direction, PThandle *tmpPointHandle)
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	return false;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Querying for points extraction to user buffers
//-----------------------------------------------------------------------------
PTbool PTAPI ptResetQuery( PThandle query )
{
	PTTRACE_FUNC_P1(query)

	using namespace querydetail;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{
		i->second->resetQuery();
		return true;
	}
	setLastErrorCode( PTV_INVALID_HANDLE );
	return false;
}
//-----------------------------------------------------------------------------
// Visible Points Query
//-----------------------------------------------------------------------------
/*struct FrustumCondition
{
	static bool nodeCheck(const Node *n)
	{
		return  true;
	}
	static bool boundsCheck(const pt::BoundingBoxD &box) { return true; }
	static bool processWhole(const Node *n) { return true; }
	static bool escapeWhole(const Node *n) { return false; }
	inline static bool validPoint(const vector3d &pnt, ubyte &f) { return true; }
};*/
//-----------------------------------------------------------------------------
// Selected Point Query
//-----------------------------------------------------------------------------
struct VisibleCondition
{
	VisibleCondition() : m_clipManager(pointsengine::ClipManager::instance()) { ; }
	bool nodeCheck(const Node *n)
	{
		return ( (n->flag( pcloud::PartHidden ) || (!n->flag(pcloud::WholeHidden))) 
			&& (!n->flag( pcloud::WholeClipped )) )
			? true : false;
	}
	static const char* name()  { return "VISIBLE"; }
	bool boundsCheck(const pt::BoundingBoxD &box) { return true; }
	bool processWhole(const Node *n)
	{ 
		return (!(n->flag(pcloud::WholeHidden)) || !(n->flag( pcloud::PartClipped ))); 
	}
	bool escapeWhole(const Node *n) 
	{ 
		return (n->flag(pcloud::WholeHidden) || n->flag(pcloud::WholeClipped)); 
	}
	inline bool validPoint(const vector3d &pnt, ubyte &f) 
	{ 
		return ( (thePointLayersState().pointVisLayerMask() & f) && m_clipManager.inside(pnt) ) ? true : false;
	}

	pointsengine::ClipManager& m_clipManager;
};
//-----------------------------------------------------------------------------
// Visible points condition, ie everything in the scene that is visible
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateVisPointsQuery()
{
	PTTRACE_FUNC

	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		s_queries.insert( QueryMap::value_type(h, new ConditionQuery<VisibleCondition>) ); ///!ToDo: Exception safety: avoid leaks [Trac #315]
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}

//-----------------------------------------------------------------------------
// All points condition, ie everything in the scene that is visible
// used in restricted PODReader build
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateAllPointsQuery()
{
	PTTRACE_FUNC

	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		querydetail::Query *q = new ConditionQuery<NullCondition>; ///!ToDo: Exception safety: avoid leaks [Trac #315]
		q->setDensity( PT_QUERY_DENSITY_FULL, 1.0f );				//all points
		s_queries.insert( QueryMap::value_type( h, q ));
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}
//-----------------------------------------------------------------------------
// SimpleTuple
// provides a generic (template happy) way to uniformly deal with small collections
// of values. This is used for UserChannel queries only
//-----------------------------------------------------------------------------

template <typename T, int M>
struct SimpleTuple
{
	bool operator > (const SimpleTuple &t) const
	{
		for (int i=0; i<M;i++) if (v[i] <= t.v[i]) return false;
		return true;
	}
	bool operator < (const SimpleTuple &t) const
	{
		for (int i=0; i<M;i++) if (v[i] >= t.v[i]) return false;
		return true;
	}
	bool operator == (const SimpleTuple &t) const
	{
		for (int i=0; i<M;i++) if (v[i] != t.v[i]) return false;
		return true;
	}
	const SimpleTuple &operator = (const SimpleTuple &t)
	{
		for (int i=0; i<M;i++) v[i] = t.v[i];
		return (*this);
	}
	bool operator >= (const SimpleTuple &t) const
	{
		for (int i=0; i<M;i++) if (v[i] < t.v[i]) return false;
		return true;
	}
	bool operator <= (const SimpleTuple &t) const
	{
		for (int i=0; i<M;i++) if (v[i] > t.v[i]) return false;
		return true;
	}
	T v[M];
};
//-----------------------------------------------------------------------------
// UCValue
// provides abstracted access to userchannel and range comparisions 
//-----------------------------------------------------------------------------
template <class T>
struct UCValue
{
	typedef T ValueType; 

	static bool hasValuesInRange(VoxelChannelData *vc, void *vmin, void *vmax)
	{
		return (vc->isUniform()) ? compareToUniform(vc,vmin,vmax) : compareToRange(vc,vmin,vmax);
	}
	static bool compareToUniform(VoxelChannelData *vc, void *vmin, void *vmax)
	{
		T uniform;
		if (vc->getUniformVal(uniform))
		{
			const T &minVal = *reinterpret_cast<const T*>(vmin);
			const T &maxVal = *reinterpret_cast<const T*>(vmax);

			return (uniform >= minVal && uniform <= maxVal) ? true : false;
		}
		return false;
	}
	static bool compareToRange(VoxelChannelData *vc, void *vmin, void *vmax)
	{
		T cmin, cmax;
		if (vc->getMinVal(cmin) && vc->getMaxVal(cmax))
		{
			const T &minVal = *reinterpret_cast<const T*>(vmin);
			const T &maxVal = *reinterpret_cast<const T*>(vmax);

			return (cmin >= minVal && cmax <= maxVal) ? true : false;
		}
		return false;
	}
	static bool valueInRange(VoxelChannelData *vc, int index, void *vmin, void *vmax)
	{
		T value;
		if (vc->getValR(index, value))
		{
			const T &minVal = *reinterpret_cast<const T*>(vmin);
			const T &maxVal = *reinterpret_cast<const T*>(vmax);

			return (value >= minVal && value <= maxVal) ? true : false;
		}
		return false;
	}
	static void copy(void *into, const void *value)	{ memcpy(into, value, sizeof(T)); }
};
//-----------------------------------------------------------------------------
// Condition for user channel queries
//-----------------------------------------------------------------------------
template <class UCValueCompare>
struct UserChannelCondition
{
	UserChannelCondition( PThandle userChannel, const PTvoid *minValue, const PTvoid *maxValue)
		: m_queryChannel( userChannel )
	{
		m_vxData = 0;
		UCValueCompare::copy(m_minValue, minValue);
		UCValueCompare::copy(m_maxValue, maxValue);
	}

	bool nodeCheck(const Node *n)	
	{ 
		m_channel = _ptGetUserChannel(  m_queryChannel );	// make sure its still valid, do for every node, fast enough

		if (m_channel)
		{
			if (n->isLeaf())
			{
				const pcloud::Voxel* v = static_cast<const pcloud::Voxel*>(n);

				if (m_channel)
					m_vxData = m_channel->voxelChannel(v);
				else
					m_vxData = 0;

				m_pntIndex = -1;	// to enable pre-increment
			}
		}
		return (m_channel) ? true : false; 
	}

	static const char* name()		{ return "USERCHANNEL"; }

	bool boundsCheck(const pt::BoundingBox &box)	{ return true; }
	bool processWhole(const Node *n)				{ return false; }
	void prepareToTestVoxelPointsInWorldspaceBBox(const pt::BoundingBox &box, PTint numPoints) {}
	static void onResetQuery() {}

	bool escapeWhole(const Node *n)					
	{ 
		if (n->isLeaf())
		{
			const pcloud::Voxel* v = static_cast<const pcloud::Voxel*>(n);
			
			if (m_vxData)
			{
				UCValueCompare::hasValuesInRange(m_vxData, m_minValue, m_maxValue);
			}
		}
		return false;
	}
	bool validPoint(const vector3d &pnt, ubyte &f)
	{ 
		return UCValueCompare::valueInRange( m_vxData, ++m_pntIndex, m_minValue, m_maxValue);
	}	
	VoxelChannelData	*m_vxData;
	int					m_bytesPerValue;
	int					m_multiple;

	UserChannel			*m_channel;
	PThandle			m_queryChannel;
	PTvoid				*m_minValue;
	PTvoid				*m_maxValue;
	PTint				m_pntIndex;
};
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel
// template function. Type must be specified via min/max values. Multiple is
// deduced from the UserChannel configuration
//-----------------------------------------------------------------------------
template <typename T>
PThandle t_ptCreateUserChannelQuery( PThandle userChannel, T *minValue, T *maxValue )
{
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		UserChannel *uc = _ptGetUserChannel(userChannel);
		if (!uc)	
		{
			setLastErrorCode( PTV_INVALID_HANDLE );
			return 0;
		}
		querydetail::Query *q=0;

		// could have used a typelist?
		switch(uc->multiple())
		{
		case 1:
			{
			typedef UserChannelCondition<UCValue<T> > UCCondition;
			q = new ConditionQuery<UCCondition>( UCCondition(userChannel, minValue, maxValue), h );
			break;
			}
		case 2:
			{
			typedef UserChannelCondition<UCValue<SimpleTuple<T,2> > > UCCondition;
			q = new ConditionQuery<UCCondition>( UCCondition(userChannel, minValue, maxValue), h );
			break;
			}

		case 3:
			{
				typedef UserChannelCondition<UCValue<SimpleTuple<T,3> > > UCCondition;
				q = new ConditionQuery<UCCondition>( UCCondition(userChannel, minValue, maxValue), h );
				break;			
			}
		case 4:
			{
				typedef UserChannelCondition<UCValue<SimpleTuple<T,4> > > UCCondition;
				q = new ConditionQuery<UCCondition>( UCCondition(userChannel, minValue, maxValue), h );
				break;			
			}
		case 5:
			{
				typedef UserChannelCondition<UCValue<SimpleTuple<T,5> > > UCCondition;
				q = new ConditionQuery<UCCondition>( UCCondition(userChannel, minValue, maxValue), h );
				break;			
			}
		}
		if (!q)
		{
			setLastErrorCode( PTV_INVALID_PARAMETER );
			--querydetail::s_lastQueryHandle;
			return 0;
		}
		q->setDensity( PT_QUERY_DENSITY_FULL, 1.0f );				//all points
		s_queries.insert( QueryMap::value_type( h, q ));
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		--querydetail::s_lastQueryHandle;
		return 0;
	}
	return h;
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryd( PThandle userChannel, PTdouble *minValue, PTdouble *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryf( PThandle userChannel, PTfloat *minValue, PTfloat *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryi( PThandle userChannel, PTint *minValue, PTint *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryui( PThandle userChannel, PTuint *minValue, PTuint *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryb( PThandle userChannel, PTbyte *minValue, PTbyte *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryub( PThandle userChannel, PTubyte *minValue, PTubyte *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQuerys( PThandle userChannel, PTshort *minValue, PTshort *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Create a query to extract values from a User Channel, int version
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateUserChannelQueryus( PThandle userChannel, PTushort *minValue, PTushort *maxValue )
{
	return t_ptCreateUserChannelQuery(userChannel, minValue, maxValue);
}
//-----------------------------------------------------------------------------
// Selected Point Query
//-----------------------------------------------------------------------------

struct SelectedCondition
{
	static bool nodeCheck(const Node *n)
	{
		return (n->flag( pcloud::PartSelected ) || n->flag(pcloud::WholeSelected ))
			? true : false;
	}
	static const char* name()  { return "SELECTED"; }
	static bool boundsCheck(const pt::BoundingBoxD &box) { return true; }
	static bool processWhole(const Node *n) { return n->flag(pcloud::WholeSelected); }
	static bool escapeWhole(const Node *n) { return n->flag(pcloud::WholeHidden); }
	inline static bool validPoint(const vector3d &pnt, ubyte &f) { return f & SELECTED_PNT_BIT ? true : false; }
};
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateSelPointsQuery()
{
	PTTRACE_FUNC

	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		s_queries.insert( QueryMap::value_type(h, new ConditionQuery<SelectedCondition>) ); ///!ToDo: Exception safety: avoid leaks [Trac #315]
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}

//-----------------------------------------------------------------------------

PThandle PTAPI ptCreateKNNQuery(PTfloat *vertices, PTint numQueryVertices, PTint k, PTfloat lod = 1)
{
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		s_queries.insert( QueryMap::value_type(h, new KNearestNeighbourQuery(vertices, numQueryVertices, k, lod)) ); ///!ToDo: Exception safety: avoid leaks [Trac #315]
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}

//-----------------------------------------------------------------------------
// Frusutm Points Query
//-----------------------------------------------------------------------------
namespace querydetail
{
struct FrustumCondition
{
	FrustumCondition() : m_clipManager(pointsengine::ClipManager::instance()) { ; }	

	bool nodeCheck(const Node *n)
	{	
		return ( n->flag( pcloud::Visible ) && (!n->flag( pcloud::WholeHidden )) && (!n->flag( pcloud::WholeClipped )) ) ? true : false;
	}
	bool boundsCheck(const pt::BoundingBoxD &box) { return true; }
	bool processWhole(const Node *n) 
	{ 
		return nodeCheck(n) && (!n->flag( pcloud::PartClipped )); 
	}
	bool escapeWhole(const Node *n) { return !(nodeCheck(n)); }
	inline bool validPoint(const vector3d &pnt, ubyte &f)
	{ 		
		return ( (thePointLayersState().pointVisLayerMask() & f) && m_clipManager.inside(pnt) ) ? true : false;
	}	

	pointsengine::ClipManager& m_clipManager;
};
//-----------------------------------------------------------------------------
inline static int voxzorder(const pcloud::Voxel *a, const pcloud::Voxel *b)
{
	return a->priority() > b->priority() ? 1 : 0;
}
//-----------------------------------------------------------------------------
struct FrustumQuery : public Query
{
	FrustumQuery(PThandle id)
	{
		cs = pt::ProjectSpace;
		_writer = 0;

		stats.m_numPoints = 0;
		stats.m_queryID = id;
		strcpy_s( stats.m_queryType, sizeof(stats.m_queryType), "FRUSTUM" );

		if (g_currentViewParams)
			theVisibilityEngine().setViewParameters( *g_currentViewParams );
	}
	~FrustumQuery()
	{
		if (_writer) delete _writer;
	}
	bool doesOwnDensityLimitCompute() const { return true; }
	
	uint selectQueryPoints(bool select)	{ return 0; }	// not implemented, has no real use

	void resetDiagnostics()
	{
		stats.m_density = 0;
		stats.m_numPoints = 0;
	}
	//-------------------------------------------------------------------------
	void resetQuery()
	{
#ifdef HAVE_OPENGL
		extern ptgl::Camera g_camera;
		extern ptgl::Light g_light;
#endif 
		Query::resetQuery();

		/*	if (_writer) delete _writer;
		_writer = 0; */

        if (g_currentViewParams)
			theVisibilityEngine().setViewParameters( *g_currentViewParams );

#ifdef HAVE_OPENGL
        if (!g_camera.getLight()){
			g_camera.setLight(&g_light);
		}
		g_light.setupGL();
#endif

		theVisibilityEngine().computeVisibility();

		voxels.clear();
		voxels.reserve(1024);

		if (stats.m_numPoints)	//completed query
		{
			ptdg::Time::endStamp( stats );
			ptdg::Diagnostics::instance()->addMetric( stats );
		}
		resetDiagnostics();

		pointsengine::thePointsPager().KBytesLoaded(true);
	}
	//-------------------------------------------------------------------------
	template <class T>
	int runQueryT( int buffersize, T *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
		PTubyte *layers=0, PTubyte *cf=0, PTuint numPointChannels=0, 
		const PThandle *pointChannelsReq=0, PTvoid **pointChannels=0)
	{
		if (!lastVoxel)
			ptdg::Time::stamp( stats );

		FrustumCondition C;

		if (this->isReset())
		{
			GatherVoxelsVisitor<FrustumCondition> gather(&C, voxels, scope);
			
			if (!thePointsScene().size()) voxels.clear();

			/* initialise query */
			if (!voxels.size())
			{
				thePointsScene().visitNodes( &gather, false );
				std::sort( voxels.begin(), voxels.end(), voxzorder);

				if (density == PT_QUERY_DENSITY_LIMIT && pointLimit)
				{
					if (pointLimit < gather.lodPointCount)
					{
						densityCoeff = (float)pointLimit / gather.lodPointCount;
					}
					else densityCoeff = 1.0f;
				}
				density = PT_QUERY_DENSITY_VIEW;
			}
		}

		if (_writer) delete _writer;
		_writer = 0;

		stats.m_density = density;
		stats.m_densityVal = densityCoeff;
		stats.m_bufferSize = buffersize;

		ReadPoints<FrustumCondition, T> reader( &C, buffersize, geomBuffer, rgbBuffer, inten, false,
			lastVoxel, lastPnt, density, densityCoeff, 0, rgbMode, layers, cf, numPointChannels, pointChannelsReq, pointChannels, 
			getLastPartiallyIteratedVoxel(), getLastPartiallyIteratedVoxelUnloadLOD(), layerMask);

		reader.cs = cs;

		if (numPointChannels && pointChannels)
		{
			_writer = new ReadPoints<FrustumCondition, T>( &C, buffersize, geomBuffer, rgbBuffer, inten, true,
			lastVoxel, lastPnt, density, densityCoeff, 0, rgbMode, layers, cf, numPointChannels, pointChannelsReq, pointChannels ,
			0, 0, layerMask );
		}
		/* run query on voxel list */ 
		for (int i=0; i<voxels.size(); i++)
		{
			if (!reader.voxel(voxels[i])) 
				break;
		}
															// Process any remaining voxels that did not get processed during visitNodes()
		reader.processDeferredVoxels();
															// Release the stream manager in case any remote access was carried out
															// Note: This end() does not need to have a matching begin()
		getStreamManager().end();


		lastPnt = reader.rwPos.lastPoint;
		lastVoxel = reader.rwPos.lastVoxel;

		setLastPartiallyIteratedVoxel(reader.rwPos.getLastPartiallyIteratedVoxel());
		setLastPartiallyIteratedVoxelUnloadLOD(reader.rwPos.getLastPartiallyIteratedVoxelUnloadLOD());

		stats.m_numPoints += reader.rwPos.counter;
		
		if (buffersize != reader.rwPos.counter)	//completed query
		{
			ptdg::Time::endStamp( stats );
			ptdg::Diagnostics::instance()->addMetric( stats );
			resetDiagnostics();
		}

		return reader.spatialGridFailure ? -1 : reader.rwPos.counter;
	}
	//-------------------------------------------------------------------------
	int runQuery( int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )
	{		
		return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, layers, classification  );
	}
	//-------------------------------------------------------------------------
	int runQuery( int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten, PTubyte *classification, PTubyte *layers )
	{
		return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, layers, classification  );
	}
	//-------------------------------------------------------------------------
	int runDetailedQuery( int buffersize, PTdouble *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
		PTubyte *filter, PTubyte *cf, PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
	{	
		return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, filter, cf, numPointChannels, pointChannelsReq, pointChannels  );
	}
	//-------------------------------------------------------------------------
	int runDetailedQuery( int buffersize, PTfloat *geomBuffer, ubyte *rgbBuffer, PTshort *inten,
		PTubyte *filter, PTubyte *cf, PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
	{
		return runQueryT( buffersize, geomBuffer, rgbBuffer, inten, filter, cf, numPointChannels, pointChannelsReq, pointChannels );
	}
	//-------------------------------------------------------------------------
	void submitChannelUpdate(PThandle channel)
	{
		if (_writer)
		{
			for (int i=0; i<voxels.size(); i++)
			{
				if (!_writer->voxel(voxels[i]))
					break;
			}
			//thePointsScene().visitNodes( _writer, false );
		}
	}

	//---------------------------------------------------------------------
	int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTfloat **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
	{
		return 0;
	}

	//---------------------------------------------------------------------
	int runQueryMulti(int numResultSets, int bufferSize, PTuint *resultSetSize, PTdouble **geomBufferArray, PTubyte **rgbBufferArray, PTshort **inten)
	{
		return 0;
	}


	PointsVisitor *_writer;

	pt::CoordinateSpace cs;
	std::vector<pcloud::Voxel *> voxels;
};
} // namespace::queryDetail
//-----------------------------------------------------------------------------
// Frustum Points Query
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateFrustumPointsQuery()
{
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	try
	{
		s_queries.insert( QueryMap::value_type(h, new FrustumQuery(h) ) ); ///!ToDo: Exception safety: avoid leaks [Trac #315]
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}
//-----------------------------------------------------------------------------
// Bounding Box Query
//-----------------------------------------------------------------------------
namespace querydetail
{
struct BoxCondition
{
	BoxCondition( const BoxCondition &b ) { bb = b.bb; bb.makeValid(); m_clipManager = b.m_clipManager; }
	BoxCondition( const pt::BoundingBoxD &box ) : bb(box), m_clipManager(&pointsengine::ClipManager::instance()) {}	

	static const char* name()  { return "BOX"; }

	bool nodeCheck(const Node *n)	{ return !n->flag( pcloud::WholeClipped ); }
	bool boundsCheck(const pt::BoundingBoxD &box) { nodeBox = box; return bb.intersects( &box ); }

	bool processWhole(const Node *n)
	{
		return (bb.contains(&nodeBox) && (!n->flag( pcloud::PartClipped ))); 
	}
	bool escapeWhole(const Node *n)
	{
		return ((!nodeBox.intersects(&bb) && !nodeBox.contains(&bb)) || n->flag( pcloud::WholeClipped )); 
	}

	inline bool validPoint(const vector3d &pnt, ubyte &f)
	{
		static double pf[3];
		pf[0] = pnt.x; pf[1] = pnt.y; pf[2] = pnt.z;
		bool in = (bb.inBounds(pf) && m_clipManager->inside(pnt));
		return in;
	}
	pt::BoundingBoxD bb;
	pt::BoundingBoxD nodeBox;
	pointsengine::ClipManager* m_clipManager;
};

//-----------------------------------------------------------------------------

struct OrientedBoxCondition
{
	pt::BoundingBoxD	bb;

	mvector4d			position;
	mvector4d			uAxis;
	mvector4d			vAxis;
	mvector4d			wAxis;

	mmatrix4d			transformWorldToLocal;

	pt::BoundingBoxD	nodeBox;

	bool				lastTestSeparate;

	pointsengine::ClipManager* m_clipManager;


	OrientedBoxCondition( const OrientedBoxCondition &b )
	{ 
		bb = b.bb; bb.makeValid(); 

		position	= b.position;
		uAxis		= b.uAxis;
		vAxis		= b.vAxis;
		wAxis		= b.wAxis;

		transformWorldToLocal	= b.transformWorldToLocal;
		//lastTestSeparate = false;
		
		m_clipManager = b.m_clipManager;
	}

	OrientedBoxCondition( const pt::BoundingBoxD &box ) 
		: bb(box),
		m_clipManager(&pointsengine::ClipManager::instance())
		/*, lastTestSeparate(false) */
	{}

	static const char* name() 
	{
		return "ORIENTEDBOX";
	}

	bool nodeCheck(const Node *n)
	{ 
		return !n->flag( pcloud::WholeClipped );
	}



//	bool boundsCheck(const pt::BoundingBox &box) { nodeBox = box; return bb.intersects( &box ); }

		bool boundsCheck(const pt::BoundingBoxD &box)
		{
			nodeBox = box;									// Note: This is very modal ! Not thread safe.
			lastTestSeparate = false;
															// Create box vertices in world coordinates
			mvector4d	c1(box.lower(0), box.lower(1), box.lower(2));
			mvector4d	c2(box.upper(0), box.lower(1), box.lower(2));
			mvector4d	c3(box.upper(0), box.upper(1), box.lower(2));
			mvector4d	c4(box.lower(0), box.upper(1), box.lower(2));

			mvector4d	c5(box.lower(0), box.lower(1), box.upper(2));
			mvector4d	c6(box.upper(0), box.lower(1), box.upper(2));
			mvector4d	c7(box.upper(0), box.upper(1), box.upper(2));
			mvector4d	c8(box.lower(0), box.upper(1), box.upper(2));

			mvector4d	tc1, tc2, tc3, tc4;
			mvector4d	tc5, tc6, tc7, tc8;

															// Transform box vertices into OOBB's local coordinate system
			c1.vec3_multiply_mat4(transformWorldToLocal, tc1);
			c2.vec3_multiply_mat4(transformWorldToLocal, tc2);
			c3.vec3_multiply_mat4(transformWorldToLocal, tc3);
			c4.vec3_multiply_mat4(transformWorldToLocal, tc4);
			c5.vec3_multiply_mat4(transformWorldToLocal, tc5);
			c6.vec3_multiply_mat4(transformWorldToLocal, tc6);
			c7.vec3_multiply_mat4(transformWorldToLocal, tc7);
			c8.vec3_multiply_mat4(transformWorldToLocal, tc8);

			pt::BoundingBoxD approxBox;
															// Calculate extents of transformed box to form an approximate AABB
			approxBox.expand(tc1.data());
			approxBox.expand(tc2.data());
			approxBox.expand(tc3.data());
			approxBox.expand(tc4.data());
			approxBox.expand(tc5.data());
			approxBox.expand(tc6.data());
			approxBox.expand(tc7.data());
			approxBox.expand(tc8.data());
															// Test whether OOBB contains approximated AABB
			if(bb.contains(&approxBox))
				return true;								// FullyInside
															// If test box contains this oriented bounding box, it is partial
															// and must therefore be tested further
			if(approxBox.contains(&bb))
				return true;								// PartiallyInside

			if(bb.intersects(&approxBox))
				return true;								// PartiallyInside

			lastTestSeparate = true;
															// Box must be outside
			return false;									// FullyOutside
		}


	bool processWhole(const Node *n)
	{
															// Without full calculation, we never know this state accurately
		return false;
//		return bb.contains(&nodeBox);
	}

	bool escapeWhole(const Node *n)
	{
//		return false;
		return lastTestSeparate || n->flag( pcloud::WholeClipped );
//		return !nodeBox.intersects(&bb) && !nodeBox.contains(&bb);
	}

/* Original
	inline bool validPoint(const vector3d &pnt, ubyte &f)
	{
		static float pf[3];
		pf[0] = pnt.x; pf[1] = pnt.y; pf[2] = pnt.z;
		bool in = bb.inBounds(pf);
		return in;
	}
*/

//	inline static bool inside(int thread, const OrientedBoxSelect &boxSelect, const pt::vector3d &pnt)
	inline bool validPoint(const vector3d &pnt, ubyte &f)
	{
		mvector4d p(pnt[0], pnt[1], pnt[2], 1);
		mvector4d localP;
															// Transform point into local coordinate system
		p.vec3_multiply_mat4(transformWorldToLocal, localP);
															// Test
		return (bb.inBounds(localP.data()) && m_clipManager->inside(pnt));
	}



		void setTransform(const pt::vector3d &initPosition, const pt::vector3d &initUAxis, const pt::vector3d &initVAxis)
		{
			// Copy position
			position(0, 0)	= initPosition[0];
			position(1, 0)	= initPosition[1];
			position(2, 0)	= initPosition[2];
			position(3, 0)	= 0;
			// Copy local coordinate system U and V vectors (X and Y)
			uAxis(0, 0)		= initUAxis[0];
			uAxis(1, 0)		= initUAxis[1];
			uAxis(2, 0)		= initUAxis[2];
			uAxis(3, 0)		= 0;

			vAxis(0, 0)		= initVAxis[0];
			vAxis(1, 0)		= initVAxis[1];
			vAxis(2, 0)		= initVAxis[2];
			vAxis(3, 0)		= 0;
			// Make sure input vectors are normalized
			uAxis.normalize();
			vAxis.normalize();
			// Calculate W axis (Z) as cross product of U (X) and V (Y)
//			matrix<4,1> wAxis;
			wAxis = uAxis;
			wAxis.cross(vAxis);
			wAxis.normalize();

			// Calculate inverse translation matrix
			mvector4d invTransVec = -position;
			invTransVec(3, 0) = 1;

			mmatrix4d	invTranslation;
			mmatrix4d	invRotation;
			// Build inverse translation matrix
			invTranslation = mmatrix4d::identity();
			invTranslation = mmatrix4d::translation(invTransVec.data());

			// Calculate inverse rotation matrix
			invRotation = mmatrix4d::identity();
			invRotation.setVector(0, uAxis.data());
			invRotation.setVector(1, vAxis.data());
			invRotation.setVector(2, wAxis.data());
			// Calculate world to local transformation matrix
			// as invTranslation * invRotation
			transformWorldToLocal = invTranslation >> invRotation;
		}		
};

//-----------------------------------------------------------------------------
struct SphereCondition
{
	SphereCondition( const SphereCondition &b ) { cen  = b.cen; radSqr = b.radSqr; m_clipManager = b.m_clipManager; }
	SphereCondition( const pt::vector3d &c, double rad ) : m_clipManager(&pointsengine::ClipManager::instance())
	{
		cen = c;
		radSqr = rad * rad;
	}
	static const char* name()  { return "SPHERE"; }
	bool sphereContains()
	{
		static ubyte f; 
		vector3d v;
		for (int c=0; c<8; c++)
		{
			nodeBox.getExtrema(c, v);
			if (!validPoint( v, f ))	return false;
		}
		return true;
	}
	//-------------------------------------------------------------------------
	bool boxIntersects()
	{
		if (nodeBox.inBounds( cen ) ) return true;

		static ubyte f;
		vector3d v;
		for (int c=0; c<8; c++)
		{
			nodeBox.getExtrema(c, v);
			if (validPoint( v, f ))	return true;
		}
		/* check for overlap */
		double s, d = 0;
		for( int i=0 ; i<3 ; i++ )
		{
			if( cen[i] < nodeBox.lower(i) )
			{
				s = cen[i] - nodeBox.lower(i);
				d += s*s;
			}
			else if( cen[i] > nodeBox.upper(i) )
			{
				s = cen[i] - nodeBox.upper(i);
				d += s*s;
			}
		}
		return d <= radSqr ? true : false;
	}
	bool nodeCheck(const Node *n)	{ return !n->flag( pcloud::WholeClipped ); }
	bool boundsCheck(const pt::BoundingBoxD &box) { nodeBox = box; return boxIntersects(); }
	bool processWhole(const Node *n) { return (sphereContains() && (!n->flag( pcloud::PartClipped ))); }
	bool escapeWhole(const Node *n) { return (!boxIntersects() || n->flag( pcloud::WholeClipped )); }

	inline bool validPoint(const vector3d &pnt, ubyte &f)
	{
		return ((pnt.dist2(cen) < radSqr) && m_clipManager->inside(pnt)) ? true : false;
	}
	double radSqr;
	pt::vector3d cen;
	pt::BoundingBoxD nodeBox;
	pointsengine::ClipManager* m_clipManager;
};
}
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateBoundingBoxQuery( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz )
{
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	BoxCondition  box( pt::BoundingBoxD(maxx, minx, maxy, miny, maxz, minz) );

	try
	{
		s_queries.insert( QueryMap::value_type(h, 
			new ConditionQuery< BoxCondition >( BoxCondition( box ), h )
			) );
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}

	return h;
}
//-----------------------------------------------------------------------------
// Bounding Sphere Query
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateBoundingSphereQuery( PTdouble *cen, PTdouble radius )
{
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	if (!cen || radius < 1e-6) 
	{
		PTTRACE_FUNC

		setLastErrorCode( PTV_INVALID_PARAMETER );
		return 0;
	}
	PTTRACE_FUNC_P4( cen[0], cen[1], cen[2], radius ) 

	try
	{
		s_queries.insert( QueryMap::value_type(h, new ConditionQuery< SphereCondition > 
			( SphereCondition( cen, radius ), h ) ) );
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}
	return h;
}
//-----------------------------------------------------------------------------
// Delete the query
//-----------------------------------------------------------------------------
PTbool PTAPI ptDeleteQuery( PThandle query )
{
	PTTRACE_FUNC_P1( query )

	using namespace querydetail;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{
		delete i->second;
		s_queries.erase(i);

		return PT_TRUE;
	}
	setLastErrorCode( PTV_INVALID_HANDLE );
	return PT_FALSE;
}
//-----------------------------------------------------------------------------
// Other queries not yet implemented
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateKrigSurfaceQuery( PTuint numPoints, PTdouble *pnts ) 
{ 
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
	return 0; 
}


PThandle PTAPI ptCreateOrientedBoundingBoxQuery( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz, PTdouble posx, PTdouble posy, PTdouble posz, PTdouble ux, PTdouble uy, PTdouble uz, PTdouble vx, PTdouble vy, PTdouble vz)
{ 
	using namespace querydetail;
	PThandle h = ++querydetail::s_lastQueryHandle;

	OrientedBoxCondition  orientedbox(pt::BoundingBoxD(maxx, minx, maxy, miny, maxz, minz));

	pt::vector3d	position(posx, posy, posz);
	pt::vector3d	UAxis(ux, uy, uz);
	pt::vector3d	VAxis(vx, vy, vz);

	orientedbox.setTransform(position, UAxis, VAxis);
	
	try
	{
		s_queries.insert( QueryMap::value_type(h, new ConditionQuery< OrientedBoxCondition >( OrientedBoxCondition( orientedbox ), h ) ) );
	}
	catch (...)
	{
		setLastErrorCode( PTV_OUT_OF_MEMORY );
		return 0;
	}

	return h;
}

//-----------------------------------------------------------------------------
// Set the query density
//-----------------------------------------------------------------------------
PTres PTAPI ptSetQueryDensity( PThandle query, PTenum densityType, PTfloat densityValue )
{
	PTTRACE_FUNC_P3( query, densityType, densityValue )

	using namespace querydetail;

	/* TEST ONLY */ 
	//if (densityType == PT_QUERY_DENSITY_FULL)
	//	densityType = PT_QUERY_DENSITY_VIEW_COMPLETE;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{

		if (densityType <PT_QUERY_DENSITY_FULL  || (densityType > PT_QUERY_DENSITY_VIEW_COMPLETE && densityType != PT_QUERY_DENSITY_SPATIAL)) 
			return setLastErrorCode( PTV_INVALID_PARAMETER );

		i->second->setDensity( densityType, densityValue );
		return setLastErrorCode( PTV_SUCCESS );
	}
	else
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
}
//-----------------------------------------------------------------------------
// Sets the queries scope
//-----------------------------------------------------------------------------
PTres PTAPI ptSetQueryScope( PThandle query, PThandle cloudOrSceneHandle )
{
	PTTRACE_FUNC_P2( query, cloudOrSceneHandle )

	using namespace querydetail;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{
		i->second->setScope( cloudOrSceneHandle );
		return setLastErrorCode( PTV_SUCCESS );
	}
	else
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
}
//
//
//
PTres PTAPI ptSetQueryLayerMask( PThandle query, PTubyte layerMask )
{
	PTTRACE_FUNC_P2( query, layerMask )

	using namespace querydetail;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{
		i->second->setLayers( layerMask );
		return setLastErrorCode( PTV_SUCCESS );
	}
	else
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
}
//-----------------------------------------------------------------------------
// The RGB mode
//-----------------------------------------------------------------------------
PTres PTAPI ptSetQueryRGBMode( PThandle query, PTenum mode )
{
	PTTRACE_FUNC_P2( query, mode )

	using namespace querydetail;

	QueryMap::iterator i = s_queries.find( query );
	if (i != s_queries.end())
	{
		i->second->setRGBMode( mode );
		return setLastErrorCode( PTV_SUCCESS );
	}
	else
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
static void updateColourConstants()
{
	if (g_currentRenderContext)
	{
		PointsRenderer *renderer = g_currentRenderContext->renderer();
		renderer->updateColourConstants( g_currentRenderContext->settings());
	}
}
//-----------------------------------------------------------------------------
// compute number of points that will be returned by query | false = no points
//-----------------------------------------------------------------------------
bool computePntLimitDensity( querydetail::Query *query )
{
	using namespace querydetail;
	
	if (query->doesOwnDensityLimitCompute()						// not required
		|| query->getDensityType() != PT_QUERY_DENSITY_LIMIT) 
		return true;
	
	int64_t numPointsRequired = query->getDensityLimit();

	query->setDensity( PT_QUERY_DENSITY_FULL, 1.0f );	//no good because it'll load
	//query->resetQuery();

	//int numPointsIteration = query->runQuery(1000000, (PTdouble*)0, 0, 0, 0);
	//int64_t numPointsInQuery = numPointsIteration;

	//while (numPointsIteration)
	//{
	//	numPointsIteration = query->runQuery(1000000, (PTdouble*)0, 0, 0, 0);
	//	numPointsInQuery += numPointsIteration;
	//}
	int64_t numPointsInQuery = query->computeNumPointsInQuery();

	float densityCoeff = (numPointsInQuery > 0) ? (float)numPointsRequired / numPointsInQuery : 0;

	if (!numPointsInQuery) 
		return false;

	// cap
	if (densityCoeff > 1.0f) 
		densityCoeff = 1.0f;

	query->setDensity( PT_QUERY_DENSITY_LIMIT, densityCoeff );
	query->resetQuery();
	
	return true;
}
//-----------------------------------------------------------------------------
// get the points from the query
//-----------------------------------------------------------------------------
static querydetail::Query *prepareQueryRun( PThandle query )
{
	using namespace querydetail;

	//int *c =0 ;	// force a crash to check symbols
	//(*c)++;
	
	setLastErrorCode( PTV_SUCCESS );

	updateColourConstants();

	extern PTvoid _ptApplyShader(PTint);
	extern PTuint g_currentViewport;
	_ptApplyShader(g_currentViewport);

	QueryMap::iterator i = s_queries.find(query);
	if (i == s_queries.end()) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );			
		return 0;
	}
	if (i->second->isReset())	// compute density required to hit point limit
	{
		if (!computePntLimitDensity( i->second )) 
			return 0;
	}

	static int counter = 0;

	return i->second;
}
//-----------------------------------------------------------------------------
// select the result of the query
//-----------------------------------------------------------------------------
PTuint PTAPI ptSelectQueryPoints( PThandle query )
{
	PTTRACE_FUNC_P1( query )

	using namespace querydetail;

	setLastErrorCode( PTV_SUCCESS ); ///!FixMe: What if there is already an outstanding error from a previous operation?

	QueryMap::iterator i = s_queries.find(query);
	if (i == s_queries.end()) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );			
		return 0;
	}
	Query *q = i->second;

	return q->selectQueryPoints(true);
}

//-----------------------------------------------------------------------------
// select the result of the query
//-----------------------------------------------------------------------------
PTuint PTAPI ptDeselectQueryPoints( PThandle query )
{
	PTTRACE_FUNC_P1( query )

	using namespace querydetail;

	setLastErrorCode( PTV_SUCCESS ); ///!FixMe: What if there is already an outstanding error from a previous operation?

	QueryMap::iterator i = s_queries.find(query);
	if (i == s_queries.end()) 
	{
		setLastErrorCode( PTV_INVALID_HANDLE );			
		return 0;
	}
	Query *q = i->second;

	return q->selectQueryPoints(false);
}
//-----------------------------------------------------------------------------
// get the points from the query
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetQueryPointsd( PThandle query, PTuint buffersize, PTdouble *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer )
{ 
	PTTRACE_FUNC_P3( query, buffersize, geomBuffer )

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;

	int number_points = q->runQuery( buffersize, geomBuffer, rgbBuffer, intensityBuffer, classificationBuffer, selectionBuffer );

	PTTRACEOUT << "= " << number_points;

	return (PTuint)number_points;
}
//-----------------------------------------------------------------------------
// Float version
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetQueryPointsf( PThandle query, PTuint buffersize, PTfloat *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer )
{
	PTTRACE_FUNC_P3( query, buffersize, geomBuffer )

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;

	int number_points = q->runQuery( buffersize, geomBuffer, rgbBuffer, intensityBuffer, classificationBuffer, selectionBuffer );

	PTTRACEOUT << "= " << number_points;

	return (PTuint)number_points;
}

//-----------------------------------------------------------------------------
// Multi result set Float version
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetQueryPointsMultif( PThandle query, PTuint numResultSets, PTuint bufferSize, PTuint *resultSetSize, PTfloat **geomBufferArray, PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray)
{
	PTTRACE_FUNC_P3( query, bufferSize, geomBufferArray )

	if(bufferSize == 0 || geomBufferArray == NULL)
		return setLastErrorCode(PTV_INVALID_PARAMETER);		

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;

	return q->runQueryMulti(numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, intensityBufferArray);
}

//-----------------------------------------------------------------------------
// Multi result set double version
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetQueryPointsMultid( PThandle query, PTuint numResultSets, PTuint bufferSize, PTuint *resultSetSize, PTdouble **geomBufferArray, PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray)
{
	PTTRACE_FUNC_P3( query, bufferSize, geomBufferArray )

	if(bufferSize == 0 || geomBufferArray == NULL)
		return setLastErrorCode(PTV_INVALID_PARAMETER);		

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;

	return q->runQueryMulti(numResultSets, bufferSize, resultSetSize, geomBufferArray, rgbBufferArray, intensityBufferArray);
}


//-----------------------------------------------------------------------------
// Query version
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetDetailedQueryPointsf( PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer,
							   PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *filter, PTubyte *classificationBuffer,
							   PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
{
	PTTRACE_FUNC_P3( query, bufferSize, geomBuffer )

	if(bufferSize == 0 || geomBuffer == NULL)
		return setLastErrorCode(PTV_INVALID_PARAMETER);		

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;


	return q->runDetailedQuery( bufferSize, geomBuffer, rgbBuffer, intensityBuffer,
		filter, classificationBuffer, numPointChannels, pointChannelsReq, pointChannels);

}
//-----------------------------------------------------------------------------
// Query version
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetDetailedQueryPointsd( PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer,
							   PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *filter, PTubyte *classificationBuffer,
							   PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels )
{
	PTTRACE_FUNC_P3( query, bufferSize, geomBuffer )

	if(bufferSize == 0 || geomBuffer == NULL)
		return setLastErrorCode(PTV_INVALID_PARAMETER);		

	querydetail::Query *q = prepareQueryRun( query );

	if (!q) return 0;

	return q->runDetailedQuery( bufferSize, geomBuffer, rgbBuffer, intensityBuffer,
		filter, classificationBuffer, numPointChannels, pointChannelsReq, pointChannels);
}
//-----------------------------------------------------------------------------
/* interaction */
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptFlipMouseYCoords( void );
PTvoid	PTAPI ptDontFlipMouseYCoords( void );
//-----------------------------------------------------------------------------
// User data channel
//-----------------------------------------------------------------------------
PTres		PTAPI ptSubmitPointChannelUpdate( PThandle query, PThandle channel )
{
	PTTRACE_FUNC_P2( query, channel )

	using namespace querydetail;

	/* find the query and run an update */
	QueryMap::iterator i = s_queries.find(query);
	if (i == s_queries.end())
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );				
	}
	i->second->submitChannelUpdate(channel);
	return setLastErrorCode( PTV_SUCCESS );
}


//-----------------------------------------------------------------------------
// Doxygen pages on specific topics
//-----------------------------------------------------------------------------

/* -----------------------------------------------------------------------------
 * Condition Classes *//**
 * \page conditions Condition Classes
 * 
 * Note that the classes named <Prefix>Condition and
 * used as policy template arguments are divided into two classes.
 *
 * The first, including FrustumCondition, VisibleCondition, and
 * SelectedCondition have static member functions.
 * The second, including SphereCondition, BoxCondition, and NullCondition, have
 * non-static member functions.
 * [I didn't know that this could work: we call static functions via
 * instance_ptr->member_func() and instance.member_func() syntax - AC ]
 *
 * ...
 */
